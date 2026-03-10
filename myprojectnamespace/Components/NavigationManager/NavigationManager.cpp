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

    bool gotRmc = false, gotGga = false;
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
            gotRmc = true;
        }

        if (strncmp(sentence + 1, "GNGGA", 5) == 0 && numFields > 9) {
            if (fields[2] && fields[3] && fields[4] && fields[5]) {
                lat = str2deg(fields[2], fields[3][0], true);
                lon = str2deg(fields[4], fields[5][0], false);
            }
            numSats = static_cast<U8>(fields[7] ? atoi(fields[7]) : 0);
            alt     = static_cast<F32>(fields[9] ? atof(fields[9]) : 0.0f);
            gotGga  = true;
        }

        i = end + 2;
    }

    return Drv::I2cStatus::I2C_OK;
}

void NavigationManager::configure() {
    U8 message[] = {
        0xB5, 0x62,             // UBX header
        0x06, 0x8A,             // class, ID (UBX-CFG-VALSET)
        0x09, 0x00,             // payload length
        0x00,                   // version
        0x03,                   // RAM + BBR write
        0x00, 0x00,             // reserved
        0x21, 0x00, 0x11, 0x20, // dynamic platform model key
        0x08,                   // airborne <4g
        0x00,                   // checksum A (computed below)
        0x00                    // checksum B (computed below)
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

NavigationManager::NavigationManager(const char* const compName) : NavigationManagerComponentBase(compName) {}

NavigationManager::~NavigationManager() {}

void NavigationManager::GPS_RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->m_hasFix = false;
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void NavigationManager::run_handler(FwIndexType portNum, U32 context) {
    // F64 latitude = 42.2808;
    // F64 longitude = -83.7430;
    // F32 altitude = 270.0f;
    // F32 groundSpeed = 0.0f;
    // U8 numSatellites = 8;

    F64 latitude = 0.0, longitude = 0.0;
    F32 altitude = 0.0f, groundSpeed = 0.0f;
    U8 numSatellites = 0;

    Drv::I2cStatus status = this->readGpsData(latitude, longitude, altitude, groundSpeed, numSatellites);
    if (status != Drv::I2cStatus::I2C_OK) {
        this->log_WARNING_HI_GpsFixLost();
        return;
    }

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
