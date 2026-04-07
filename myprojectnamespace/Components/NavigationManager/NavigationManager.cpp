// ======================================================================
// \title  NavigationManager.cpp
// \author yuktivijay
// \brief  cpp file for NavigationManager component implementation class
// ======================================================================

#include "myprojectnamespace/Components/NavigationManager/NavigationManager.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace Managers {

static constexpr U8  GPS_I2C_ADDRESS = 0x42;
static constexpr U8  GPS_DATA_REG    = 0xFF;
static constexpr U32 GPS_CHUNK_SIZE  = 32;
static constexpr U32 GPS_BUFFER_SIZE = 512;

static int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    return 0;
}

static bool validateChecksum(const char* sentence, U32 len) {
    const char* star = nullptr;
    for (U32 i = 0; i < len; i++) {
        if (sentence[i] == '*') { star = &sentence[i]; break; }
    }
    if (star == nullptr || (star - sentence) + 2 >= static_cast<I32>(len)) return false;

    U8 sum = 0;
    for (const char* p = sentence + 1; p < star; p++) {
        sum ^= static_cast<U8>(*p);
    }
    U8 expected = static_cast<U8>((hexCharToInt(star[1]) << 4) + hexCharToInt(star[2]));
    return sum == expected;
}

static F64 str2deg(const char* coord, char direction, bool isLat) {
    if (coord == nullptr || coord[0] == '\0') return 0.0;
    F64 raw = atof(coord);
    F64 deg = static_cast<I32>(raw / 100);
    F64 minutes = raw - deg * 100.0;
    F64 result = deg + minutes / 60.0;
    if (direction == 'S' || direction == 'W') result = -result;
    return result;
}

NavigationManager::NavigationManager(const char* const compName) : NavigationManagerComponentBase(compName) {}

NavigationManager::~NavigationManager() {}

void NavigationManager::run_handler(FwIndexType portNum, U32 context) {
    this->navSm_sendSignal_tick();
}

// ---- State machine actions ----

void NavigationManager::Managers_NavigationManagerStateMachine_action_doInit(
    SmId smId, Managers_NavigationManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::navSm);
    F64 lat = 0, lon = 0;
    F32 alt = 0, speed = 0;
    U8 sats = 0;
    Drv::I2cStatus status = this->readGpsData(lat, lon, alt, speed, sats);
    if (status == Drv::I2cStatus::I2C_OK) {
        this->navSm_sendSignal_success();
    } else {
        this->navSm_sendSignal_fault();
    }
}

void NavigationManager::Managers_NavigationManagerStateMachine_action_doRead(
    SmId smId, Managers_NavigationManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::navSm);
    F64 lat = 0, lon = 0;
    F32 alt = 0, speed = 0;
    U8 sats = 0;
    Drv::I2cStatus status = this->readGpsData(lat, lon, alt, speed, sats);

    if (status != Drv::I2cStatus::I2C_OK) {
        this->log_WARNING_HI_GpsFixLost();
        if (this->isConnected_healthOut_OutputPort(0)) {
            this->healthOut_out(0, false);
        }
        this->navSm_sendSignal_fault();
        return;
    }

    this->reportGpsTelemetry(lat, lon, alt, speed, sats);
    if (this->isConnected_healthOut_OutputPort(0)) {
        this->healthOut_out(0, true);
    }
}

void NavigationManager::Managers_NavigationManagerStateMachine_action_doFaultRecovery(
    SmId smId, Managers_NavigationManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::navSm);
    if (this->isConnected_healthOut_OutputPort(0)) {
        this->healthOut_out(0, false);
    }
    F64 lat = 0, lon = 0;
    F32 alt = 0, speed = 0;
    U8 sats = 0;
    Drv::I2cStatus status = this->readGpsData(lat, lon, alt, speed, sats);
    if (status == Drv::I2cStatus::I2C_OK) {
        this->navSm_sendSignal_success();
    }
}

void NavigationManager::Managers_NavigationManagerStateMachine_action_doSimRead(
    SmId smId, Managers_NavigationManagerStateMachine::Signal signal)
{
    FW_ASSERT(smId == SmId::navSm);
    this->reportGpsTelemetry(42.2808, -83.7430, 270.0f, 0.0f, 8);
    if (this->isConnected_healthOut_OutputPort(0)) {
        this->healthOut_out(0, true);
    }
}

// ---- Command handlers ----

void NavigationManager::GPS_RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->m_hasFix = false;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void NavigationManager::ENABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_SimModeEnabled();
    this->navSm_sendSignal_enableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void NavigationManager::DISABLE_SIM_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_SimModeDisabled();
    this->navSm_sendSignal_disableSim();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

// ---- Helpers ----

void NavigationManager::reportGpsTelemetry(F64 lat, F64 lon, F32 alt, F32 speed, U8 numSats) {
    bool currentFix = (numSats >= MIN_SATELLITES_FOR_FIX);

    this->tlmWrite_Latitude(lat);
    this->tlmWrite_Longitude(lon);
    this->tlmWrite_Altitude(alt);
    this->tlmWrite_GroundSpeed(speed);
    this->tlmWrite_NumSatellites(numSats);

    if (currentFix && !this->m_hasFix) {
        this->m_hasFix = true;
        this->log_ACTIVITY_HI_GpsFixAcquired(numSats);
    } else if (!currentFix && this->m_hasFix) {
        this->m_hasFix = false;
        this->log_WARNING_HI_GpsFixLost();
    }
}

