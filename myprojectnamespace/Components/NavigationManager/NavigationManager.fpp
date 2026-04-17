module Managers {

    active component NavigationManager {

        state machine instance navSm: NavigationManagerStateMachine

        async command GPS_RESET opcode 0

        async command ENABLE_SIM opcode 1

        async command DISABLE_SIM opcode 2

        enum SensorState { INIT, RUNNING, FAULT, SIM }

        event StateChange(newState: SensorState) \
            severity activity high \
            format "NavigationManager: {}"

        event GpsFixAcquired(
            numSats: U8
        ) severity activity high \
          format "GPS fix acquired with {} satellites"

        event GpsFixLost \
            severity warning high \
            format "GPS fix lost"

        event GpsReading(lat: F64, lon: F64, alt: F32, sats: U8) \
            severity activity low \
            format "GPS reading: lat={f}, lon={f}, alt={f} m, sats={}"

        telemetry Latitude: F64
        telemetry Longitude: F64
        telemetry Altitude: F32
        telemetry GroundSpeed: F32
        telemetry NumSatellites: U8

        async input port run: Svc.Sched

        output port busWriteRead: Drv.I2cWriteRead
        output port busWrite: Drv.I2c

        output port healthOut: Managers.ComponentHealth

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
