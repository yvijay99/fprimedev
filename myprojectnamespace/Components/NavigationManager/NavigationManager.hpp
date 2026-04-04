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
    void configure();

  private:
    bool m_hasFix = false;
    bool m_simEnabled = false;
    static constexpr U8 MIN_SATELLITES_FOR_FIX = 4;
    void GPS_RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void run_handler(FwIndexType portNum, U32 context) override;
    Drv::I2cStatus readGpsData(F64& lat, F64& lon, F32& alt, F32& speed, U8& numSats);
};

}  // namespace Managers

#endif
