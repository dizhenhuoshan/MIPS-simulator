//
// decoder is to execute the encoded command
//

#ifndef MIPS_SIMULATOR_DECODER_H
#define MIPS_SIMULATOR_DECODER_H

#include "constant.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <map>
namespace mips
{
    class decoder
    {
    private:
        std::string tmpstr; //用于输入的暂存
        word *register_slot; //寄存器
        char *data_memory_bottom; //假内存头地址
        unsigned int current_line; //当前执行到的行号
        command current_command;
        //std::map<unsigned int, std::string> debug_map;//debug

        void clear_commmand(command &obj)
        {
            obj.OPT = label;
            obj.rt = obj.rs = obj.rd = 255;
            obj.address.w_data_address = -1;
            obj.offset.w_data_signed = 0;
            obj.imm_num.w_data_signed = 0;
        }

        void copy_command(command &obj, const command &src)
        {
            obj.OPT = src.OPT;
            obj.rd = src.rd;
            obj.rs = src.rs;
            obj.rt = src.rt;
            obj.address.w_data_address = src.address.w_data_address;
            obj.imm_num.w_data_signed = src.imm_num.w_data_signed;
            obj.offset.w_data_signed = src.offset.w_data_signed;
        }

        void mul_overflow(word &left_src, word &right_src, word &low, word &high)
        {
            long long left = left_src.w_data_signed;
            long long right = right_src.w_data_signed;
            low.w_data_signed = static_cast<int>(left * right & 0xffffffff);
            high.w_data_signed = static_cast<int>((left * right)>>32);
        }

        void mulu_overflow(word &left_src, word &right_src, word &low, word &high)
        {
            unsigned long long left = left_src.w_data_unsigned;
            unsigned long long right = right_src.w_data_unsigned;
            low.w_data_unsigned = static_cast<unsigned int>(left * right & 0xffffffff);
            high.w_data_unsigned = static_cast<unsigned int>((left * right)>>32);
        }

        void div_overflow(word &left_src, word &right_src, word &low, word &high)
        {
            low.w_data_signed = left_src.w_data_signed / right_src.w_data_signed;
            high.w_data_signed = left_src.w_data_signed - low.w_data_signed * right_src.w_data_signed;
        }

        void divu_overflow(word &left_src, word &right_src, word &low, word &high)
        {
            low.w_data_unsigned = left_src.w_data_unsigned / right_src.w_data_unsigned;
            high.w_data_unsigned = left_src.w_data_unsigned - low.w_data_unsigned * right_src.w_data_unsigned;
        }

