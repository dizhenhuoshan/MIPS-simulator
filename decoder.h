//
// decoder is to execute the encoded command
//

#ifndef MIPS_SIMULATOR_DECODER_H
#define MIPS_SIMULATOR_DECODER_H

#include "constant.h"
#include "codeline.h"
#include <iostream>
#include <vector>
#include <map>
namespace mips
{
    class decoder
    {
    private:
        std::string tmpstr; //用于输入的暂存
        word *register_slot; //寄存器
        unsigned int current_line; //当前执行到的行号
        command current_command;
        
        void mul_overflow(word &left_src, word &right_src, word &low, word &high)
        {
            long long left = left_src.w_data_signed;
            long long right = right_src.w_data_signed;
            low.w_data_signed = static_cast<int>((left * right)&0xffffffff);
            high.w_data_signed = static_cast<int>((left * right)>>32);
        }
    
        void mulu_overflow(word &left_src, word &right_src, word &low, word &high)
        {
            unsigned long long left = left_src.w_data_unsigned;
            unsigned long long right = right_src.w_data_unsigned;
            low.w_data_unsigned = static_cast<unsigned int>((left * right)&0xffffffff);
            high.w_data_unsigned = static_cast<unsigned int>((left * right)>>32);
        }
    
        void div_overflow(word &left_src, word &right_src, word &low, word &high)
        {
            low.w_data_signed = left_src.w_data_signed / right_src.w_data_signed;
            high.w_data_signed = left_src.w_data_signed % right_src.w_data_signed;
        }
        
        void divu_overflow(word &left_src, word &right_src, word &low, word &high)
        {
            low.w_data_unsigned = left_src.w_data_unsigned / right_src.w_data_unsigned;
            high.w_data_unsigned = left_src.w_data_unsigned % right_src.w_data_unsigned;
        }
        
        void command_syscall(char *&heap_bottom)
        {
            switch (register_slot[2].w_data_unsigned)
            {
                case 1:
                    std::cout << register_slot[4].w_data_signed;
                    break;
                case 4:
                    std::cout << register_slot[4].w_data_address;
                    break;
                case 5:
                    std::cin >> register_slot[2].w_data_signed;
                    break;
                case 8:
                    std::cin >> tmpstr;
                    memcpy(register_slot[4].w_data_address, tmpstr.c_str(), register_slot[5].w_data_unsigned);
                    break;
                case 9:
                    register_slot[2].w_data_address = heap_bottom;
                    heap_bottom += register_slot[4].w_data_unsigned;
                    break;
                case 10:
                    exit(0);
                case 17:
                    exit(register_slot[4].w_data_signed);
            }
        }
        
    public:
        decoder(unsigned int main_pos)
        {
            current_line = main_pos;
        }
        ~decoder() = default;
        
