// IMUManager.hpp

#ifndef Managers_IMUManager_HPP
#define Managers_IMUManager_HPP

#include "myprojectnamespace/Components/IMUManager/IMUManagerComponentAc.hpp"

namespace Managers {

class IMUManager final : public IMUManagerComponentBase {
  public:
    IMUManager(const char* const compName);
    ~IMUManager();

    void configure(U32 i2cAddress);

  private:
    U32 m_i2cAddress = 0x68;

    static constexpr U8 ACCEL_XOUT_H = 0x2D;
    static constexpr U8 DATA_SIZE = 14;

    U32 m_simTick = 0;

    void Managers_IMUManagerStateMachine_action_doInit(
        SmId smId, Managers_IMUManagerStateMachine::Signal signal) override;
    void Managers_IMUManagerStateMachine_action_doRead(
        SmId smId, Managers_IMUManagerStateMachine::Signal signal) override;
    void Managers_IMUManagerStateMachine_action_doFaultRecovery(
        SmId smId, Managers_IMUManagerStateMachine::Signal signal) override;
    void Managers_IMUManagerStateMachine_action_doSimRead(
        SmId smId, Managers_IMUManagerStateMachine::Signal signal) override;

    void CALIBRATE_IMU_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void run_handler(FwIndexType portNum, U32 context) override;

    Drv::I2cStatus readImuData(F32& ax, F32& ay, F32& az,
                                F32& gx, F32& gy, F32& gz,
                                F32& temp);
    void simulateImuData(F32& ax, F32& ay, F32& az,
                         F32& gx, F32& gy, F32& gz,
                         F32& temp);
};

}  // namespace Managers

#endif
