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

        @ Port receiving calls from the rate group
        async input port run: Svc.Sched

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
