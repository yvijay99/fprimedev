// ======================================================================
// \title  TempManager.cpp
// \author lauraf26846
// \brief  cpp file for TempManager component implementation class
// ======================================================================

#include "myprojectnamespace/Components/TempManager/TempManager.hpp"

namespace Components {

TempManager::TempManager(const char* const compName) : TempManagerComponentBase(compName) {}

TempManager::~TempManager() {}

void TempManager::readAndReportTemp() {
    const U8 devAddr = 0x4a;
    const U8 tempReg = 0x00;

    U8 writeBuf[1] = {tempReg};
    U8 readBuf[2] = {0};

    Fw::Buffer writeFwBuf(writeBuf, sizeof(writeBuf));
    Fw::Buffer readFwBuf(readBuf, sizeof(readBuf));

    Drv::I2cStatus status = this->busWriteRead_out(0, devAddr, writeFwBuf, readFwBuf);

    if (status != Drv::I2cStatus::I2C_OK) {
        this->log_WARNING_HI_TempReadError(status);
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
        return;
    }

    I16 raw = (readBuf[0] << 8) | readBuf[1];
    raw >>= 4;
    if (raw & 0x800) {
        raw |= 0xF000;
    }
    F32 temperature = raw * 0.0625f;

    this->tlmWrite_Temperature(temperature);
    if (this->isConnected_healthOut_OutputPort(0)) {
        this->healthOut_out(0, true);
    }
}

void TempManager::run_handler(FwIndexType portNum, U32 context) {
    if (this->isConnected_busWriteRead_OutputPort(0)) {
        this->readAndReportTemp();
    }
}

void TempManager::READ_TEMP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->readAndReportTemp();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
