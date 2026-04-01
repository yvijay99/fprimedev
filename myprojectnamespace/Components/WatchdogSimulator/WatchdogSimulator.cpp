// ======================================================================
// \title  WatchdogSimulator.cpp
// \author yuktivijay
// \brief  Watchdog Simulator - fault injection driven state machine demo
//
// Usage:
//   Send INJECT_FAULT  → state machine transitions NORMAL → SAFE_MODE
//   Send RECOVER       → state machine transitions SAFE_MODE → NORMAL
//
// The rate group tick just keeps the state machine alive and emits
// telemetry so you can see the current mode in the GDS.
// ======================================================================

#include "myprojectnamespace/Components/WatchdogSimulator/WatchdogSimulator.hpp"

namespace Managers {

WatchdogSimulator::WatchdogSimulator(const char* const compName)
    : WatchdogSimulatorComponentBase(compName),
      m_totalFaults(0) {}

WatchdogSimulator::~WatchdogSimulator() {}

// Rate group handler - drives the state machine each tick

void WatchdogSimulator::run_handler(FwIndexType portNum, U32 context) {
    this->watchdogSm_sendSignal_tick();
    this->dispatchCurrentMessages();
}

// State machine action: checkHealth  (NORMAL, called on every tick)
// emits telemetry so you can see mode = 0 in the GDS.

void WatchdogSimulator::Managers_WatchdogStateMachine_action_checkHealth(
    SmId smId,
    Managers_WatchdogStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::watchdogSm);
    this->tlmWrite_OperatingMode(0);   // 0 = NORMAL
    this->tlmWrite_TotalFaults(m_totalFaults);
}

// State machine action: monitorRecovery  (SAFE_MODE, called on every tick)
// emits telemetry mode = 1 in the GDS.

void WatchdogSimulator::Managers_WatchdogStateMachine_action_monitorRecovery(
    SmId smId,
    Managers_WatchdogStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::watchdogSm);
    this->tlmWrite_OperatingMode(1);   // 1 = SAFE_MODE
    this->tlmWrite_TotalFaults(m_totalFaults);
}

// Command: INJECT_FAULT - trigger SAFE_MODE right now

void WatchdogSimulator::INJECT_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_totalFaults++;
    this->log_WARNING_HI_EnteredSafeMode(m_totalFaults);
    this->watchdogSm_sendSignal_fault();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// Command: RECOVER - return to NORMAL from SAFE_MODE

void WatchdogSimulator::RECOVER_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_RecoveredToNormal();
    this->watchdogSm_sendSignal_recover();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Managers
