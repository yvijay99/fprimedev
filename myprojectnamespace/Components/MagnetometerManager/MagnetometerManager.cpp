// ======================================================================
// \title  MagnetometerManager.cpp
// \author yuktivijay
// \brief  cpp file for MagnetometerManager component implementation class
// ======================================================================

#include "myprojectnamespace/Components/MagnetometerManager/MagnetometerManager.hpp"

namespace Managers {

MagnetometerManager::MagnetometerManager(const char* const compName) : MagnetometerManagerComponentBase(compName) {}

MagnetometerManager::~MagnetometerManager() {}

void MagnetometerManager::configure(U32 i2cAddress) {
    this->m_i2cAddress = i2cAddress;
}

Drv::I2cStatus MagnetometerManager::readMagData(F32& mx, F32& my, F32& mz) {
    U8 regAddr = MX_REG;
    Fw::Buffer writeBuffer(&regAddr, sizeof(regAddr));

    U8 rawData[MAG_DATA_SIZE] = {};
    Fw::Buffer readBuffer(rawData, sizeof(rawData));

    Drv::I2cStatus status = this->busWriteRead_out(0, this->m_i2cAddress, writeBuffer, readBuffer);

    if (status == Drv::I2cStatus::I2C_OK) {
        I32 rawMx = (static_cast<I32>(rawData[0]) << 16) |
                    (static_cast<I32>(rawData[1]) << 8) |
                     static_cast<I32>(rawData[2]);
        I32 rawMy = (static_cast<I32>(rawData[3]) << 16) |
                    (static_cast<I32>(rawData[4]) << 8) |
                     static_cast<I32>(rawData[5]);
        I32 rawMz = (static_cast<I32>(rawData[6]) << 16) |
                    (static_cast<I32>(rawData[7]) << 8) |
                     static_cast<I32>(rawData[8]);

        if (rawMx & 0x800000) { rawMx |= 0xFF000000; }
        if (rawMy & 0x800000) { rawMy |= 0xFF000000; }
        if (rawMz & 0x800000) { rawMz |= 0xFF000000; }

        mx = static_cast<F32>(rawMx) / RM3100_SENSITIVITY;
        my = static_cast<F32>(rawMy) / RM3100_SENSITIVITY;
        mz = static_cast<F32>(rawMz) / RM3100_SENSITIVITY;
    }

    return status;
}

Drv::I2cStatus MagnetometerManager::readTemperature(F32& tempC) {
    U8 regAddr = STATUS_REG;
    Fw::Buffer writeBuffer(&regAddr, sizeof(regAddr));

    U8 rawData[1] = {};
    Fw::Buffer readBuffer(rawData, sizeof(rawData));

    Drv::I2cStatus status = this->busWriteRead_out(0, this->m_i2cAddress, writeBuffer, readBuffer);

    if (status == Drv::I2cStatus::I2C_OK) {
        tempC = 0.0f;
    }

    return status;
}

void MagnetometerManager::CALIBRATE_MAG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    this->log_ACTIVITY_HI_MagCalibrationStarted();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void MagnetometerManager::run_handler(FwIndexType portNum, U32 context) {
    if (this->isConnected_busWriteRead_OutputPort(0)) {
        F32 mx = 0, my = 0, mz = 0;

        Drv::I2cStatus status = this->readMagData(mx, my, mz);

        if (status == Drv::I2cStatus::I2C_OK) {
            this->tlmWrite_MagX(mx);
            this->tlmWrite_MagY(my);
            this->tlmWrite_MagZ(mz);

            F32 tempC = 0.0f;
            if (this->readTemperature(tempC) == Drv::I2cStatus::I2C_OK) {
                this->tlmWrite_MagTemp(tempC);
            }

            this->log_ACTIVITY_LO_MagReadOk();
        } else {
            this->log_WARNING_HI_MagReadError(static_cast<I32>(status.e));
        }
    }
}

}  // namespace Managers
