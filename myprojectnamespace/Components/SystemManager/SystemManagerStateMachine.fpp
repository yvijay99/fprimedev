module Managers {

    state machine SystemManagerStateMachine {

        initial enter NOMINAL

        signal tick           # rate group fires this every second
        signal componentFault # triggered when a component misbehaves (missed packet, data jump)
        signal rebootComplete # triggered from GDS once fault is cleared

        action runHealthCheck # runs each tick in NOMINAL - emits health telemetry, mode=0
        action performReboot  # runs each tick in REBOOT  - emits reboot telemetry, mode=1

        state NOMINAL {
            on tick do { runHealthCheck }
            on componentFault enter REBOOT
        }

        state REBOOT {
            on tick do { performReboot }
            on rebootComplete enter NOMINAL
        }

    }

}
