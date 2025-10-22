// ======================================================================
// \title  Gyro.cpp
// \author yuktivijay
// \brief  cpp file for Gyro component implementation class
// ======================================================================

#include "myprojectnamespace/Components/Gyro/Gyro.hpp"

namespace Gyro {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Gyro ::Gyro(const char* const compName) : GyroComponentBase(compName) {}

Gyro ::~Gyro() {}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void Gyro ::TODO_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Gyro
