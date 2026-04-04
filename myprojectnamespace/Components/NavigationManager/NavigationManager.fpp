module Managers {

    @ Navigation Manager - reads GPS position and velocity data (stubbed for now)
    active component NavigationManager {

        @ Command to reset the GPS module
        async command GPS_RESET opcode 0

        @ GPS fix acquired
        event GpsFixAcquired(
            numSats: U8 @< Number of satellites in view
        ) severity activity high \
          format "GPS fix acquired with {} satellites"

        @ GPS fix lost
        event GpsFixLost \
            severity warning high \
            format "GPS fix lost"

        @ Latitude in degrees
        telemetry Latitude: F64

        @ Longitude in degrees
        telemetry Longitude: F64

        @ Altitude in meters
        telemetry Altitude: F32

        @ Ground speed in m/s
        telemetry GroundSpeed: F32

        @ Number of satellites in view
        telemetry NumSatellites: U8

        @ Enable simulation mode - returns hardcoded GPS coordinates without I2C
        async command ENABLE_SIM opcode 1

        @ Disable simulation mode - return to real I2C GPS reads
        async command DISABLE_SIM opcode 2

        @ Simulation mode enabled
        event SimModeEnabled \
            severity activity high \
            format "NavigationManager: simulation mode enabled"

        @ Simulation mode disabled
        event SimModeDisabled \
            severity activity high \
            format "NavigationManager: simulation mode disabled"

        @ Port receiving calls from the rate group
        async input port run: Svc.Sched

        @ I2C write-read port for GPS
        output port busWriteRead: Drv.I2cWriteRead

        @ I2C write port for GPS
        output port busWrite: Drv.I2c

        @ Health status reported to SystemManager each tick
        output port healthOut: Managers.ComponentHealth

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
