module Managers {

    @ System Manager - performs periodic health checks and reports system status
    active component SystemManager {

        @ Command to trigger an immediate health status report
        async command REPORT_STATUS opcode 0

        @ Periodic health check completed successfully
        event HealthCheckComplete \
            severity activity low \
            format "Health check completed"

        @ System health warning detected
        event HealthWarning(
            warningMsg: string size 80 @< Description of the warning
        ) severity warning high \
          format "Health warning: {}"

        @ CPU usage percentage
        telemetry CpuUsage: F32

        @ Memory usage percentage
        telemetry MemUsage: F32

        @ System uptime in seconds
        telemetry SystemUptime: U64

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
