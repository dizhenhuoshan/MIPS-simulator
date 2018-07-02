//
// Paser for the mips-simulator
//
#ifndef MIPS_SIMULATOR_PASER_H
#define MIPS_SIMULATOR_PASER_H

#include "codeline.h"
#include "constant.h"
#include <map>
namespace mips
{
    class paser
    {
    private:
        std::map<std::string, std::string> f_command;
        void load_f_command()
        {
            f_command[".align"] = "align_";
            f_command[".ascii"] = "ascii_";
            f_command[".asciiz"] = "asciiz_";
            f_command[".byte"] = "byte_";
            f_command[".half"] = "half_";
            f_command[".word"] = "word_";
            f_command[".space"] = "space_";
            
        }
    public:
        paser()
        {
            load_f_command();
        }
        ~paser() = default;
        bool paser_code()
        {
        
        }
    };
};
#endif //MIPS_SIMULATOR_PASER_H
