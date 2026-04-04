module Managers {

    @ Magnetometer Manager - reads PNI RM3100 magnetometer data over I2C
    active component MagnetometerManager {

        @ Command to trigger magnetometer calibration
        async command CALIBRATE_MAG opcode 0

        @ Enable simulation mode - generates fake mag field data without I2C
        async command ENABLE_SIM opcode 1

        @ Disable simulation mode - return to real I2C reads
        async command DISABLE_SIM opcode 2

        @ Magnetometer data read successfully
        event MagReadOk \
            severity activity low \
            format "Magnetometer data read successfully"

        @ Error reading magnetometer data
        event MagReadError(
            i2cStatus: I32 @< I2C status code
        ) severity warning high \
          format "Magnetometer read error, I2C status={}"

        @ Magnetometer calibration started
        event MagCalibrationStarted \
            severity activity high \
            format "Magnetometer calibration started"

        @ Simulation mode enabled
        event SimModeEnabled \
            severity activity high \
            format "MagnetometerManager: simulation mode enabled"

        @ Simulation mode disabled
        event SimModeDisabled \
            severity activity high \
            format "MagnetometerManager: simulation mode disabled"

        @ Magnetic field X component (microtesla)
        telemetry MagX: F32

        @ Magnetic field Y component (microtesla)
        telemetry MagY: F32

        @ Magnetic field Z component (microtesla)
        telemetry MagZ: F32

        @ Magnetometer temperature (degrees C)
        telemetry MagTemp: F32

        @ Port receiving calls from the rate group
        async input port run: Svc.Sched

        @ Health status reported to SystemManager each tick
        output port healthOut: Managers.ComponentHealth

        @ I2C write-then-read port for magnetometer communication
        output port busWriteRead: Drv.I2cWriteRead

        @ I2C write-only port for magnetometer configuration
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
