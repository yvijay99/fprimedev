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
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    static constexpr U8 DEFAULT_ADDR = 0x48; // default TMP102 I2C address
    static constexpr U8 DATA_SIZE = 6; // 2 bytes for temperature + 4 bytes for timestamp

    //! Construct TempManager object
    TempManager(const char* const compName  //!< The component name
    );

    //! Destroy TempManager object
    ~TempManager();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for run
    //!
    //! Port receiving calls from the rate group
    void run_handler(FwIndexType portNum,  //!< The port number
                     U32 context           //!< The call order
                     ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command READ_TEMP
    //!
    //! Example async command
    void READ_TEMP_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                              U32 cmdSeq            //!< The command sequence number
                              ) override;
};

}  // namespace tempManager

#endif
