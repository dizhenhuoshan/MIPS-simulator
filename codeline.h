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
        friend class decoder;
    private:
        CommandType OPT; //储存操作类型
        unsigned char rd; //储存代码中调用的第一个寄存器序号
        unsigned char rs; //储存代码中调用的第二个寄存器序号
        unsigned char rt; //储存代码中调用的第三个寄存器序号
        word address; //储存地址（代码行数/数据地址）
        word offset; //储存出现的偏移量
        word imm_num; //储存出现的数字
        
        
    public:
        command()
        {
            rd = static_cast<unsigned char>(255);
            rs = static_cast<unsigned char>(255);
            rt = static_cast<unsigned char>(255);
            address.w_data_signed = 0;
            offset.w_data_signed = 0;
            imm_num.w_data_signed = 0;
        }
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
            imm_num = other.imm_num;
        }
        
        ~command() = default;
    };
    
}
#endif //MIPS_SIMULATOR_CODELINE_H
