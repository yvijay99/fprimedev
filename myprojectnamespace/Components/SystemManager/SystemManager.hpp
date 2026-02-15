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
    U64 m_uptimeSeconds = 0;

    void REPORT_STATUS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void run_handler(FwIndexType portNum, U32 context) override;
};

}  // namespace Managers

#endif
