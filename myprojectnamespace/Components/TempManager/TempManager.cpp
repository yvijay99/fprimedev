// TempManager.cpp

#include <cmath>
#include "myprojectnamespace/Components/TempManager/TempManager.hpp"


namespace Components {

TempManager::TempManager(const char* const compName)
    : TempManagerComponentBase(compName),
      m_simTick(0) {}

TempManager::~TempManager() {}

void TempManager::run_handler(FwIndexType portNum, U32 context) {
    this->tempSm_sendSignal_tick();
}

void TempManager::Components_TempManagerStateMachine_action_doInit(
    SmId smId, Components_TempManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::tempSm);
    F32 temperature = 0.0f;
    Drv::I2cStatus status = this->readRawTemp(temperature);
    if (status == Drv::I2cStatus::I2C_OK) {
        this->log_ACTIVITY_HI_StateChange(TempManager_SensorState::RUNNING);
        this->tempSm_sendSignal_success();
    } else {
        this->log_ACTIVITY_HI_StateChange(TempManager_SensorState::FAULT);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
        this->tempSm_sendSignal_fault();
    }
}

void TempManager::Components_TempManagerStateMachine_action_doRead(
    SmId smId, Components_TempManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::tempSm);
    F32 temperature = 0.0f;
    Drv::I2cStatus status = this->readRawTemp(temperature);

    if (status == Drv::I2cStatus::I2C_OK) {
        this->tlmWrite_Temperature(temperature);
        if (++m_readCount % READ_LOG_INTERVAL == 0) {
            this->log_ACTIVITY_LO_TempReading(temperature);
        }
    } else {
        this->log_ACTIVITY_HI_StateChange(TempManager_SensorState::FAULT);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
        this->tempSm_sendSignal_fault();
    }
}

void TempManager::Components_TempManagerStateMachine_action_doFaultRecovery(
    SmId smId, Components_TempManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::tempSm);
    F32 temperature = 0.0f;
    Drv::I2cStatus status = this->readRawTemp(temperature);
    if (status == Drv::I2cStatus::I2C_OK) {
        this->tlmWrite_Temperature(temperature);
        this->log_ACTIVITY_HI_StateChange(TempManager_SensorState::RUNNING);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, true);
        }
        this->tempSm_sendSignal_success();
    } else {
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
    }
}

void TempManager::Components_TempManagerStateMachine_action_doSimRead(
    SmId smId, Components_TempManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::tempSm);
    float t = static_cast<float>(m_simTick) * 0.1f;
    F32 temperature = 25.0f + 5.0f * sinf(t * 0.05f);
    m_simTick++;

    this->tlmWrite_Temperature(temperature);
}

void TempManager::READ_TEMP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    F32 temperature = 0.0f;
    Drv::I2cStatus status = this->readRawTemp(temperature);
    if (status == Drv::I2cStatus::I2C_OK) {
        this->tlmWrite_Temperature(temperature);
    }
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void TempManager::ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_simTick = 0;
    this->log_ACTIVITY_HI_StateChange(TempManager_SensorState::SIM);
    this->tempSm_sendSignal_enableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void TempManager::DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_StateChange(TempManager_SensorState::INIT);
    this->tempSm_sendSignal_disableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

Drv::I2cStatus TempManager::readRawTemp(F32& temperature) {
    const U8 devAddr = 0x4a;
    const U8 tempReg = 0x00;

    U8 writeBuf[1] = {tempReg};
    U8 readBuf[2] = {0};

    Fw::Buffer writeFwBuf(writeBuf, sizeof(writeBuf));
    Fw::Buffer readFwBuf(readBuf, sizeof(readBuf));

    Drv::I2cStatus status = this->busWriteRead_out(0, devAddr, writeFwBuf, readFwBuf);

    if (status == Drv::I2cStatus::I2C_OK) {
        I16 raw = (readBuf[0] << 8) | readBuf[1];
        raw >>= 4;
        if (raw & 0x800) {
            raw |= 0xF000;
        }
        temperature = raw * 0.0625f;
    }

    return status;
}

}  // namespace Components
