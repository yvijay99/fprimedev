// ======================================================================
// \title  TempManager.hpp
// \author lauraf26846
// \brief  hpp file for TempManager component implementation class
// ======================================================================

#ifndef tempManager_TempManager_HPP
#define tempManager_TempManager_HPP

#include "myprojectnamespace/Components/TempManager/TempManagerComponentAc.hpp"

namespace Components {

class TempManager final : public TempManagerComponentBase {
  public:
    static constexpr U8 DEFAULT_ADDR = 0x48;
    static constexpr U8 DATA_SIZE = 6;

    TempManager(const char* const compName);
    ~TempManager();

  private:
    void run_handler(FwIndexType portNum, U32 context) override;
    void READ_TEMP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    void readAndReportTemp();
};

}  // namespace Components

#endif
