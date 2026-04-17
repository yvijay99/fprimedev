module Components {

    active component TempManager {

        state machine instance tempSm: TempManagerStateMachine

        async command READ_TEMP

        async command ENABLE_SIM opcode 1

        async command DISABLE_SIM opcode 2

        telemetry Temperature: F32

        async input port run: Svc.Sched

        output port busWriteRead: Drv.I2cWriteRead
        output port busWrite: Drv.I2c

        output port healthOut: Managers.ComponentHealth

        enum SensorState { INIT, RUNNING, FAULT, SIM }

        event StateChange(newState: SensorState) \
            severity activity high \
            format "TempManager: {}"

        event TempReading(temperature: F32) \
            severity activity low \
            format "Temp reading: {f} C"

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
