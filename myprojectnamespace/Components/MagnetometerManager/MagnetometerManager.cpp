// MagnetometerManager.cpp

#include <cmath>
#include "myprojectnamespace/Components/MagnetometerManager/MagnetometerManager.hpp"


namespace Managers {

MagnetometerManager::MagnetometerManager(const char* const compName)
    : MagnetometerManagerComponentBase(compName),
      m_simTick(0) {}

MagnetometerManager::~MagnetometerManager() {}

void MagnetometerManager::configure(U32 i2cAddress) {
    this->m_i2cAddress = i2cAddress;
}

void MagnetometerManager::run_handler(FwIndexType portNum, U32 context) {
    this->magSm_sendSignal_tick();
}

// state machine actions
// heads up: we're on rateGroup2 (10Hz, one tick every 100ms).
// the rm3100 in poll mode needs ~25ms between "start measurement" and "data ready".
// we used to usleep(25000) between the two i2c calls but that blocked the rate group thread,
// which also blocks every other component sharing the same rate group. bad news.
// instead we split the read across two ticks: tick N writes the poll register, tick N+1 reads
// the data register. 100ms between them is way more than the 25ms the chip needs, so it's safe.
// net effect: we get a fresh reading every 200ms (5Hz), which is plenty for attitude.

// first tick after boot: kick off a measurement so tick 2 can actually read something
void MagnetometerManager::Managers_MagnetometerManagerStateMachine_action_doInit(
    SmId smId, Managers_MagnetometerManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::magSm);
    Drv::I2cStatus status = this->triggerMeasurement();

    if (status == Drv::I2cStatus::I2C_OK) {
        m_pollIssued = true;
        this->log_ACTIVITY_HI_StateChange(MagnetometerManager_SensorState::RUNNING);
        this->magSm_sendSignal_success();
    } else {
        m_pollIssued = false;
        this->log_ACTIVITY_HI_StateChange(MagnetometerManager_SensorState::FAULT);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
        this->magSm_sendSignal_fault();
    }
}

// called every 100ms tick. alternates between "trigger" and "read" using m_pollIssued
// so we never need to sleep waiting for the chip to finish converting.
void MagnetometerManager::Managers_MagnetometerManagerStateMachine_action_doRead(
    SmId smId, Managers_MagnetometerManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::magSm);

    // no measurement in flight yet: fire one off and bail, we'll read it next tick
    if (!m_pollIssued) {
        if (this->triggerMeasurement() != Drv::I2cStatus::I2C_OK) {
            this->log_ACTIVITY_HI_StateChange(MagnetometerManager_SensorState::FAULT);
            if (this->isConnected_healthOut_OutputPort(0)) {
                this->healthOut_out(0, false);
            }
            this->magSm_sendSignal_fault();
            return;
        }
        m_pollIssued = true;
        return;
    }

    // second tick: measurement should be done by now, pull the 9 data bytes
    F32 mx = 0, my = 0, mz = 0;
    Drv::I2cStatus status = this->readMagData(mx, my, mz);
    m_pollIssued = false;

    if (status != Drv::I2cStatus::I2C_OK) {
        this->log_ACTIVITY_HI_StateChange(MagnetometerManager_SensorState::FAULT);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
        this->magSm_sendSignal_fault();
        return;
    }

    this->tlmWrite_MagX(mx);
    this->tlmWrite_MagY(my);
    this->tlmWrite_MagZ(mz);
    // we read at 5Hz so an event every 10 reads = one event every 2s, not spammy
    if (++m_readCount % READ_LOG_INTERVAL == 0) {
        this->log_ACTIVITY_LO_MagReading(mx, my, mz);
    }
}

// same two-phase dance as doRead, just flips back to RUNNING on a clean read
void MagnetometerManager::Managers_MagnetometerManagerStateMachine_action_doFaultRecovery(
    SmId smId, Managers_MagnetometerManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::magSm);

    if (!m_pollIssued) {
        if (this->triggerMeasurement() == Drv::I2cStatus::I2C_OK) {
            m_pollIssued = true;
        }
        return;
    }

    F32 mx = 0, my = 0, mz = 0;
    Drv::I2cStatus status = this->readMagData(mx, my, mz);
    m_pollIssued = false;

    if (status == Drv::I2cStatus::I2C_OK) {
        this->tlmWrite_MagX(mx);
        this->tlmWrite_MagY(my);
        this->tlmWrite_MagZ(mz);
        this->log_ACTIVITY_HI_StateChange(MagnetometerManager_SensorState::RUNNING);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, true);
        }
        this->magSm_sendSignal_success();
    }
}

