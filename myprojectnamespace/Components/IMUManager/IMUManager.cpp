// IMUManager.cpp

#include <cmath>
#include "myprojectnamespace/Components/IMUManager/IMUManager.hpp"


namespace Managers {

IMUManager::IMUManager(const char* const compName)
    : IMUManagerComponentBase(compName),
      m_simTick(0) {}

IMUManager::~IMUManager() {}

void IMUManager::configure(U32 i2cAddress) {
    this->m_i2cAddress = i2cAddress;
}

void IMUManager::run_handler(FwIndexType portNum, U32 context) {
    this->imuSm_sendSignal_tick();
}

// state machine actions

// setting this to always sim for debugging
void IMUManager::Managers_IMUManagerStateMachine_action_doInit(
    SmId smId, Managers_IMUManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::imuSm);
    this->log_ACTIVITY_HI_StateChange(IMUManager_SensorState::SIM);
    this->imuSm_sendSignal_enableSim();
}

// grab accel, gyro, and temp and push telemetry, fault out if the bus dies
void IMUManager::Managers_IMUManagerStateMachine_action_doRead(
    SmId smId, Managers_IMUManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::imuSm);
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
    } else {
        this->log_ACTIVITY_HI_StateChange(IMUManager_SensorState::FAULT);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
        this->imuSm_sendSignal_fault();
    }
}

// retry the read, if it comes back we flip back to running
void IMUManager::Managers_IMUManagerStateMachine_action_doFaultRecovery(
    SmId smId, Managers_IMUManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::imuSm);
    F32 ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0, temp = 0;
    Drv::I2cStatus status = this->readImuData(ax, ay, az, gx, gy, gz, temp);
    if (status == Drv::I2cStatus::I2C_OK) {
        this->log_ACTIVITY_HI_StateChange(IMUManager_SensorState::RUNNING);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, true);
        }
        this->imuSm_sendSignal_success();
    } else {
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
    }
}

// fake sinusoidal imu data when there's no hardware
void IMUManager::Managers_IMUManagerStateMachine_action_doSimRead(
    SmId smId, Managers_IMUManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::imuSm);
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
}

// command handlers

void IMUManager::CALIBRATE_IMU_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_ImuCalibrationStarted();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void IMUManager::ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_simTick = 0;
    this->log_ACTIVITY_HI_StateChange(IMUManager_SensorState::SIM);
    this->imuSm_sendSignal_enableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void IMUManager::DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_StateChange(IMUManager_SensorState::INIT);
    this->imuSm_sendSignal_disableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// pull 14 bytes from reg 0x2d (accel xyz, temp, gyro xyz) and decode big-endian 16-bit signed
Drv::I2cStatus IMUManager::readImuData(F32& ax, F32& ay, F32& az,
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

        // scale factors for +-4g accel and +-500dps gyro
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

void IMUManager::simulateImuData(F32& ax, F32& ay, F32& az,
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
