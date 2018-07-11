//
// Some const or enum
//

#ifndef MIPS_SIMULATOR_CONSTANT_H
#define MIPS_SIMULATOR_CONSTANT_H

#include <iostream>
#include <string>
#include <cstring>

namespace mips
{
    //定义三个类型
    struct byte
    {
        char b_data;
    };
    
    union half
    {
        short h_data_signed;
        unsigned short h_data_unsigned;
        char b[2];
    };
    
    union word
    {
        int w_data_signed;
        unsigned int w_data_unsigned;
        unsigned int w_data_address;
        half h[2];
        char b[4];
    };
    
    //枚举所有的操作符
    enum CommandType
    {
        label,
        _align, _ascii, _asciiz, _byte, _half, _word, _space, _data, _text,
        add_, addu_, addiu_, sub_, subu_, mul_, mulu_, div_, divu_, xor_, xoru_, neg_, negu_, rem_, remu_,
        li_, seq_, sge_, sgt_, sle_, slt_, sne_,
        b_, beq_, bne_, bge_, ble_, bgt_, blt_, beqz_, bnez_, blez_, bgez_, bgtz_, bltz_, j_, jr_, jal_, jalr_,
        la_, lb_, lh_, lw_,
        sb_, sh_, sw_,
        move_, mfhi_, mflo_,
        nop_, syscall_
    };
    
    //储存一个函数的名字，开始行以及结束行
    struct label_info
    {
        unsigned int start_line;
        label_info()
        {
            start_line = 0;
        }
    };
    
    class command
    {
        friend class paser;
        friend class decoder;
    private:
        unsigned char OPT; //储存操作类型
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
            OPT = 0;
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
    
    struct register_ifid
    {
        command current_command;
    };
    
    struct register_idex
    {
        unsigned char OPT;
        unsigned char rd_num;
        word rs;
        word rt_imm_num;
        word address;
        word offset;
    };
    
    struct register_exmem
    {
        unsigned char OPT;
        unsigned char rd;
        word reg_data;
        word reg_data2; //用于lo hi寄存器写入
        word address;
    };
    
    struct register_memwb
    {
        unsigned char OPT;
        unsigned char rd;
        word reg_data;
        word reg_data2; //用于lo hi寄存器同时写入
    };
    
}
#endif //MIPS_SIMULATOR_CONSTANT_H
