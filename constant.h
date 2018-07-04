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
        unsigned w_data_unsigned;
        char *w_data_address;
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
        std::string name;
        unsigned int start_line;
        unsigned int end_line;
        label_info()
        {
            name = "";
            start_line = 0;
            end_line = 0;
        }
    };
}
#endif //MIPS_SIMULATOR_CONSTANT_H
