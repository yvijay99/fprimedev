// ======================================================================
// \title  SystemManager.hpp
// \author yuktivijay
// \brief  hpp file for SystemManager component implementation class
// ======================================================================

#ifndef Managers_SystemManager_HPP
#define Managers_SystemManager_HPP

#include "myprojectnamespace/Components/SystemManager/SystemManagerComponentAc.hpp"

namespace Managers {

class SystemManager final : public SystemManagerComponentBase {
  public:
    SystemManager(const char* const compName);
    ~SystemManager();

  private:
    // ---- State machine action handlers ----

    //! In NOMINAL: emits health telemetry each tick (SystemState = 0)
    void Managers_SystemManagerStateMachine_action_runHealthCheck(
        SmId smId,
        Managers_SystemManagerStateMachine::Signal signal
    ) override;

    //! In REBOOT: emits reboot telemetry each tick (SystemState = 1)
    void Managers_SystemManagerStateMachine_action_performReboot(
        SmId smId,
        Managers_SystemManagerStateMachine::Signal signal
    ) override;

    // ---- Port handlers ----
    void run_handler(FwIndexType portNum, U32 context) override;

    // ---- Command handlers ----
    void REPORT_STATUS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void INJECT_COMPONENT_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void CLEAR_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    // ---- Member variables ----
    U64 m_uptimeSeconds = 0;
    U32 m_totalComponentFaults = 0;  //!< Total component faults since startup
};

}  // namespace Managers

#endif
