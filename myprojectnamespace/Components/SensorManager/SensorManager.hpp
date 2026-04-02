// ======================================================================
// \title  SensorManager.hpp
// \author yuktivijay
// \brief  hpp file for SensorManager component implementation class
// ======================================================================

#ifndef Managers_SensorManager_HPP
#define Managers_SensorManager_HPP

#include "myprojectnamespace/Components/SensorManager/SensorManagerComponentAc.hpp"

namespace Managers {

class SensorManager final : public SensorManagerComponentBase {
  public:
    SensorManager(const char* const compName);
    ~SensorManager();

    void configure(U32 i2cAddress);

  private:
    U32 m_i2cAddress = 0x68;

    static constexpr U8 REG_BANK_SEL = 0x7F;
    static constexpr U8 WHO_AM_I = 0x00;
    static constexpr U8 ACCEL_XOUT_H = 0x2D;
    static constexpr U8 GYRO_XOUT_H = 0x33;
    static constexpr U8 TEMP_OUT_H = 0x39;
    static constexpr U8 PWR_MGMT_1 = 0x06;
    static constexpr U8 DATA_SIZE = 14;
    static constexpr U8 ICM20649_WHO_AM_I = 0xE1;

    bool m_simEnabled = false;  //!< When true, generates fake data instead of reading I2C
    U32  m_simTick = 0;         //!< Tick counter used to vary simulated sensor values

    void CALIBRATE_IMU_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void run_handler(FwIndexType portNum, U32 context) override;

    Drv::I2cStatus readImuData(F32& ax, F32& ay, F32& az,
                                F32& gx, F32& gy, F32& gz,
                                F32& temp);

    //! Fills in fake IMU + temp values for testing state machine without hardware.
    //! Values oscillate slowly so you can see live changes in GDS.
    void simulateImuData(F32& ax, F32& ay, F32& az,
                         F32& gx, F32& gy, F32& gz,
                         F32& temp);
};

}  // namespace Managers

#endif
