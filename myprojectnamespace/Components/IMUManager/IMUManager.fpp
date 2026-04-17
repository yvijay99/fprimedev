module Managers {

    active component IMUManager {

        state machine instance imuSm: IMUManagerStateMachine

        async command CALIBRATE_IMU opcode 0

        async command ENABLE_SIM opcode 1

        async command DISABLE_SIM opcode 2

        enum SensorState { INIT, RUNNING, FAULT, SIM }

        event StateChange(newState: SensorState) \
            severity activity high \
            format "IMUManager: {}"

        event ImuCalibrationStarted \
            severity activity high \
            format "IMU calibration started"

        telemetry AccelX: F32
        telemetry AccelY: F32
        telemetry AccelZ: F32

        telemetry GyroX: F32
        telemetry GyroY: F32
        telemetry GyroZ: F32

        telemetry ImuTemp: F32

        async input port run: Svc.Sched

        output port healthOut: Managers.ComponentHealth

        output port busWriteRead: Drv.I2cWriteRead
        output port busWrite: Drv.I2c

        time get port timeCaller
        command reg port cmdRegOut
        command recv port cmdIn
        command resp port cmdResponseOut
        text event port logTextOut
        event port logOut
        telemetry port tlmOut
        param get port prmGetOut
        param set port prmSetOut
    }

}
