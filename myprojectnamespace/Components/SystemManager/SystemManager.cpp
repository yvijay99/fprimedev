// ======================================================================
// \title  SystemManager.cpp
// \author yuktivijay
// \brief  System Manager - component health monitor with NOMINAL/REBOOT state machine
//
// Usage:
//   Send INJECT_COMPONENT_FAULT → transitions NOMINAL → REBOOT
//     (simulates a missed packet or unexpected data jump from a component)
//   Send CLEAR_FAULT            → transitions REBOOT → NOMINAL
//
// The rate group tick drives the state machine each second and emits
// telemetry so you can see SystemState (0=NOMINAL, 1=REBOOT) in the GDS.
// Not sure if physical reboot can actually be triggered from here yet,
// but this expands the state machine module to cover that case.
// ======================================================================

#include "myprojectnamespace/Components/SystemManager/SystemManager.hpp"

namespace Managers {

SystemManager::SystemManager(const char* const compName)
    : SystemManagerComponentBase(compName),
      m_uptimeSeconds(0),
      m_totalComponentFaults(0) {}

SystemManager::~SystemManager() {}

// Rate group handler - drives the state machine and increments uptime

void SystemManager::run_handler(FwIndexType portNum, U32 context) {
    m_uptimeSeconds++;
    this->systemMgrSm_sendSignal_tick();
    this->dispatchCurrentMessages();
}

// State machine action: runHealthCheck (NOMINAL, called on every tick)
// Emits telemetry showing the system is running normally.

void SystemManager::Managers_SystemManagerStateMachine_action_runHealthCheck(
    SmId smId,
    Managers_SystemManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::systemMgrSm);
    this->tlmWrite_SystemState(0);  // 0 = NOMINAL
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_CpuUsage(0.0f);
    this->tlmWrite_MemUsage(0.0f);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
    this->log_ACTIVITY_LO_HealthCheckComplete();
}

// State machine action: performReboot (REBOOT, called on every tick)
// Emits telemetry showing the system is in reboot/recovery mode.

void SystemManager::Managers_SystemManagerStateMachine_action_performReboot(
    SmId smId,
    Managers_SystemManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::systemMgrSm);
    this->tlmWrite_SystemState(1);  // 1 = REBOOT
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
}

// Command: REPORT_STATUS - dump current health to GDS on demand

void SystemManager::REPORT_STATUS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->tlmWrite_CpuUsage(0.0f);
    this->tlmWrite_MemUsage(0.0f);
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
    this->log_ACTIVITY_LO_HealthCheckComplete();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// Command: INJECT_COMPONENT_FAULT - simulates a missed packet or data jump
// Transitions NOMINAL → REBOOT. Commanded from GDS for now.

void SystemManager::INJECT_COMPONENT_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_totalComponentFaults++;
    this->log_WARNING_HI_ComponentFaultDetected(m_totalComponentFaults);
    this->systemMgrSm_sendSignal_componentFault();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// Command: CLEAR_FAULT - clears the fault and returns to NOMINAL
// Transitions REBOOT → NOMINAL.

void SystemManager::CLEAR_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_RebootComplete();
    this->systemMgrSm_sendSignal_rebootComplete();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Managers
