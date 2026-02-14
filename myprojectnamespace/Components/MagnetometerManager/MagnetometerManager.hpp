// ======================================================================
// \title  MagnetometerManager.hpp
// \author yuktivijay
// \brief  hpp file for MagnetometerManager component implementation class
// ======================================================================

#ifndef Managers_MagnetometerManager_HPP
#define Managers_MagnetometerManager_HPP

#include "myprojectnamespace/Components/MagnetometerManager/MagnetometerManagerComponentAc.hpp"

namespace Managers {

class MagnetometerManager final : public MagnetometerManagerComponentBase {
  public:
    MagnetometerManager(const char* const compName);
    ~MagnetometerManager();

    void configure(U32 i2cAddress);

  private:
    U32 m_i2cAddress = 0x20;

    static constexpr U8 POLL_REG = 0x00;
    static constexpr U8 CMM_REG = 0x01;
    static constexpr U8 CCX_REG = 0x04;
    static constexpr U8 CCY_REG = 0x06;
    static constexpr U8 CCZ_REG = 0x08;
    static constexpr U8 MX_REG = 0x24;
    static constexpr U8 MY_REG = 0x27;
    static constexpr U8 MZ_REG = 0x2A;
    static constexpr U8 STATUS_REG = 0x34;
    static constexpr U8 MAG_DATA_SIZE = 9;
    static constexpr F32 RM3100_SENSITIVITY = 75.0f;

    void CALIBRATE_MAG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void run_handler(FwIndexType portNum, U32 context) override;

    Drv::I2cStatus readMagData(F32& mx, F32& my, F32& mz);
    Drv::I2cStatus readTemperature(F32& tempC);
};

}  // namespace Managers

#endif