        void command_syscall(unsigned int &heap_bottom)
        {
            switch (register_slot[2].w_data_unsigned)
            {
                case 1:
                    std::cout << register_slot[4].w_data_signed;
                    break;
                case 4:
                    std::cout << data_memory_bottom + register_slot[4].w_data_address;
                    break;
                case 5:
                    std::cin >> register_slot[2].w_data_signed;
                    break;
                case 8:
                    std::cin >> tmpstr;
                    memcpy(data_memory_bottom + register_slot[4].w_data_address, tmpstr.c_str(), std::min(register_slot[5].w_data_unsigned, static_cast<unsigned int>(tmpstr.size() + 1)));
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
        decoder(unsigned int main_pos, char *memory)
        {
            current_line = main_pos;
            data_memory_bottom = memory;
            // //for debug
            // debug_map[0] = "label";
            // debug_map[1] = "align";
            // debug_map[2] = "ascii";
            // debug_map[3] = "asciiz";
            // debug_map[4] = "byte";
            // debug_map[5] = "half";
            // debug_map[6] = "word";
            // debug_map[7] = "space";
            // debug_map[8] = "data";
            // debug_map[9] = "text";
            // debug_map[10] = "add";
            // debug_map[11] = "addu";
            // debug_map[12] = "addiu";
            // debug_map[13] = "sub";
            // debug_map[14] = "subu";
            // debug_map[15] = "mul";
            // debug_map[16] = "mulu";
            // debug_map[17] = "div";
            // debug_map[18] = "divu";
            // debug_map[19] = "xor";
            // debug_map[20] = "xoru";
            // debug_map[21] = "neg";
            // debug_map[22] = "negu";
            // debug_map[23] = "rem";
            // debug_map[24] = "remu";
            // debug_map[25] = "li";
            // debug_map[26] = "seq";
            // debug_map[27] = "sge";
            // debug_map[28] = "sgt";
            // debug_map[29] = "sle";
            // debug_map[30] = "slt";
            // debug_map[31] = "sne";
            // debug_map[32] = "b";
            // debug_map[33] = "beq";
            // debug_map[34] = "bne";
            // debug_map[35] = "bge";
            // debug_map[36] = "ble";
            // debug_map[37] = "bgt";
            // debug_map[38] = "blt";
            // debug_map[39] = "beqz";
            // debug_map[40] = "bnez";
            // debug_map[41] = "blez";
            // debug_map[42] = "bgez";
            // debug_map[43] = "bgtz";
            // debug_map[44] = "bltz";
            // debug_map[45] = "j";
            // debug_map[46] = "jr";
            // debug_map[47] = "jal";
            // debug_map[48] = "jalr";
            // debug_map[49] = "la";
            // debug_map[50] = "lb";
            // debug_map[51] = "lh";
            // debug_map[52] = "lw";
            // debug_map[53] = "sb";
            // debug_map[54] = "sh";
            // debug_map[55] = "sw";
            // debug_map[56] = "move";
            // debug_map[57] = "mfhi";
            // debug_map[58] = "mflo";
            // debug_map[59] = "nop";
            // debug_map[60] = "syscall";
        }
        ~decoder() = default;

        void decode_command(std::vector<mips::command> &text_memory, unsigned int &data_memory_pos, word *register_add)
        {
            register_slot = register_add;
            while (true)
            {
                clear_commmand(current_command);
                copy_command(current_command, text_memory[current_line - 1]);
                //std::cout << debug_map[current_command.OPT] << "\t  line_cnt:" << current_line << std::endl;
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
                        register_slot[current_command.rd].w_data_signed = -register_slot[current_command.rs].w_data_signed;
                        current_line++;
                        break;
                    case negu_:
                        register_slot[current_command.rd].w_data_unsigned = ~register_slot[current_command.rs].w_data_unsigned;
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
                        register_slot[current_command.rd].w_data_signed = current_command.imm_num.w_data_signed;
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
                        if (current_command.rt == 255) //该操作的第二个参数是立即数
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
                        if (current_command.rt == 255) //address是一个label
                            register_slot[current_command.rd].w_data_address = current_command.address.w_data_address;
                        else //address存在rt寄存器里并且有偏移量
                        {
                            unsigned int src = register_slot[current_command.rt].w_data_address + current_command.offset.w_data_signed;
                            register_slot[current_command.rd].w_data_address = src;
                        }
                        current_line++;
                        break;
                    case lb_:
                        if (current_command.rt == 255) //address是一个label
                        {
                            memset(register_slot[current_command.rd].b, 0, sizeof(word));
                            memcpy(register_slot[current_command.rd].b, current_command.address.w_data_address + data_memory_bottom, sizeof(byte));
                        }
                        else //address存在rt寄存器里并且有偏移量
                        {
                            unsigned int src = register_slot[current_command.rt].w_data_address + current_command.offset.w_data_signed;
                            memset(register_slot[current_command.rd].b, 0, sizeof(word));
                            memcpy(register_slot[current_command.rd].b, data_memory_bottom + src, sizeof(byte));
                        }
                        current_line++;
                        break;
                    case lh_:
                        if (current_command.rt == 255) //address是一个label
                        {
                            memset(register_slot[current_command.rd].b, 0, sizeof(word));
                            memcpy(register_slot[current_command.rd].b, data_memory_bottom + current_command.address.w_data_address, sizeof(half));
                        }
                        else //address存在rt寄存器里并且有偏移量
                        {
                            unsigned int src = register_slot[current_command.rt].w_data_address + current_command.offset.w_data_signed;
                            memset(register_slot[current_command.rd].b, 0, sizeof(word));
                            memcpy(register_slot[current_command.rd].b, data_memory_bottom + src, sizeof(half));
                        }
                        current_line++;
                        break;
                    case lw_:
                        if (current_command.rt == 255) //address是一个label
                            memcpy(register_slot[current_command.rd].b, data_memory_bottom + current_command.address.w_data_address, sizeof(word));
                        else //address存在rt寄存器里并且有偏移量
                        {
                            unsigned int src = register_slot[current_command.rt].w_data_address + current_command.offset.w_data_signed;
                            memset(register_slot[current_command.rd].b, 0, sizeof(word));
                            memcpy(register_slot[current_command.rd].b, data_memory_bottom + src, sizeof(word));
                        }
                        current_line++;
                        break;
                    case sb_:
                        if (current_command.rt == 255) //address是一个label
                            memcpy(data_memory_bottom + current_command.address.w_data_address, register_slot[current_command.rs].b, sizeof(byte));
                        else //address存在rt寄存器里并且有偏移量
                        {
                            unsigned int des = register_slot[current_command.rt].w_data_address + current_command.offset.w_data_signed;
                            memcpy(data_memory_bottom + des, register_slot[current_command.rs].b, sizeof(byte));
                        }
                        current_line++;
                        break;
                    case sh_:
                        if (current_command.rt == 255) //address是一个label
                            memcpy(data_memory_bottom + current_command.address.w_data_address, register_slot[current_command.rs].b, sizeof(half));
                        else //address存在rt寄存器里并且有偏移量
                        {
                            unsigned int des = register_slot[current_command.rt].w_data_address + current_command.offset.w_data_signed;
                            memcpy(data_memory_bottom + des, register_slot[current_command.rs].b, sizeof(half));
                        }
                        current_line++;
                        break;
                    case sw_:
                        if (current_command.rt == 255) //address是一个label
                            memcpy(data_memory_bottom + current_command.address.w_data_address, register_slot[current_command.rs].b, sizeof(word));
                        else //address存在rt寄存器里并且有偏移量
                        {
                            unsigned int des = register_slot[current_command.rt].w_data_address + current_command.offset.w_data_signed;
                            memcpy(data_memory_bottom + des, register_slot[current_command.rs].b, sizeof(word));
                        }
                        current_line++;
                        break;
                    case move_:
                        register_slot[current_command.rd] = register_slot[current_command.rs];
                        current_line++;
                        break;
                    case mfhi_:
                        register_slot[current_command.rd] = register_slot[32];
                        current_line++;
                        break;
                    case mflo_:
                        register_slot[current_command.rd] = register_slot[33];
                        current_line++;
                        break;
                    case nop_:
                        current_line++;
                        break;
                    case syscall_:
                        command_syscall(data_memory_pos);
                        current_line++;
                        break;
                    case label:
                    case _align:
                    case _ascii:
                    case _asciiz:
                    case _byte:
                    case _half:
                    case _word:
                    case _space:
                    case _data:
                    case _text:
                        std::cerr << "error" << current_command.OPT << std::endl;
                        std::cerr << current_line << std::endl;
                        break;
                }
            }
        }
    };
};


#endif //MIPS_SIMULATOR_DECODER_H
