// ======================================================================
// \title  SensorManager.cpp
// \author yuktivijay
// \brief  cpp file for SensorManager component implementation class
// ======================================================================

#include <cmath>
#include "myprojectnamespace/Components/SensorManager/SensorManager.hpp"

namespace Managers {

SensorManager::SensorManager(const char* const compName)
    : SensorManagerComponentBase(compName),
      m_simTick(0) {}

SensorManager::~SensorManager() {}

void SensorManager::configure(U32 i2cAddress) {
    this->m_i2cAddress = i2cAddress;
}

void SensorManager::run_handler(FwIndexType portNum, U32 context) {
    this->sensorSm_sendSignal_tick();
}

// ---- State machine actions ----

void SensorManager::Managers_SensorManagerStateMachine_action_doInit(
    SmId smId, Managers_SensorManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::sensorSm);
    F32 ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0, temp = 0;
    Drv::I2cStatus status = this->readImuData(ax, ay, az, gx, gy, gz, temp);
    if (status == Drv::I2cStatus::I2C_OK) {
        this->sensorSm_sendSignal_success();
    } else {
        this->sensorSm_sendSignal_fault();
    }
}

void SensorManager::Managers_SensorManagerStateMachine_action_doRead(
    SmId smId, Managers_SensorManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::sensorSm);
    F32 ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0, temp = 0;
    Drv::I2cStatus status = this->readImuData(ax, ay, az, gx, gy, gz, temp);

    if (status == Drv::I2cStatus::I2C_OK) {
        this->tlmWrite_AccelX(ax);
        this->tlmWrite_AccelY(ay);
        this->tlmWrite_AccelZ(az);
        this->tlmWrite_GyroX(gx);
        this->tlmWrite_GyroY(gy);
        this->tlmWrite_GyroZ(gz);
        this->tlmWrite_ImuTemp(temp);
        this->log_ACTIVITY_LO_ImuReadOk();
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, true);
        }
    } else {
        this->log_WARNING_HI_ImuReadError(static_cast<I32>(status.e));
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
        this->sensorSm_sendSignal_fault();
    }
}

void SensorManager::Managers_SensorManagerStateMachine_action_doFaultRecovery(
    SmId smId, Managers_SensorManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::sensorSm);
    if (this->isConnected_healthOut_OutputPort(0)) {
        this->healthOut_out(0, false);
    }
    F32 ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0, temp = 0;
    Drv::I2cStatus status = this->readImuData(ax, ay, az, gx, gy, gz, temp);
    if (status == Drv::I2cStatus::I2C_OK) {
        this->sensorSm_sendSignal_success();
    }
}

void SensorManager::Managers_SensorManagerStateMachine_action_doSimRead(
    SmId smId, Managers_SensorManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::sensorSm);
    F32 ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0, temp = 0;
    this->simulateImuData(ax, ay, az, gx, gy, gz, temp);
    m_simTick++;

    this->tlmWrite_AccelX(ax);
    this->tlmWrite_AccelY(ay);
    this->tlmWrite_AccelZ(az);
    this->tlmWrite_GyroX(gx);
    this->tlmWrite_GyroY(gy);
    this->tlmWrite_GyroZ(gz);
    this->tlmWrite_ImuTemp(temp);
    this->log_ACTIVITY_LO_ImuReadOk();
    if (this->isConnected_healthOut_OutputPort(0)) {
        this->healthOut_out(0, true);
    }
}

// ---- Command handlers ----

void SensorManager::CALIBRATE_IMU_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_ImuCalibrationStarted();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SensorManager::ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_simTick = 0;
    this->log_ACTIVITY_HI_SimModeEnabled();
    this->sensorSm_sendSignal_enableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SensorManager::DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_SimModeDisabled();
    this->sensorSm_sendSignal_disableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// ---- I2C helpers ----

Drv::I2cStatus SensorManager::readImuData(F32& ax, F32& ay, F32& az,
                                           F32& gx, F32& gy, F32& gz,
                                           F32& temp) {
    U8 regAddr = ACCEL_XOUT_H;
    Fw::Buffer writeBuffer(&regAddr, sizeof(regAddr));

    U8 rawData[DATA_SIZE] = {};
    Fw::Buffer readBuffer(rawData, sizeof(rawData));

    Drv::I2cStatus status = this->busWriteRead_out(0, this->m_i2cAddress, writeBuffer, readBuffer);

    if (status == Drv::I2cStatus::I2C_OK) {
        I16 rawAx = static_cast<I16>((rawData[0] << 8) | rawData[1]);
        I16 rawAy = static_cast<I16>((rawData[2] << 8) | rawData[3]);
        I16 rawAz = static_cast<I16>((rawData[4] << 8) | rawData[5]);
        I16 rawTemp = static_cast<I16>((rawData[6] << 8) | rawData[7]);
        I16 rawGx = static_cast<I16>((rawData[8] << 8) | rawData[9]);
        I16 rawGy = static_cast<I16>((rawData[10] << 8) | rawData[11]);
        I16 rawGz = static_cast<I16>((rawData[12] << 8) | rawData[13]);

        ax = static_cast<F32>(rawAx) / 8192.0f;
        ay = static_cast<F32>(rawAy) / 8192.0f;
        az = static_cast<F32>(rawAz) / 8192.0f;
        gx = static_cast<F32>(rawGx) / 65.5f;
        gy = static_cast<F32>(rawGy) / 65.5f;
        gz = static_cast<F32>(rawGz) / 65.5f;
        temp = (static_cast<F32>(rawTemp) - 21.0f) / 333.87f + 21.0f;
    }

    return status;
}

void SensorManager::simulateImuData(F32& ax, F32& ay, F32& az,
                                     F32& gx, F32& gy, F32& gz,
                                     F32& temp) {
    float t = static_cast<float>(m_simTick) * 0.1f;

    ax = 0.5f * sinf(t);
    ay = 0.5f * sinf(t + 1.0f);
    az = 1.0f + 0.1f * sinf(t + 2.0f);

    gx = 2.0f * sinf(t * 0.5f);
    gy = 2.0f * sinf(t * 0.5f + 1.0f);
    gz = 1.0f * sinf(t * 0.5f + 2.0f);

    temp = 25.0f + 5.0f * sinf(t * 0.05f);
}

}  // namespace Managers
