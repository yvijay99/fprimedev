// ======================================================================
// \title  TempManager.hpp
// \author lauraf26846
// \brief  hpp file for TempManager component implementation class
// ======================================================================

#ifndef tempManager_TempManager_HPP
#define tempManager_TempManager_HPP

#include "myprojectnamespace/Components/TempManager/TempManagerComponentAc.hpp"

namespace Components {

class TempManager final : public TempManagerComponentBase {
  public:
    static constexpr U8 DEFAULT_ADDR = 0x48;
    static constexpr U8 DATA_SIZE = 6;

    TempManager(const char* const compName);
    ~TempManager();

  private:
    U32 m_simTick = 0;

    // ---- State machine action handlers ----
    void Components_TempManagerStateMachine_action_doInit(
        SmId smId, Components_TempManagerStateMachine::Signal signal) override;
    void Components_TempManagerStateMachine_action_doRead(
        SmId smId, Components_TempManagerStateMachine::Signal signal) override;
    void Components_TempManagerStateMachine_action_doFaultRecovery(
        SmId smId, Components_TempManagerStateMachine::Signal signal) override;
    void Components_TempManagerStateMachine_action_doSimRead(
        SmId smId, Components_TempManagerStateMachine::Signal signal) override;

    // ---- Command handlers ----
    void run_handler(FwIndexType portNum, U32 context) override;
    void READ_TEMP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    Drv::I2cStatus readRawTemp(F32& temperature);
};

}  // namespace Components

#endif
