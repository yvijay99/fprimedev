module Managers {

    @ Watchdog Simulator
    queued component WatchdogSimulator { # from tut - queued so rate group drives everything

        state machine instance watchdogSm: WatchdogStateMachine # from tut - hooks up our SM

        @ Rate-group driven periodic tick - drives the state machine
        async input port run: Svc.Sched # from tut - same as Led and SystemManager

        @ Inject a fault - immediately transitions to SAFE_MODE
        async command INJECT_FAULT opcode 0 # added extra - the button you press in GDS

        @ Recover from SAFE_MODE - return to NORMAL
        async command RECOVER opcode 1 # added extra - brings it back

        @ Watchdog entered SAFE_MODE due to injected fault
        event EnteredSafeMode(
            faultCount: U32 @< Total faults triggered since startup
        ) severity warning high \
          format "Watchdog entered SAFE MODE (total faults: {})"

        @ Watchdog recovered and returned to NORMAL mode
        event RecoveredToNormal \
            severity activity high \
            format "Watchdog recovered: NORMAL mode restored"

        @ Current operating mode: 0 = NORMAL, 1 = SAFE_MODE
        telemetry OperatingMode: U32

        @ Total number of faults injected since startup
        telemetry TotalFaults: U32

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
