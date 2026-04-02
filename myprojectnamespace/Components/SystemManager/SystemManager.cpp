// ======================================================================
// \title  SystemManager.cpp
// \author yuktivijay
// \brief  cpp file for SystemManager component implementation class
// ======================================================================

#include "myprojectnamespace/Components/SystemManager/SystemManager.hpp"

namespace Managers {

SystemManager::SystemManager(const char* const compName)
    : SystemManagerComponentBase(compName),
      m_uptimeSeconds(0),
      m_totalComponentFaults(0) {}

SystemManager::~SystemManager() {}

void SystemManager::run_handler(FwIndexType portNum, U32 context) {
    m_uptimeSeconds++;
    this->systemMgrSm_sendSignal_tick();
    this->dispatchCurrentMessages();
}

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
    if (this->isConnected_statusLedSet_OutputPort(0)) {
        this->statusLedSet_out(0, Fw::Logic::HIGH);
    }
}

void SystemManager::Managers_SystemManagerStateMachine_action_performReboot(
    SmId smId,
    Managers_SystemManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::systemMgrSm);
    this->tlmWrite_SystemState(1);  // 1 = REBOOT
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
    if (this->isConnected_statusLedSet_OutputPort(0)) {
        this->statusLedSet_out(0, Fw::Logic::LOW);
    }
}

void SystemManager::REPORT_STATUS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->tlmWrite_CpuUsage(0.0f);
    this->tlmWrite_MemUsage(0.0f);
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
    this->log_ACTIVITY_LO_HealthCheckComplete();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SystemManager::INJECT_COMPONENT_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_totalComponentFaults++;
    this->log_WARNING_HI_ComponentFaultDetected(m_totalComponentFaults);
    this->systemMgrSm_sendSignal_componentFault();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SystemManager::CLEAR_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_RebootComplete();
    this->systemMgrSm_sendSignal_rebootComplete();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Managers
