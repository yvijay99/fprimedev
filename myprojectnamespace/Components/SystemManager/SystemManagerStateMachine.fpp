module Managers {

    state machine SystemManagerStateMachine {

        initial enter NOMINAL

        signal tick           # rate group fires this every second
        signal sensorFault    # IMUManager reported an I2C failure
        signal tempFault      # TempManager reported an I2C failure
        signal gpsFault       # NavigationManager reported an I2C failure
        signal magFault       # MagnetometerManager reported an I2C failure
        signal faultCleared   # all reporting components are healthy again
        signal escalate       # too long in DEGRADED - promote to REBOOT
        signal rebootComplete # GDS cleared the fault, return to NOMINAL

        action runHealthCheck  # NOMINAL tick - emit telemetry, LED solid on
        action monitorDegraded # DEGRADED tick - emit telemetry, blink LED, check escalation
        action performReboot   # REBOOT tick - emit telemetry, LED off

        state NOMINAL {
            on tick do { runHealthCheck }
            on sensorFault enter DEGRADED
            on tempFault enter DEGRADED
            on gpsFault enter DEGRADED
            on magFault enter DEGRADED
        }

        state DEGRADED {
            on tick do { monitorDegraded }
            on faultCleared enter NOMINAL
            on escalate enter REBOOT
        }

        state REBOOT {
            on tick do { performReboot }
            on rebootComplete enter NOMINAL
        }

    }

}