// fake mag data wobbling around earth-field-ish values when there's no hardware
void MagnetometerManager::Managers_MagnetometerManagerStateMachine_action_doSimRead(
    SmId smId, Managers_MagnetometerManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::magSm);
    F32 mx = 0, my = 0, mz = 0;
    this->simulateMagData(mx, my, mz);
    m_simTick++;

    this->tlmWrite_MagX(mx);
    this->tlmWrite_MagY(my);
    this->tlmWrite_MagZ(mz);
    if (++m_readCount % READ_LOG_INTERVAL == 0) {
        this->log_ACTIVITY_LO_MagReading(mx, my, mz);
    }
}

// command handlers

void MagnetometerManager::CALIBRATE_MAG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_MagCalibrationStarted();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void MagnetometerManager::ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_simTick = 0;
    this->log_ACTIVITY_HI_StateChange(MagnetometerManager_SensorState::SIM);
    this->magSm_sendSignal_enableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void MagnetometerManager::DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_StateChange(MagnetometerManager_SensorState::INIT);
    this->magSm_sendSignal_disableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// write 0x70 to POLL_REG (0x00) = "start a one-shot measurement on all three axes"
// rm3100 will take ~25ms to finish, we read next tick (~100ms later) so no sleep needed
Drv::I2cStatus MagnetometerManager::triggerMeasurement() {
    U8 cmd[2] = {POLL_REG, POLL_XYZ};
    Fw::Buffer buffer(cmd, sizeof(cmd));
    return this->busWrite_out(0, this->m_i2cAddress, buffer);
}

// pull 9 bytes starting at MX_REG (0x24): 3 axes x 3 bytes each (24-bit signed)
// divide by 75 (sensitivity count) to get microtesla
Drv::I2cStatus MagnetometerManager::readMagData(F32& mx, F32& my, F32& mz) {
    U8 regAddr = MX_REG;
    Fw::Buffer writeBuffer(&regAddr, sizeof(regAddr));
    U8 rawData[MAG_DATA_SIZE] = {};
    Fw::Buffer readBuffer(rawData, sizeof(rawData));

    Drv::I2cStatus status = this->busWriteRead_out(0, this->m_i2cAddress, writeBuffer, readBuffer);

    if (status == Drv::I2cStatus::I2C_OK) {
        I32 rawMx = (static_cast<I32>(rawData[0]) << 16) |
                    (static_cast<I32>(rawData[1]) << 8) |
                     static_cast<I32>(rawData[2]);
        I32 rawMy = (static_cast<I32>(rawData[3]) << 16) |
                    (static_cast<I32>(rawData[4]) << 8) |
                     static_cast<I32>(rawData[5]);
        I32 rawMz = (static_cast<I32>(rawData[6]) << 16) |
                    (static_cast<I32>(rawData[7]) << 8) |
                     static_cast<I32>(rawData[8]);

        // sign extend 24-bit to 32-bit
        if (rawMx & 0x800000) { rawMx |= 0xFF000000; }
        if (rawMy & 0x800000) { rawMy |= 0xFF000000; }
        if (rawMz & 0x800000) { rawMz |= 0xFF000000; }

        mx = static_cast<F32>(rawMx) / RM3100_SENSITIVITY;
        my = static_cast<F32>(rawMy) / RM3100_SENSITIVITY;
        mz = static_cast<F32>(rawMz) / RM3100_SENSITIVITY;
    }

    return status;
}

void MagnetometerManager::simulateMagData(F32& mx, F32& my, F32& mz) {
    float t = static_cast<float>(m_simTick) * 0.1f;
    mx = 20.0f + 5.0f * sinf(t);
    my = 20.0f + 5.0f * sinf(t + 1.0f);
    mz = -40.0f + 5.0f * sinf(t + 2.0f);
}

}  // namespace Managers
