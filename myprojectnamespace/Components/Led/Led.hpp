// ======================================================================
// \title  Led.hpp
// \author yuktivijay
// \brief  hpp file for Led component implementation class
// ======================================================================

#ifndef ledmanager_Led_HPP
#define ledmanager_Led_HPP

#include "myprojectnamespace/Components/Led/LedComponentAc.hpp"

namespace ledmanager {

class Led final : public LedComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct Led object
    Led(const char* const compName  //!< The component name
    );

    //! Destroy Led object
    ~Led();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command TODO
    //!
    //! TODO
    
    Fw::On m_state = Fw::On::OFF; //! Keeps track if LED is on or off
    U64 m_transitions = 0; //! The number of on/off transitions that have occurred from FSW boot up
    U32 m_toggleCounter = 0; //! Keeps track of how many ticks the LED has been on for
    Fw::On m_blinking = Fw::On::OFF; //! Flag: if true then LED blinking will occur else no blinking will happen
    
    void BLINKING_ON_OFF_cmdHandler(FwOpcodeType opCode, 
              U32 cmdSeq, 
              Fw::On onOff) override;

    void run_handler(FwIndexType portNum,  //!< The port number
                     U32 context           //!< The call order
                     ) override;
    
    void parameterUpdated(FwPrmIdType id  //!< The parameter ID
                      ) override;
    
    void configure(U32 number);
};

}  // namespace ledmanager

#endif
