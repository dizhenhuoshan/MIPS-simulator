//
// Register class
//

#ifndef MIPS_SIMULATOR_REGISTER_H
#define MIPS_SIMULATOR_REGISTER_H

#include "constant.h"

namespace mips
{
    class mips_register
    {
    private:
        word *register_slot;
    public:
        mips_register()
        {
            register_slot = new word[36];
            register_slot[0].w_data_signed = 0;
        }
        ~mips_register()
        {
            delete [] register_slot;
        }
        void set_stack(char *stack_address)
        {
            register_slot[29].w_data_address = stack_address;
        }
    };
};

#endif //MIPS_SIMULATOR_REGISTER_H
