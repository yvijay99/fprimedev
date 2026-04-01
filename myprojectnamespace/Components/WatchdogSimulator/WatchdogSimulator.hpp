// ======================================================================
// \title  WatchdogSimulator.hpp
// \author yuktivijay
// \brief  Watchdog Simulator - fault injection driven state machine demo
// ======================================================================

#ifndef Managers_WatchdogSimulator_HPP
#define Managers_WatchdogSimulator_HPP

#include "myprojectnamespace/Components/WatchdogSimulator/WatchdogSimulatorComponentAc.hpp"

namespace Managers {

class WatchdogSimulator final : public WatchdogSimulatorComponentBase {
  public:
    explicit WatchdogSimulator(const char* const compName);
    ~WatchdogSimulator();

  private:
    // ---- State machine action handlers ----

    //! In NORMAL: emits telemetry each tick to show the system is running normally
    void Managers_WatchdogStateMachine_action_checkHealth(
        SmId smId,
        Managers_WatchdogStateMachine::Signal signal
    ) override;

    //! In SAFE_MODE: emits telemetry each tick to show safe mode is active
    void Managers_WatchdogStateMachine_action_monitorRecovery(
        SmId smId,
        Managers_WatchdogStateMachine::Signal signal
    ) override;

    // ---- Port handlers ----
    void run_handler(FwIndexType portNum, U32 context) override;

    // ---- Command handlers ----
    void INJECT_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void RECOVER_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    // ---- Member variables ----
    U32 m_totalFaults;  //!< Total faults injected since startup
};

}  // namespace Managers

#endif  // Managers_WatchdogSimulator_HPP
