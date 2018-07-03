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
        int line_nuber; //传入的当前行号
        std::string current_line; //传入的当前行代码
        int arg_num; //该命令中有多少个参数
        std::string *args; //各个参数
        char* inner_ptr; // 辅助strtok
        std::map<std::string, CommandType> f_command; //操作符映射表
        
        void load_f_command()
        {
            f_command[".align"] = _align;
            f_command[".ascii"] = _ascii;
            f_command[".asciiz"] = _asciiz;
            f_command[".byte"] = _byte;
            f_command[".half"] = _half;
            f_command[".word"] = _word;
            f_command[".space"] = _space;
            f_command[".data"] = _data;
            f_command[".text"] = _text;
            f_command["add"] = add_;
            f_command["addu"] = addu_;
            f_command["addiu"] = addiu_;
            f_command["sub"] = sub_;
            f_command["subu"] = subu_;
            f_command["mul"] = mul_;
            f_command["mulu"] = mulu_;
            f_command["div"] = div_;
            f_command["divu"] = divu_;
            f_command["xor"] = xor_;
            f_command["xoru"] = xoru_;
            f_command["neg"] = neg_;
            f_command["negu"] = negu_;
            f_command["rem"] = rem_;
            f_command["remu"] = remu_;
            f_command["li"] = li_;
            f_command["seq"] = seq_;
            f_command["sge"] = sge_;
            f_command["sgt"] = sgt_;
            f_command["sle"] = sle_;
            f_command["slt"] = slt_;
            f_command["sne"] = sne_;
            f_command["b"] = b_;
            f_command["beq"] = beq_;
            f_command["bne"] = bne_;
            f_command["bge"] = bge_;
            f_command["ble"] = ble_;
            f_command["bgt"] = bgt_;
            f_command["blt"] = blt_;
            f_command["beqz"] = beqz_;
            f_command["bnez"] = bnez_;
            f_command["blez"] = blez_;
            f_command["bgez"] = bgez_;
            f_command["bgtz"] = bgtz_;
            f_command["bltz"] = bltz_;
            f_command["j"] = j_;
            f_command["jr"] = jr_;
            f_command["jal"] = jal_;
            f_command["jalr"] = jalr_;
            f_command["la"] = la_;
            f_command["lb"] = lb_;
            f_command["lh"] = lh_;
            f_command["lw"] = lw_;
            f_command["sb"] = sb_;
            f_command["sh"] = sh_;
            f_command["sw"] = sw_;
            f_command["move"] = move_;
            f_command["mfhi"] = mfhi_;
            f_command["mflo"] = mflo_;
            f_command["nop"] = nop_;
            f_command["syscall"] = syscall_;
        }
        void split_line()
        {
            char *tmpline = new char[current_line.size() + 10];
        }
        CommandType get_command_type()
        {}
    public:
        paser()
        {
            load_f_command();
            line_nuber = -1;
            inner_ptr = NULL;
            args = new std::string[10];
        }
        ~paser() = default;
        void paser_code(const int &obj_linenumber, std::string &obj_current_line, std::vector<command> &text_memory, char *data_memory, std::map<int, std::string> &unknown_table, char &dtflag)
        {
        
        }
    };
};
#endif //MIPS_SIMULATOR_PASER_H
