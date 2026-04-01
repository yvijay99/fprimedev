module Managers {

    state machine WatchdogStateMachine {

        initial enter NORMAL  # from tut - every SM needs a starting state

        signal tick    # from tut - rate group fires this every second
        signal fault   # added extra - triggered by INJECT_FAULT command
        signal recover # added extra - triggered by RECOVER command

        action checkHealth     # runs each tick while in NORMAL, writes telemetry mode=0
        action monitorRecovery # runs each tick while in SAFE_MODE, writes telemetry mode=1

        state NORMAL {
            on tick do { checkHealth } # from tut 
            on fault enter SAFE_MODE   # fault signal? go to safe mode
        }

        state SAFE_MODE {
            on tick do { monitorRecovery } # from tut
            on recover enter NORMAL # waiting for RECOVER command to come back
        }

    }

}
