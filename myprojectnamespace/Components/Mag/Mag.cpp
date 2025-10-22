// ======================================================================
// \title  Mag.cpp
// \author yuktivijay
// \brief  cpp file for Mag component implementation class
// ======================================================================

#include "myprojectnamespace/Components/Mag/Mag.hpp"

namespace Mag {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Mag ::Mag(const char* const compName) : MagComponentBase(compName) {}

Mag ::~Mag() {}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void Mag ::TODO_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // TODO
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Mag
