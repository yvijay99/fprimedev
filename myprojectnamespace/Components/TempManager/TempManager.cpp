// ======================================================================
// \title  TempManager.cpp
// \author lauraf26846
// \brief  cpp file for TempManager component implementation class
// ======================================================================

#include "myprojectnamespace/Components/TempManager/TempManager.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

TempManager ::TempManager(const char* const compName) : TempManagerComponentBase(compName) {}

TempManager ::~TempManager() {}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void TempManager ::run_handler(FwIndexType portNum, U32 context) {
    // TODO
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void TempManager ::READ_TEMP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    
    const U8 devAddr = 0x4a; // default TMP102 I2C register
    const U8 tempReg = 0x00; // default TMP102 12-bit temperature register

    U8 writeBuf[1] = {tempReg}; // Write buffer (register pointer)
    U8 readBuf[2] = {0}; // Read buffer (2 bytes from TMP102)

    Fw::Buffer writeFwBuf(writeBuf, sizeof(writeBuf));
    Fw::Buffer readFwBuf(readBuf, sizeof(readBuf));

    // I2C transaction: write register -> read data
    Drv::I2cStatus status = this->busWriteRead_out(0, devAddr, writeFwBuf, readFwBuf);

    if (status != Drv::I2cStatus::I2C_OK) {
        this->log_WARNING_HI_TempReadError(status);
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }

    // Combine bytes (16 bits)
    I16 raw = (readBuf[0] << 8) | readBuf[1];
    // TMP102 uses upper 12 bits -> shift right by 4 to get rid of unused bits
    raw >>= 4;
    // Handle negative temperatures (sign extend 12-bit)
    if (raw & 0x800) {  // sign bit (bit 11)
        raw |= 0xF000;
    }
    // Convert to Celsius
    F32 temperature = raw * 0.0625f; // 1 Least Significant Bit (LSB) = 0.0625 °C

    // Send telemetry
    this->tlmWrite_Temperature(temperature);

    // End command
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
