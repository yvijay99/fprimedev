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

    void Managers_SystemManagerStateMachine_action_runHealthCheck(
        SmId smId,
        Managers_SystemManagerStateMachine::Signal signal
    ) override;

    void Managers_SystemManagerStateMachine_action_monitorDegraded(
        SmId smId,
        Managers_SystemManagerStateMachine::Signal signal
    ) override;

    void Managers_SystemManagerStateMachine_action_performReboot(
        SmId smId,
        Managers_SystemManagerStateMachine::Signal signal
    ) override;

    // ---- Port handlers ----
    void run_handler(FwIndexType portNum, U32 context) override;
    void sensorHealth_handler(FwIndexType portNum, bool healthy) override;
    void tempHealth_handler(FwIndexType portNum, bool healthy) override;

    // ---- Command handlers ----
    void REPORT_STATUS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void INJECT_COMPONENT_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void CLEAR_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void ESCALATE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    // ---- Member variables ----
    U64 m_uptimeSeconds = 0;
    U32 m_totalComponentFaults = 0;
    U32 m_degradedTicks = 0;       //!< Ticks spent in DEGRADED - escalates at threshold
    bool m_sensorFaultActive = false;
    bool m_tempFaultActive = false;
    bool m_ledToggle = false;      //!< Used to blink LED in DEGRADED state

    static constexpr U32 ESCALATION_THRESHOLD = 10;  //!< Ticks in DEGRADED before auto-escalate
};

}  // namespace Managers

#endif
