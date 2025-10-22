// ======================================================================
// \title  Gyro.hpp
// \author yuktivijay
// \brief  hpp file for Gyro component implementation class
// ======================================================================

#ifndef Gyro_Gyro_HPP
#define Gyro_Gyro_HPP

#include "myprojectnamespace/Components/Gyro/GyroComponentAc.hpp"

namespace Gyro {

class Gyro final : public GyroComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct Gyro object
    Gyro(const char* const compName  //!< The component name
    );

    //! Destroy Gyro object
    ~Gyro();

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

}  // namespace Gyro

#endif
