// ======================================================================
// \title  NavigationManager.hpp
// \author yuktivijay
// \brief  hpp file for NavigationManager component implementation class
// ======================================================================

#ifndef Managers_NavigationManager_HPP
#define Managers_NavigationManager_HPP

#include "myprojectnamespace/Components/NavigationManager/NavigationManagerComponentAc.hpp"

namespace Managers {

class NavigationManager final : public NavigationManagerComponentBase {
  public:
    NavigationManager(const char* const compName);
    ~NavigationManager();

  private:
    bool m_hasFix = false;
    static constexpr U8 MIN_SATELLITES_FOR_FIX = 4;

    void GPS_RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void run_handler(FwIndexType portNum, U32 context) override;
};

}  // namespace Managers

#endif
