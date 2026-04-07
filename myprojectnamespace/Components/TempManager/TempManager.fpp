module Components {
    @ TMP102 temperature sensor component
    active component TempManager {

        @ State machine instance
        state machine instance tempSm: TempManagerStateMachine

        @ Read current temperature from TMP102 sensor
        async command READ_TEMP

        @ Enable simulation mode - generates fake temperature data without I2C
        async command ENABLE_SIM opcode 1

        @ Disable simulation mode - return to reading real I2C hardware
        async command DISABLE_SIM opcode 2

        @ Temperature measurement (deg C)
        telemetry Temperature: F32

        @ Port receiving calls from the rate group
        async input port run: Svc.Sched

        @ I2C write-read port for TMP
        output port busWriteRead: Drv.I2cWriteRead

        @ I2C write port for TMP
        output port busWrite: Drv.I2c

        @ Health status reported to SystemManager each tick
        output port healthOut: Managers.ComponentHealth

        @ Event for logging I2C read errors
        event TempReadError(status: Drv.I2cStatus) severity warning high format "I2C read error with status {}"

        @ Simulation mode enabled
        event SimModeEnabled \
            severity activity high \
            format "TempManager: simulation mode enabled"

        @ Simulation mode disabled
        event SimModeDisabled \
            severity activity high \
            format "TempManager: simulation mode disabled"

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

        @Port to set the value of a parameter
        param set port prmSetOut

    }
}