        void decode_command(std::vector<mips::command> &text_memory, char *data_memory_bottom, char *&data_memory_pos, word *register_add)
        {
            register_slot = register_add;
            while (true)
            {
                current_command = text_memory[current_line - 1];
                switch (current_command.OPT)
                {
                    case add_:
                        if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                            register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed + current_command.imm_num.w_data_signed;
                        else //第三个数也是寄存器
                            register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed + register_slot[current_command.rt].w_data_signed;
                        current_line++;
                        break;
                    case addu_:
                        if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned + current_command.imm_num.w_data_unsigned;
                        else //第三个数也是寄存器
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned + register_slot[current_command.rt].w_data_unsigned;
                        current_line++;
                        break;
                    case addiu_:
                        if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned + current_command.imm_num.w_data_unsigned;
                        else //第三个数也是寄存器
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned + register_slot[current_command.rt].w_data_unsigned;
                        current_line++;
                        break;
                    case sub_:
                        if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                            register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed - current_command.imm_num.w_data_signed;
                        else //第三个数也是寄存器
                            register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed - register_slot[current_command.rt].w_data_signed;
                        current_line++;
                        break;
                    case subu_:
                        if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned - current_command.imm_num.w_data_unsigned;
                        else //第三个数也是寄存器
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned - register_slot[current_command.rt].w_data_unsigned;
                        current_line++;
                        break;
                    case mul_:
                        if (current_command.rd == 255) //没有rd寄存器，该操作是二参数的
                        {
                            if (current_command.rt == 255) //该操作的第二个参数是一个立即数
                                mul_overflow(register_slot[current_command.rs], current_command.imm_num, register_slot[33], register_slot[32]);
                            else //第三个数也是寄存器
                                mul_overflow(register_slot[current_command.rs], register_slot[current_command.rt], register_slot[33], register_slot[32]);
                        }
                        else //三参数
                        {
                            if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                                register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed * current_command.imm_num.w_data_signed;
                            else //第三个数也是寄存器
                                register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed * register_slot[current_command.rt].w_data_signed;
                        }
                        current_line++;
                        break;
                    case mulu_:
                        if (current_command.rd == 255) //没有rd寄存器，该操作是二参数的
                        {
                            if (current_command.rt == 255) //该操作的第二个参数是一个立即数
                                mulu_overflow(register_slot[current_command.rs], current_command.imm_num, register_slot[33], register_slot[32]);
                            else //第三个数也是寄存器
                                mulu_overflow(register_slot[current_command.rs], register_slot[current_command.rt], register_slot[33], register_slot[32]);
                        }
                        else //三参数
                        {
                            if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                                register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned * current_command.imm_num.w_data_unsigned;
                            else //第三个数也是寄存器
                                register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned * register_slot[current_command.rt].w_data_unsigned;
                        }
                        current_line++;
                        break;
                    case div_:
                        if (current_command.rd == 255) //没有rd寄存器，该操作是二参数的
                        {
                            if (current_command.rt == 255) //该操作的第二个参数是一个立即数
                                div_overflow(register_slot[current_command.rs], current_command.imm_num, register_slot[33], register_slot[32]);
                            else //第三个数也是寄存器
                                div_overflow(register_slot[current_command.rs], register_slot[current_command.rt], register_slot[33], register_slot[32]);
                        }
                        else //三参数
                        {
                            if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                                register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed / current_command.imm_num.w_data_signed;
                            else //第三个数也是寄存器
                                register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed / register_slot[current_command.rt].w_data_signed;
                        }
                        current_line++;
                        break;
                    case divu_:
                        if (current_command.rd == 255) //没有rd寄存器，该操作是二参数的
                        {
                            if (current_command.rt == 255) //该操作的第二个参数是一个立即数
                                divu_overflow(register_slot[current_command.rs], current_command.imm_num, register_slot[33], register_slot[32]);
                            else //第三个数也是寄存器
                                divu_overflow(register_slot[current_command.rs], register_slot[current_command.rt], register_slot[33], register_slot[32]);
                        }
                        else //三参数
                        {
                            if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                                register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned / current_command.imm_num.w_data_unsigned;
                            else //第三个数也是寄存器
                                register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned / register_slot[current_command.rt].w_data_unsigned;
                        }
                        current_line++;
                        break;
                    case xor_:
                        if (current_command.rt == 255) //第三个参数是立即数
                            register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed ^ current_command.imm_num.w_data_signed;
                        else
                            register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed ^ register_slot[current_command.rt].w_data_signed;
                        current_line++;
                        break;
                    case xoru_:
                        if (current_command.rt == 255) //第三个参数是立即数
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned ^ current_command.imm_num.w_data_unsigned;
                        else
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned ^ register_slot[current_command.rt].w_data_unsigned;
                        current_line++;
                        break;
                    case neg_:
                        register_slot[current_command.rs].w_data_signed = -register_slot[current_command.rt].w_data_signed;
                        current_line++;
                        break;
                    case negu_:
                        register_slot[current_command.rs].w_data_unsigned = ~register_slot[current_command.rt].w_data_unsigned;
                        current_line++;
                        break;
                    case rem_:
                        if (current_command.rt == 255) //第三个参数是立即数
                            register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed % current_command.imm_num.w_data_signed;
                        else
                            register_slot[current_command.rd].w_data_signed = register_slot[current_command.rs].w_data_signed % register_slot[current_command.rt].w_data_signed;
                        current_line++;
                        break;
                    case remu_:
                        if (current_command.rt == 255) //该操作的第三个参数是一个立即数
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned % current_command.imm_num.w_data_unsigned;
                        else //第三个数也是寄存器
                            register_slot[current_command.rd].w_data_unsigned = register_slot[current_command.rs].w_data_unsigned % register_slot[current_command.rt].w_data_unsigned;
                        current_line++;
                        break;
                    case li_:
                        register_slot[current_command.rs] = current_command.imm_num;
                        current_line++;
                        break;
                    case seq_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed == current_command.imm_num.w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed == register_slot[current_command.rt].w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        current_line++;
                        break;
                    case sge_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed >= current_command.imm_num.w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed >= register_slot[current_command.rt].w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        current_line++;
                        break;
                    case sgt_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed > current_command.imm_num.w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed > register_slot[current_command.rt].w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        current_line++;
                        break;
                    case sle_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed <= current_command.imm_num.w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed <= register_slot[current_command.rt].w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        current_line++;
                        break;
                    case slt_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed < current_command.imm_num.w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed < register_slot[current_command.rt].w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        current_line++;
                        break;
                    case sne_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed != current_command.imm_num.w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed != register_slot[current_command.rt].w_data_signed)
                                register_slot[current_command.rd].w_data_signed = 1;
                            else register_slot[current_command.rd].w_data_signed = 0;
                        }
                        current_line++;
                        break;
                    case b_:
                        current_line = current_command.address.w_data_unsigned;
                        break;
                    case beq_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed == current_command.imm_num.w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed == register_slot[current_command.rt].w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        break;
                    case bne_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed != current_command.imm_num.w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed != register_slot[current_command.rt].w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        break;
                    case bge_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed >= current_command.imm_num.w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed >= register_slot[current_command.rt].w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        break;
                    case ble_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed <= current_command.imm_num.w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed <= register_slot[current_command.rt].w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        break;
                    case bgt_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed > current_command.imm_num.w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed > register_slot[current_command.rt].w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        break;
                    case blt_:
                        if (current_command.rt == 255) //该操作的第三个参数是立即数
                        {
                            if (register_slot[current_command.rs].w_data_signed < current_command.imm_num.w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        else
                        {
                            if (register_slot[current_command.rs].w_data_signed < register_slot[current_command.rt].w_data_signed)
                                current_line = current_command.address.w_data_unsigned;
                            else current_line++;
                        }
                        break;
                    case beqz_:
                        if (register_slot[current_command.rs].w_data_signed == 0)
                            current_line = current_command.address.w_data_unsigned;
                        else current_line++;
                        break;
                    case bnez_:
                        if (register_slot[current_command.rs].w_data_signed != 0)
                            current_line = current_command.address.w_data_unsigned;
                        else current_line++;
                        break;
                    case blez_:
                        if (register_slot[current_command.rs].w_data_signed <= 0)
                            current_line = current_command.address.w_data_unsigned;
                        else current_line++;
                        break;
                    case bgez_:
                        if (register_slot[current_command.rs].w_data_signed >= 0)
                            current_line = current_command.address.w_data_unsigned;
                        else current_line++;
                        break;
                    case bgtz_:
                        if (register_slot[current_command.rs].w_data_signed > 0)
                            current_line = current_command.address.w_data_unsigned;
                        else current_line++;
                        break;
                    case bltz_:
                        if (register_slot[current_command.rs].w_data_signed < 0)
                            current_line = current_command.address.w_data_unsigned;
                        else current_line++;
                        break;
                    case j_:
                        current_line = current_command.address.w_data_unsigned;
                        break;
                    case jr_:
                        current_line = register_slot[current_command.rs].w_data_unsigned;
                        break;
                    case jal_:
                        register_slot[31].w_data_unsigned = current_line + 1;
                        current_line = current_command.address.w_data_unsigned;
                        break;
                    case jalr_:
                        register_slot[31].w_data_unsigned = current_line + 1;
                        current_line = register_slot[current_command.rs].w_data_unsigned;
                        break;
                    case la_:
                        register_slot[current_command.rs].w_data_address = current_command.address.w_data_address;
                        current_line++;
                        break;
                    case lb_:
                        memcpy(&(register_slot[current_command.rs].b[0]), current_command.address.w_data_address + current_command.offset.w_data_signed, sizeof(byte));
                        current_line++;
                        break;
                    case lh_:
                        memcpy(&(register_slot[current_command.rs].b[0]), current_command.address.w_data_address + current_command.offset.w_data_signed, sizeof(half));
                        current_line++;
                        break;
                    case lw_:
                        memcpy(&(register_slot[current_command.rs].b[0]), current_command.address.w_data_address + current_command.offset.w_data_signed, sizeof(word));
                        current_line++;
                        break;
                    case sb_:
                        memcpy(current_command.address.w_data_address + current_command.offset.w_data_signed, &(register_slot[current_command.rs].b[0]), sizeof(byte));
                        current_line++;
                        break;
                    case sh_:
                        memcpy(current_command.address.w_data_address + current_command.offset.w_data_signed, &(register_slot[current_command.rs].b[0]), sizeof(half));
                        current_line++;
                        break;
                    case sw_:
                        memcpy(current_command.address.w_data_address + current_command.offset.w_data_signed, &(register_slot[current_command.rs].b[0]), sizeof(word));
                        current_line++;
                        break;
                    case move_:
                        register_slot[current_command.rs].w_data_signed = register_slot[current_command.rt].w_data_signed;
                        current_line++;
                        break;
                    case mfhi_:
                        register_slot[current_command.rs].w_data_signed = register_slot[32].w_data_signed;
                        current_line++;
                        break;
                    case mflo_:
                        register_slot[current_command.rs].w_data_signed = register_slot[33].w_data_signed;
                        current_line++;
                        break;
                    case nop_:
                        current_line++;
                        break;
                    case syscall_:
                        command_syscall(data_memory_pos);
                        current_line++;
                        break;
                }
            }
        }
    };
};


#endif //MIPS_SIMULATOR_DECODER_H
