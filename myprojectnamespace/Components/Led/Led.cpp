// ======================================================================
// \title  Led.cpp
// \author yuktivijay
// \brief  cpp file for Led component implementation class
// ======================================================================

#include "myprojectnamespace/Components/Led/Led.hpp"

namespace ledmanager {

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

Led ::Led(const char* const compName) : LedComponentBase(compName) {}

Led ::~Led() {}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void Led ::BLINKING_ON_OFF_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, Fw::On onOff) {
    this->m_toggleCounter = 0;               // Reset count on any successful command
    this->m_blinking = onOff;  // Update blinking state
    // TODO: Emit an event SetBlinkingState to report the blinking state (onOff).
    // NOTE: This event will be added during the "Events" exercise.
    this -> log_ACTIVITY_HI_SetBlinkingState(onOff);
    // TODO: Report the blinking state (onOff) on channel BlinkingState.
    this -> tlmWrite_BlinkingState(onOff);
    this -> tlmWrite_BlinkCommandCount(++this ->m_toggleCounter);
    // NOTE: This telemetry channel will be added during the "Telemetry" exercise.
    // Provide command response
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void Led ::parameterUpdated(FwPrmIdType id) {
    Fw::ParamValid isValid = Fw::ParamValid::INVALID;
    switch (id) {
        case PARAMID_BLINK_INTERVAL: {
            // Read back the parameter value
            const U32 interval = this->paramGet_BLINK_INTERVAL(isValid);
            // NOTE: isValid is always VALID in parameterUpdated as it was just properly set
            FW_ASSERT(isValid == Fw::ParamValid::VALID, static_cast<FwAssertArgType>(isValid));

            // Emit the blink interval set event
            this-> log_ACTIVITY_HI_BlinkIntervalSet(interval);
            // TODO: Emit an event with, severity activity high, named BlinkIntervalSet that takes in an argument of
            // type U32 to report the blink interval.
            break;
        }
        default:
            FW_ASSERT(0, static_cast<FwAssertArgType>(id));
            break;
    }
}
// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void Led ::run_handler(FwIndexType portNum, U32 context) {
    // Read back the parameter value
    Fw::ParamValid isValid = Fw::ParamValid::INVALID;
    U32 interval = this->paramGet_BLINK_INTERVAL(isValid);
    FW_ASSERT((isValid != Fw::ParamValid::INVALID) && (isValid != Fw::ParamValid::UNINIT),
              static_cast<FwAssertArgType>(isValid));

    // Only perform actions when set to blinking
    if (this->m_blinking && (interval != 0)) {
        // If toggling state
        if (this->m_toggleCounter == 0) {
            // Toggle state
            this->m_state = (this->m_state == Fw::On::ON) ? Fw::On::OFF : Fw::On::ON;
            this->m_transitions++;
            // TODO: Report the number of LED transitions (this->m_transitions) on channel LedTransitions
            this->tlmWrite_LEDTransitions(this->m_transitions);
            // Port may not be connected, so check before sending output
            if (this->isConnected_gpioSet_OutputPort(0)) {
                this->gpioSet_out(0, (Fw::On::ON == this->m_state) ? Fw::Logic::HIGH : Fw::Logic::LOW);
            }

            // TODO: Emit an event LedState to report the LED state (this->m_state).
            this-> log_ACTIVITY_LO_LedState(this->m_state);
        }
        this->m_toggleCounter = (this->m_toggleCounter + 1) % interval;
    }
    // We are not blinking
    else {
        if (this->m_state == Fw::On::ON) {
            // Port may not be connected, so check before sending output
            if (this->isConnected_gpioSet_OutputPort(0)) {
                this->gpioSet_out(0, Fw::Logic::LOW);
            }

            this->m_state = Fw::On::OFF;
            // TODO: Emit an event LedState to report the LED state (this->m_state).
            this-> log_ACTIVITY_LO_LedState(this->m_state);
        }
    }
}

    void Led::configure(U32 number){
        printf("sup");
    }


}  // namespace ledmanager
