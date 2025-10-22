// ======================================================================
// \title  HelloWorld.hpp
// \author yuktivijay
// \brief  hpp file for HelloWorld component implementation class
// ======================================================================

#ifndef Components_HelloWorld_HPP
#define Components_HelloWorld_HPP

#include "myprojectnamespace/Components/HelloWorld/HelloWorldComponentAc.hpp"

namespace Components {

class HelloWorld final : public HelloWorldComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct HelloWorld object
    HelloWorld(const char* const compName  //!< The component name
    );

    //! Destroy HelloWorld object
    ~HelloWorld();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------
    U32 m_greetingCount = 0;
    //! Handler implementation for command TODO
    //!
    //! TODO
    void SAY_HELLO_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, 
    const Fw::CmdStringArg& greeting) override;
};

}  // namespace Components

#endif
