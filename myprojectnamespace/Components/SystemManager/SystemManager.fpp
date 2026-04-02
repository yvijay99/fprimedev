module Managers {

    @ System Manager - monitors component health and manages system state
    queued component SystemManager {

        @ State machine instance - drives NOMINAL/REBOOT transitions
        state machine instance systemMgrSm: SystemManagerStateMachine

        @ Rate-group driven periodic tick - drives the state machine
        async input port run: Svc.Sched

        @ GPIO output to drive the status LED (HIGH = NOMINAL, LOW = REBOOT)
        output port statusLedSet: Drv.GpioWrite

        @ Command to trigger an immediate health status report
        async command REPORT_STATUS opcode 0

        @ Inject a component fault (missed packet, data jump) - transitions to REBOOT
        async command INJECT_COMPONENT_FAULT opcode 1

        @ Clear the fault and return to NOMINAL after reboot
        async command CLEAR_FAULT opcode 2

        @ Periodic health check completed successfully
        event HealthCheckComplete \
            severity activity low \
            format "Health check completed"

        @ System health warning detected
        event HealthWarning(
            warningMsg: string size 80 @< Description of the warning
        ) severity warning high \
          format "Health warning: {}"

        @ Component fault triggered reboot mode
        event ComponentFaultDetected(
            faultCount: U32 @< Total component faults since startup
        ) severity warning high \
          format "Component fault detected - entering REBOOT mode (total faults: {})"

        @ System recovered from reboot back to nominal
        event RebootComplete \
            severity activity high \
            format "Reboot complete: returned to NOMINAL"

        @ Current system state: 0 = NOMINAL, 1 = REBOOT
        telemetry SystemState: U32

        @ CPU usage percentage
        telemetry CpuUsage: F32

        @ Memory usage percentage
        telemetry MemUsage: F32

        @ System uptime in seconds
        telemetry SystemUptime: U64

        @ Total component faults detected since startup
        telemetry TotalComponentFaults: U32

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
