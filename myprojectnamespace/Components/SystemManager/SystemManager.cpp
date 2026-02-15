// ======================================================================
// \title  SystemManager.cpp
// \author yuktivijay
// \brief  cpp file for SystemManager component implementation class
// ======================================================================

#include "myprojectnamespace/Components/SystemManager/SystemManager.hpp"

namespace Managers {

SystemManager::SystemManager(const char* const compName) : SystemManagerComponentBase(compName) {}

SystemManager::~SystemManager() {}

void SystemManager::REPORT_STATUS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->tlmWrite_CpuUsage(0.0f);
    this->tlmWrite_MemUsage(0.0f);
    this->tlmWrite_SystemUptime(this->m_uptimeSeconds);
    this->log_ACTIVITY_LO_HealthCheckComplete();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void SystemManager::run_handler(FwIndexType portNum, U32 context) {
    this->m_uptimeSeconds++;
    this->tlmWrite_SystemUptime(this->m_uptimeSeconds);
    this->tlmWrite_CpuUsage(0.0f);
    this->tlmWrite_MemUsage(0.0f);
    this->log_ACTIVITY_LO_HealthCheckComplete();
}

}  // namespace Managers
