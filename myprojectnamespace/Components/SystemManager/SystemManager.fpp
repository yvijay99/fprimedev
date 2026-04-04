module Managers {

    @ System Manager - monitors component health and manages system state
    queued component SystemManager {

        @ State machine instance
        state machine instance systemMgrSm: SystemManagerStateMachine

        @ Rate-group driven periodic tick
        async input port run: Svc.Sched

        @ Health report from SensorManager (IMU)
        async input port sensorHealth: Managers.ComponentHealth

        @ Health report from TempManager (TMP102)
        async input port tempHealth: Managers.ComponentHealth

        @ Health report from NavigationManager (GPS)
        async input port gpsHealth: Managers.ComponentHealth

        @ Health report from MagnetometerManager (RM3100)
        async input port magHealth: Managers.ComponentHealth

        @ GPIO output to drive the status LED
        output port statusLedSet: Drv.GpioWrite

        @ Trigger an immediate health status report
        async command REPORT_STATUS opcode 0

        @ Manually inject a component fault - transitions NOMINAL/DEGRADED → REBOOT
        async command INJECT_COMPONENT_FAULT opcode 1

        @ Clear a fault and return to NOMINAL
        async command CLEAR_FAULT opcode 2

        @ Manually escalate from DEGRADED to REBOOT
        async command ESCALATE opcode 3

        event HealthCheckComplete \
            severity activity low \
            format "Health check completed"

        event EnteredDegraded(
            faultCount: U32
        ) severity warning high \
          format "System degraded - component fault detected (total faults: {})"

        event EscalatedToReboot \
            severity warning high \
            format "Fault persisted - escalating to REBOOT"

        event ComponentFaultDetected(
            faultCount: U32
        ) severity warning high \
          format "Component fault detected - entering REBOOT mode (total faults: {})"

        event RebootComplete \
            severity activity high \
            format "Reboot complete: returned to NOMINAL"

        @ Current system state: 0 = NOMINAL, 1 = DEGRADED, 2 = REBOOT
        telemetry SystemState: U32

        telemetry CpuUsage: F32
        telemetry MemUsage: F32
        telemetry SystemUptime: U64
        telemetry TotalComponentFaults: U32

        @ How many ticks the system has been in DEGRADED
        telemetry DegradedTicks: U32

        ###############################################################################
        # Standard AC Ports: Required for Channels, Events, Commands, and Parameters  #
        ###############################################################################
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
