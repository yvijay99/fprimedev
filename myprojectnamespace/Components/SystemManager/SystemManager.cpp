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
      m_totalComponentFaults(0),
      m_degradedTicks(0),
      m_sensorFaultActive(false),
      m_tempFaultActive(false),
      m_gpsFaultActive(false),
      m_magFaultActive(false),
      m_ledToggle(false) {}

SystemManager::~SystemManager() {}

void SystemManager::run_handler(FwIndexType portNum, U32 context) {
    m_uptimeSeconds++;
    this->systemMgrSm_sendSignal_tick();
    this->dispatchCurrentMessages();
}

// ---- State machine actions ----

void SystemManager::Managers_SystemManagerStateMachine_action_runHealthCheck(
    SmId smId,
    Managers_SystemManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::systemMgrSm);
    m_degradedTicks = 0;
    this->tlmWrite_SystemState(0);  // 0 = NOMINAL
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_CpuUsage(0.0f);
    this->tlmWrite_MemUsage(0.0f);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
    this->tlmWrite_DegradedTicks(0);
    this->log_ACTIVITY_LO_HealthCheckComplete();
    if (this->isConnected_statusLedSet_OutputPort(0)) {
        this->statusLedSet_out(0, Fw::Logic::HIGH);
    }
}

void SystemManager::Managers_SystemManagerStateMachine_action_monitorDegraded(
    SmId smId,
    Managers_SystemManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::systemMgrSm);
    m_degradedTicks++;
    this->tlmWrite_SystemState(1);  // 1 = DEGRADED
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
    this->tlmWrite_DegradedTicks(m_degradedTicks);
    // Blink LED at 1Hz in degraded state
    m_ledToggle = !m_ledToggle;
    if (this->isConnected_statusLedSet_OutputPort(0)) {
        this->statusLedSet_out(0, m_ledToggle ? Fw::Logic::HIGH : Fw::Logic::LOW);
    }
    // Auto-escalate if fault persists too long
    if (m_degradedTicks >= ESCALATION_THRESHOLD) {
        this->systemMgrSm_sendSignal_escalate();
    }
}

void SystemManager::Managers_SystemManagerStateMachine_action_performReboot(
    SmId smId,
    Managers_SystemManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::systemMgrSm);
    this->tlmWrite_SystemState(2);  // 2 = REBOOT
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
    if (this->isConnected_statusLedSet_OutputPort(0)) {
        this->statusLedSet_out(0, Fw::Logic::LOW);
    }
}

// ---- Health port handlers ----

void SystemManager::sensorHealth_handler(FwIndexType portNum, bool healthy) {
    if (!healthy && !m_sensorFaultActive) {
        m_sensorFaultActive = true;
        m_totalComponentFaults++;
        this->log_WARNING_HI_EnteredDegraded(m_totalComponentFaults);
        this->systemMgrSm_sendSignal_sensorFault();
    } else if (healthy && m_sensorFaultActive) {
        m_sensorFaultActive = false;
        if (!m_tempFaultActive && !m_gpsFaultActive && !m_magFaultActive) {
            this->systemMgrSm_sendSignal_faultCleared();
        }
    }
}

void SystemManager::tempHealth_handler(FwIndexType portNum, bool healthy) {
    if (!healthy && !m_tempFaultActive) {
        m_tempFaultActive = true;
        m_totalComponentFaults++;
        this->log_WARNING_HI_EnteredDegraded(m_totalComponentFaults);
        this->systemMgrSm_sendSignal_tempFault();
    } else if (healthy && m_tempFaultActive) {
        m_tempFaultActive = false;
        if (!m_sensorFaultActive && !m_gpsFaultActive && !m_magFaultActive) {
            this->systemMgrSm_sendSignal_faultCleared();
        }
    }
}

void SystemManager::gpsHealth_handler(FwIndexType portNum, bool healthy) {
    if (!healthy && !m_gpsFaultActive) {
        m_gpsFaultActive = true;
        m_totalComponentFaults++;
        this->log_WARNING_HI_EnteredDegraded(m_totalComponentFaults);
        this->systemMgrSm_sendSignal_gpsFault();
    } else if (healthy && m_gpsFaultActive) {
        m_gpsFaultActive = false;
        if (!m_sensorFaultActive && !m_tempFaultActive && !m_magFaultActive) {
            this->systemMgrSm_sendSignal_faultCleared();
        }
    }
}

void SystemManager::magHealth_handler(FwIndexType portNum, bool healthy) {
    if (!healthy && !m_magFaultActive) {
        m_magFaultActive = true;
        m_totalComponentFaults++;
        this->log_WARNING_HI_EnteredDegraded(m_totalComponentFaults);
        this->systemMgrSm_sendSignal_magFault();
    } else if (healthy && m_magFaultActive) {
        m_magFaultActive = false;
        if (!m_sensorFaultActive && !m_tempFaultActive && !m_gpsFaultActive) {
            this->systemMgrSm_sendSignal_faultCleared();
        }
    }
}

// ---- Command handlers ----

void SystemManager::REPORT_STATUS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->tlmWrite_CpuUsage(0.0f);
    this->tlmWrite_MemUsage(0.0f);
    this->tlmWrite_SystemUptime(m_uptimeSeconds);
    this->tlmWrite_TotalComponentFaults(m_totalComponentFaults);
    this->tlmWrite_DegradedTicks(m_degradedTicks);
    this->log_ACTIVITY_LO_HealthCheckComplete();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SystemManager::INJECT_COMPONENT_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_totalComponentFaults++;
    this->log_WARNING_HI_ComponentFaultDetected(m_totalComponentFaults);
    this->systemMgrSm_sendSignal_sensorFault();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SystemManager::ESCALATE_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_WARNING_HI_EscalatedToReboot();
    this->systemMgrSm_sendSignal_escalate();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SystemManager::CLEAR_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    m_sensorFaultActive = false;
    m_tempFaultActive = false;
    m_gpsFaultActive = false;
    m_magFaultActive = false;
    m_degradedTicks = 0;
    this->log_ACTIVITY_HI_RebootComplete();
    this->systemMgrSm_sendSignal_rebootComplete();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Managers
