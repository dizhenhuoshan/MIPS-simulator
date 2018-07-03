//
// Codeline is for a single line of a code
//

#ifndef MIPS_SIMULATOR_CODELINE_H
#define MIPS_SIMULATOR_CODELINE_H

#include <string>
#include "constant.h"

namespace mips
{
    class command
    {
        friend class paser;
    private:
        CommandType OPT; //储存操作类型
        char rd; //储存代码中调用的第一个寄存器序号
        char rs; //储存代码中调用的第二个寄存器序号
        char rt; //储存代码中调用的第三个寄存器序号
        int address; //储存指令地址（代码行数）
        int offset; //储存出现的偏移量
        word imm_num; //储存出现的数字
        char *label; //储存出现的静态数据地址
        
        
    public:
        command(){}
//        command(const CommandType &objOPT, const char &objrd = static_cast<const char &>(255),
//                const char &objrs = static_cast<const char &>(255),
//                const char &objrt = static_cast<const char &>(255), const int &objaddress = -1,
//                const int &objoffset = 0, char *&objconstant)
//        {
//            OPT = objOPT;
//            rd = objrd;
//            rs = objrs;
//            rt = objrs;
//            address = objaddress;
//            offset = objoffset;
//            constant = objconstant;
//        }
//
        command(const command &other)
        {
            OPT = other.OPT;
            rd = other.rd;
            rs = other.rs;
            rt = other.rt;
            address = other.address;
            offset = other.offset;
            constant = other.constant;
        }
        
        ~command() = default;
    };
    
}
#endif //MIPS_SIMULATOR_CODELINE_H
