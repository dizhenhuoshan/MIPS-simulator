//
// Memory class
//

#ifndef MIPS_SIMULATOR_MEMEORY_H
#define MIPS_SIMULATOR_MEMEORY_H

#include "constant.h"

namespace mips
{
    class mips_memeory
    {
    private:
        char *data_memory;
    public:
        mips_memeory()
        {
            data_memory = new char[6 * 1024 * 1024 + 1];
        }
        
        ~mips_memeory()
        {
            delete [] data_memory;
        }
        char *get_bottom()
        {
            return data_memory;
        }
    };
};
#endif //MIPS_SIMULATOR_MEMEORY_H
