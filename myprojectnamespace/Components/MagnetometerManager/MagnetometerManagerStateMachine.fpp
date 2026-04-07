module Managers {

    state machine MagnetometerManagerStateMachine {

        initial enter INIT

        signal tick
        signal success
        signal fault
        signal enableSim
        signal disableSim

        action doInit
        action doRead
        action doFaultRecovery
        action doSimRead

        state INIT {
            on tick do { doInit }
            on success enter RUNNING
            on fault enter FAULT
            on enableSim enter SIM
        }

        state RUNNING {
            on tick do { doRead }
            on fault enter FAULT
            on enableSim enter SIM
        }

        state FAULT {
            on tick do { doFaultRecovery }
            on success enter RUNNING
            on enableSim enter SIM
        }

        state SIM {
            on tick do { doSimRead }
            on disableSim enter INIT
        }

    }

}
