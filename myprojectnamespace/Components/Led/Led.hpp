// Led.hpp

#ifndef ledmanager_Led_HPP
#define ledmanager_Led_HPP

#include "myprojectnamespace/Components/Led/LedComponentAc.hpp"

namespace ledmanager {

class Led final : public LedComponentBase {
  public:
    Led(const char* const compName);
    ~Led();

  private:
    Fw::On m_state = Fw::On::OFF;
    U64 m_transitions = 0;
    U32 m_toggleCounter = 0;
    Fw::On m_blinking = Fw::On::OFF;

    void BLINKING_ON_OFF_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On onOff) override;
    void run_handler(FwIndexType portNum, U32 context) override;
    void parameterUpdated(FwPrmIdType id) override;
};

}  // namespace ledmanager

#endif
