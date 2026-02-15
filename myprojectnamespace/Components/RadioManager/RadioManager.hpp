// ======================================================================
// \title  RadioManager.hpp
// \author yuktivijay
// \brief  hpp file for RadioManager component implementation class
// ======================================================================

#ifndef Managers_RadioManager_HPP
#define Managers_RadioManager_HPP

#include "myprojectnamespace/Components/RadioManager/RadioManagerComponentAc.hpp"

namespace Managers {

class RadioManager final : public RadioManagerComponentBase {
  public:
    RadioManager(const char* const compName);
    ~RadioManager();

  private:
    U32 m_packetsReceived = 0;
    U32 m_packetsTransmitted = 0;
    I32 m_lastRssi = 0;
    RadioState m_radioState = RadioState::IDLE;

    I8 m_txPower = 14;
    U32 m_frequency = 915;
    U8 m_spreadingFactor = 7;
    U8 m_bandwidth = 7;
    U8 m_codingRate = 1;

    void GET_CONFIG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void SET_TX_POWER_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, I8 power) override;
    void SET_FREQUENCY_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, U32 frequency) override;
    void SET_LORA_PARAMS_cmdHandler(FwOpcodeType opCode, U32 cmdSeq,
                                    U8 spreadingFactor, U8 bandwidth, U8 codingRate) override;
    void TRANSMIT_PACKET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq,
                                     const Fw::CmdStringArg& data) override;
    void GET_TELEMETRY_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void RADIO_RESET_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void run_handler(FwIndexType portNum, U32 context) override;

    void emitTelemetry();
};

}  // namespace Managers

#endif
