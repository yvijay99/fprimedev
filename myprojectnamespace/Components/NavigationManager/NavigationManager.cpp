// ======================================================================
// \title  NavigationManager.cpp
// \author yuktivijay
// \brief  cpp file for NavigationManager component implementation class
// ======================================================================

#include "myprojectnamespace/Components/NavigationManager/NavigationManager.hpp"

namespace Managers {

NavigationManager::NavigationManager(const char* const compName) : NavigationManagerComponentBase(compName) {}

NavigationManager::~NavigationManager() {}

void NavigationManager::GPS_RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->m_hasFix = false;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void NavigationManager::run_handler(FwIndexType portNum, U32 context) {
    F64 latitude = 42.2808;
    F64 longitude = -83.7430;
    F32 altitude = 270.0f;
    F32 groundSpeed = 0.0f;
    U8 numSatellites = 8;
    bool currentFix = (numSatellites >= MIN_SATELLITES_FOR_FIX);

    this->tlmWrite_Latitude(latitude);
    this->tlmWrite_Longitude(longitude);
    this->tlmWrite_Altitude(altitude);
    this->tlmWrite_GroundSpeed(groundSpeed);
    this->tlmWrite_NumSatellites(numSatellites);

    if (currentFix && !this->m_hasFix) {
        this->m_hasFix = true;
        this->log_ACTIVITY_HI_GpsFixAcquired(numSatellites);
    } else if (!currentFix && this->m_hasFix) {
        this->m_hasFix = false;
        this->log_WARNING_HI_GpsFixLost();
    }
}

}  // namespace Managers