void NavigationManager::configure() {
    U8 message[] = {
        0xB5, 0x62,
        0x06, 0x8A,
        0x09, 0x00,
        0x00,
        0x03,
        0x00, 0x00,
        0x21, 0x00, 0x11, 0x20,
        0x08,
        0x00,
        0x00
    };

    U8 ck_a = 0, ck_b = 0;
    for (U8 i = 2; i < 15; i++) {
        ck_a += message[i];
        ck_b += ck_a;
    }
    message[15] = ck_a;
    message[16] = ck_b;

    Fw::Buffer writeBuffer(message, sizeof(message));
    this->busWrite_out(0, GPS_I2C_ADDRESS, writeBuffer);
}

Drv::I2cStatus NavigationManager::readGpsData(F64& lat, F64& lon, F32& alt, F32& speed, U8& numSats) {
    U8 accumBuf[GPS_BUFFER_SIZE] = {};
    U32 accumLen = 0;
    bool started  = false;
    bool finished = false;

    for (U32 attempts = 0; attempts < 64 && !finished; attempts++) {
        U8 regAddr = GPS_DATA_REG;
        Fw::Buffer writeBuffer(&regAddr, sizeof(regAddr));
        U8 chunk[GPS_CHUNK_SIZE] = {};
        Fw::Buffer readBuffer(chunk, GPS_CHUNK_SIZE);

        Drv::I2cStatus status = this->busWriteRead_out(0, GPS_I2C_ADDRESS, writeBuffer, readBuffer);
        if (status != Drv::I2cStatus::I2C_OK) {
            return status;
        }

        if (!started) {
            for (U32 i = 0; i + 1 < GPS_CHUNK_SIZE; i++) {
                if (chunk[i] == 'M' && chunk[i+1] == 'C') {
                    if (accumLen + 4 + GPS_CHUNK_SIZE < GPS_BUFFER_SIZE) {
                        accumBuf[accumLen++] = '$';
                        accumBuf[accumLen++] = 'G';
                        accumBuf[accumLen++] = 'N';
                        accumBuf[accumLen++] = 'R';
                        memcpy(accumBuf + accumLen, chunk + i, GPS_CHUNK_SIZE - i);
                        accumLen += GPS_CHUNK_SIZE - i;
                    }
                    started = true;
                    break;
                }
            }
        } else {
            if (accumLen + GPS_CHUNK_SIZE < GPS_BUFFER_SIZE) {
                memcpy(accumBuf + accumLen, chunk, GPS_CHUNK_SIZE);
                accumLen += GPS_CHUNK_SIZE;
            }
            for (U32 i = 0; i + 1 < GPS_CHUNK_SIZE; i++) {
                if (chunk[i] == 'S' && chunk[i+1] == 'A') {
                    finished = true;
                    break;
                }
            }
        }
    }

    if (!started || accumLen == 0) {
        return Drv::I2cStatus::I2C_OK;
    }

    U32 i = 0;
    while (i < accumLen) {
        if (accumBuf[i] != '$') { i++; continue; }

        U32 end = i;
        while (end + 1 < accumLen && !(accumBuf[end] == '\r' && accumBuf[end+1] == '\n')) end++;
        if (end + 1 >= accumLen) break;

        char sentence[128] = {};
        U32 sentLen = end - i;
        if (sentLen >= sizeof(sentence)) { i = end + 2; continue; }
        memcpy(sentence, accumBuf + i, sentLen);
        sentence[sentLen] = '\0';

        if (!validateChecksum(sentence, sentLen)) { i = end + 2; continue; }

        char* fields[20] = {};
        U32 numFields = 0;
        char sentCopy[128];
        strncpy(sentCopy, sentence, sizeof(sentCopy) - 1);
        char* tok = strtok(sentCopy, ",");
        while (tok && numFields < 20) {
            fields[numFields++] = tok;
            tok = strtok(nullptr, ",");
        }

        if (strncmp(sentence + 1, "GNRMC", 5) == 0 && numFields > 8) {
            if (fields[2] && fields[2][0] == 'V') { i = end + 2; continue; }
            speed = static_cast<F32>(fields[7] ? atof(fields[7]) : 0.0);
        }

        if (strncmp(sentence + 1, "GNGGA", 5) == 0 && numFields > 9) {
            if (fields[2] && fields[3] && fields[4] && fields[5]) {
                lat = str2deg(fields[2], fields[3][0], true);
                lon = str2deg(fields[4], fields[5][0], false);
            }
            numSats = static_cast<U8>(fields[7] ? atoi(fields[7]) : 0);
            alt     = static_cast<F32>(fields[9] ? atof(fields[9]) : 0.0f);
        }

        i = end + 2;
    }

    return Drv::I2cStatus::I2C_OK;
}

}  // namespace Managers
