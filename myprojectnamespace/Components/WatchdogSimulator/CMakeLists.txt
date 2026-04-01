register_fprime_library(
    AUTOCODER_INPUTS
        "${CMAKE_CURRENT_LIST_DIR}/WatchdogStateMachine.fpp"
        "${CMAKE_CURRENT_LIST_DIR}/WatchdogSimulator.fpp"
    SOURCES
        "${CMAKE_CURRENT_LIST_DIR}/WatchdogSimulator.cpp"
)
