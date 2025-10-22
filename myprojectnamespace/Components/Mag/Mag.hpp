// ======================================================================
// \title  Mag.hpp
// \author yuktivijay
// \brief  hpp file for Mag component implementation class
// ======================================================================

#ifndef Mag_Mag_HPP
#define Mag_Mag_HPP

#include "myprojectnamespace/Components/Mag/MagComponentAc.hpp"

namespace Mag {

class Mag final : public MagComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct Mag object
    Mag(const char* const compName  //!< The component name
    );

    //! Destroy Mag object
    ~Mag();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command TODO
    //!
    //! TODO
    void TODO_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                         U32 cmdSeq            //!< The command sequence number
                         ) override;
};

}  // namespace Mag

#endif
