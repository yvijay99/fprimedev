module Components {
    @ TMP 1 sensor component
    active component TempManager {

        # One async command/port is required for active components
        # This should be overridden by the developers with a useful command/port

        @ Read current temperature from TMP102 sensor
        async command READ_TEMP

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