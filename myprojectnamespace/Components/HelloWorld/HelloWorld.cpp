// ======================================================================
// \title  HelloWorld.cpp
// \author yuktivijay
// \brief  cpp file for HelloWorld component implementation class
// ======================================================================

#include "myprojectnamespace/Components/HelloWorld/HelloWorld.hpp"

namespace Components {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

HelloWorld ::HelloWorld(const char* const compName) : HelloWorldComponentBase(compName) {}

HelloWorld ::~HelloWorld() {}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void HelloWorld :: SAY_HELLO_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, const Fw::CmdStringArg& greeting) {
    Fw::LogStringArg eventGreeting(greeting.toChar());
    this -> log_ACTIVITY_HI_Hello(eventGreeting);
    this -> tlmWrite_GreetingCount(++this->m_greetingCount);
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Components
