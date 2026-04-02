module Managers {

    @ Sensor Manager - reads ICM-20649 IMU (accelerometer + gyroscope) data over I2C
    active component SensorManager {

        @ Command to trigger IMU calibration
        async command CALIBRATE_IMU opcode 0

        @ Enable simulation mode - generates fake IMU + temp data without real hardware
        async command ENABLE_SIM opcode 1

        @ Disable simulation mode - return to reading real I2C hardware
        async command DISABLE_SIM opcode 2

        @ IMU data read successfully
        event ImuReadOk \
            severity activity low \
            format "IMU data read successfully"

        @ Error reading IMU data
        event ImuReadError(
            i2cStatus: I32 @< I2C status code
        ) severity warning high \
          format "IMU read error, I2C status={}"

        @ IMU calibration started
        event ImuCalibrationStarted \
            severity activity high \
            format "IMU calibration started"

        @ Simulation mode enabled - IMU and temp data are simulated
        event SimModeEnabled \
            severity activity high \
            format "SensorManager: simulation mode enabled"

        @ Simulation mode disabled - returning to real I2C hardware
        event SimModeDisabled \
            severity activity high \
            format "SensorManager: simulation mode disabled"

        @ Acceleration vector (m/s^2)
        telemetry AccelX: F32
        telemetry AccelY: F32
        telemetry AccelZ: F32

        @ Angular rate vector (deg/s)
        telemetry GyroX: F32
        telemetry GyroY: F32
        telemetry GyroZ: F32

        @ IMU temperature (degrees C)
        telemetry ImuTemp: F32

        @ Port receiving calls from the rate group
        async input port run: Svc.Sched

        @ I2C write-then-read port for IMU communication
        output port busWriteRead: Drv.I2cWriteRead

        @ I2C write-only port for IMU configuration
        output port busWrite: Drv.I2c

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending textual representation of events
        text event port logTextOut

        @ Port for sending events to downlink
        event port logOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @ Port to set the value of a parameter
        param set port prmSetOut
    }

}
