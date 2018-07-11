//
// For pipeline of mips_simulator.
//

#ifndef MIPS_SIMULATOR_PIPELINE_H
#define MIPS_SIMULATOR_PIPELINE_H

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
        word *register_slot; //寄存器地址
        register_ifid ifid_register; // IF/ID寄存器
        register_idex idex_register; // ID/EX寄存器 注意：如果是对low，high寄存器同时操作，rd_num为 32 + 33 = 65
        register_exmem exmem_register; // EX/MEM寄存器
        register_memwb memwb_register; // MEM/WB寄存器
        char register_lock[36]; //寄存器锁
        bool data_lock; //数据Hazard标记
        bool mem_lock; //总线内存锁
        bool control_lock; //分支语句锁
        bool exit_flag; //程序终止标记
        std::string tmpstr; //syscall里面输入用的临时字符串
        std::map<unsigned int, std::string> debug_map;//debug
        unsigned int cycle_cnt; //debug
        
        void mul_overflow(word &left_src, word &right_src, word &high, word &low)
        {
            long long left = left_src.w_data_signed;
            long long right = right_src.w_data_signed;
            low.w_data_signed = static_cast<int>(left * right & 0xffffffff);
            high.w_data_signed = static_cast<int>((left * right)>>32);
        }
    
        void mulu_overflow(word &left_src, word &right_src, word &high, word &low)
        {
            unsigned long long left = left_src.w_data_unsigned;
            unsigned long long right = right_src.w_data_unsigned;
            low.w_data_unsigned = static_cast<unsigned int>(left * right & 0xffffffff);
            high.w_data_unsigned = static_cast<unsigned int>((left * right)>>32);
        }
    
        void div_overflow(word &left_src, word &right_src, word &high, word &low)
        {
            low.w_data_signed = left_src.w_data_signed / right_src.w_data_signed;
            high.w_data_signed = left_src.w_data_signed - low.w_data_signed * right_src.w_data_signed;
        }
    
        void divu_overflow(word &left_src, word &right_src, word &high, word &low)
        {
            low.w_data_unsigned = left_src.w_data_unsigned / right_src.w_data_unsigned;
            high.w_data_unsigned = left_src.w_data_unsigned - low.w_data_unsigned * right_src.w_data_unsigned;
        }
        void syscall_ID()
        {
            if (register_lock[2])
            {
                data_lock = true;
                memset(&idex_register, 0, sizeof(idex_register));
                return;
            }
            idex_register.OPT = syscall_;
            switch (register_slot[2].w_data_unsigned)
            {
                case 1:
                    if (register_lock[4])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register));
                        return; //判出有锁，直接return
                    }
                    idex_register.offset.w_data_unsigned = 1;
                    idex_register.rt_imm_num = register_slot[4];
                    break;
                case 4:
                    if (register_lock[4])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register));
                        return;
                    }
                    idex_register.offset.w_data_unsigned = 4;
                    idex_register.address = register_slot[4];
                    break;
                case 5:
                    register_lock[2]++; //v0寄存器上锁
                    idex_register.offset.w_data_unsigned = 5;
                    break;
                case 8:
                    if (register_lock[4] || register_lock[5]) //检查a0、a1有没有被锁
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register));
                        return;
                    }
                    idex_register.offset.w_data_unsigned = 8;
                    idex_register.address = register_slot[4];
                    idex_register.rt_imm_num = register_slot[5];
                    break;
                case 9:
                    if (register_lock[4])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register));
                        return;
                    }
                    register_lock[2]++;
                    idex_register.offset.w_data_unsigned = 9;
                    idex_register.rt_imm_num = register_slot[4];
                    break;
                case 10:
                    idex_register.offset.w_data_unsigned = 10;
                    exit_flag = true;
                    break;
                case 17:
                    if (register_lock[4])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register));
                        return;
                    }
                    idex_register.offset.w_data_unsigned = 17;
                    idex_register.rt_imm_num = register_slot[4];
                    exit_flag = true;
                    break;
                default:
                    std::cerr << "Error occured in syscall_ID" << std::endl;
            }
            ifid_register = register_ifid(); //复位ifid寄存器
        }
        
        void syscall_EX()
        {
            if (idex_register.offset.w_data_unsigned == 0)
                    return;
            exmem_register.OPT = syscall_;
            switch (idex_register.offset.w_data_unsigned)
            {
                case 1:
                    std::cout << idex_register.rt_imm_num.w_data_signed;
                    break;
                case 4:
                    exmem_register.reg_data.w_data_unsigned = 4;
                    exmem_register.address = idex_register.address;
                    break;
                case 5:
                    exmem_register.reg_data.w_data_unsigned = 5;
                    break;
                case 8:
                    exmem_register.reg_data.w_data_unsigned = 8;
                    exmem_register.address = idex_register.address;
                    exmem_register.reg_data2 = idex_register.rt_imm_num;
                    break;
                case 9:
                    exmem_register.reg_data.w_data_unsigned = 9;
                    exmem_register.reg_data2 = idex_register.rt_imm_num;
                    break;
                case 10:
                    exmem_register.reg_data.w_data_unsigned = 10;
                case 17:
                    exmem_register.reg_data.w_data_unsigned = 17;
                    exmem_register.reg_data2 = idex_register.rt_imm_num;
                    break;
                default:
                    std::cerr << "Error occured in syscall_EX" << std::endl;
                    break;
            }
        }
    
        void syscall_MA(char *data_memory_bottom, unsigned int &data_memory_pos)
        {
            if (exmem_register.reg_data.w_data_unsigned == 0)
                    return;
            memwb_register.OPT = syscall_;
            switch (exmem_register.reg_data.w_data_unsigned)
            {
                case 1:
                    break;
                case 4:
                    std::cout << exmem_register.address.w_data_unsigned + data_memory_bottom;
                    break;
                case 5:
                    memwb_register.rd = 2;
                    std::cin >> memwb_register.reg_data.w_data_signed;
                    break;
                case 8:
                    std::cin >> tmpstr;
                    memcpy(data_memory_bottom + exmem_register.address.w_data_address, tmpstr.c_str(), std::min(exmem_register.reg_data2.w_data_unsigned, static_cast<unsigned int>(tmpstr.size() + 1)));
                    tmpstr = "";
                    break;
                case 9:
                    memwb_register.rd = 2;
                    while (data_memory_pos % 4 != 0)
                        data_memory_pos++;
                    memwb_register.reg_data.w_data_unsigned = data_memory_pos;
                    data_memory_pos += exmem_register.reg_data2.w_data_unsigned;
                    break;
                case 10:
                case 17:
                    memwb_register.rd = 1;
                    memwb_register.reg_data = exmem_register.reg_data2;
                    break;
                default:
                    std::cerr << "Error occured in syscall_MA" << std::endl;
                    break;
            }
        }
        
        void syscall_WB()
        {
            if (memwb_register.rd == 1 && exit_flag)
            {
                std::cerr << "Program finished with return value" << memwb_register.reg_data.w_data_signed << std::endl;
                exit(0);
            }
            if (memwb_register.rd == 2)
            {
                register_slot[2] = memwb_register.reg_data;
                register_lock[2]--;
            }
            if (memwb_register.rd != 0 && memwb_register.rd != 2)
                std::cerr << "Error occured in syscall_WB" << std::endl;
        }
        
        void Instruction_Fetch(std::vector<command> &text_memory)
        {
            if (data_lock)
            {
//                mem_lock = false; //data锁生效时，不会读取数据，故内存锁可以关闭
//                data_lock = false; //data锁用过就解开
                return;
            }
            if (mem_lock || control_lock || exit_flag) //内存锁和控制线锁
            {
                ifid_register = register_ifid();
                return;
            }
            ifid_register.current_command = text_memory[register_slot[34].w_data_unsigned - 1];
            register_slot[34].w_data_unsigned++;
        }
        
        void Instruction_Decode_Data_Preparation()
        {
            if (ifid_register.current_command.OPT == label) //本行被stall了，不执行命令
                return;
            idex_register.OPT = ifid_register.current_command.OPT;
            switch (static_cast<CommandType>(ifid_register.current_command.OPT))
            {
                case add_:
                case addu_:
                case addiu_:
                case sub_:
                case subu_:
                case xor_:
                case xoru_:
                case rem_:
                case remu_:
                case seq_:
                case sge_:
                case sgt_:
                case sle_:
                case slt_:
                case sne_:
                    //判断读取的寄存器有没有被加锁
                    if ((ifid_register.current_command.rs != 255 &&
                         register_lock[ifid_register.current_command.rs]) ||
                        (ifid_register.current_command.rt != 255 &&
                         register_lock[ifid_register.current_command.rt]))
                    {
                        data_lock = true; //数据冒险锁上锁，下一次IF不读指令
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    idex_register.rd_num = ifid_register.current_command.rd;
                    register_lock[ifid_register.current_command.rd]++; //即将写入rd寄存器，rd寄存器加一层锁
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //判断第三个参数是立即数还是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case neg_:
                case negu_:
                    if (ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    register_lock[ifid_register.current_command.rd]++; //即将写入rd，rd加锁
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case mul_:
                case mulu_:
                case div_:
                case divu_:
                    //判断读取的寄存器有没有被加锁
                    if ((ifid_register.current_command.rs != 255 &&
                         register_lock[ifid_register.current_command.rs]) ||
                        (ifid_register.current_command.rt != 255 &&
                         register_lock[ifid_register.current_command.rt]))
                    {
                        data_lock = true; //复位ifid寄存器
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    if (ifid_register.current_command.rd == 255) //判断二参数乘除还是三参数乘除
                    {
                        register_lock[32]++; // 即将写入hi寄存器，hi寄存器加锁
                        register_lock[33]++; // 即将写入lo寄存器，lo寄存器加锁
                        idex_register.rd_num = 65;
                    }
                    else
                    {
                        register_lock[ifid_register.current_command.rd]++; //rd寄存器加锁
                        idex_register.rd_num = ifid_register.current_command.rd;
                    }
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //判断第三个参数是立即数还是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    ifid_register = register_ifid(); //ifid寄存器复位
                    break;
                case beq_:
                case bne_:
                case bge_:
                case ble_:
                case bgt_:
                case blt_:
                    //判断读取的寄存器有没有被加锁
                    if ((ifid_register.current_command.rs != 255 &&
                         register_lock[ifid_register.current_command.rs]) ||
                        (ifid_register.current_command.rt != 255 &&
                         register_lock[ifid_register.current_command.rt]))
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    control_lock = true; //分支跳转锁
                    idex_register.rd_num = 34; //分支跳转命令将会写入pc寄存器
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //第二个参数是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    idex_register.address = ifid_register.current_command.address;
                    ifid_register = register_ifid(); //ifid寄存器复位
                    break;
                case beqz_:
                case bnez_:
                case blez_:
                case bgez_:
                case bgtz_:
                case bltz_:
                    //判断读取的寄存器有没有被加锁
                    if (ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    control_lock = true;
                    idex_register.rd_num = 34;
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    idex_register.address = ifid_register.current_command.address;
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case la_:
                case lb_:
                case lh_:
                case lw_:
                    //判断读取的寄存器有没有被加锁
                    if (ifid_register.current_command.rt != 255 && register_lock[ifid_register.current_command.rt])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    if (ifid_register.current_command.rt == 255) //这是个label
                    {
                        idex_register.address = ifid_register.current_command.address;
                    }
                    else
                    {
                        idex_register.address = register_slot[ifid_register.current_command.rt];
                    }
                    register_lock[ifid_register.current_command.rd]++; //即将写入rd寄存器，加锁
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.offset = ifid_register.current_command.offset;
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case sb_:
                case sh_:
                case sw_:
                    //判断读取的寄存器有没有被加锁
                    if ((ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs]) || (ifid_register.current_command.rt != 255 && register_lock[ifid_register.current_command.rt]))
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    if (ifid_register.current_command.rt == 255) //这是个label
                    {
                        idex_register.address = ifid_register.current_command.address;
                    }
                    else
                    {
                        idex_register.address = register_slot[ifid_register.current_command.rt];
                    }
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    idex_register.offset = ifid_register.current_command.offset;
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case b_:
                case j_:
                    control_lock = true; // 分支跳转锁
                    idex_register.address = ifid_register.current_command.address;
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case jr_:
                    if (register_lock[ifid_register.current_command.rs])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    control_lock = true; // 分支跳转锁
                    idex_register.address = register_slot[ifid_register.current_command.rs];
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case jal_:
                    control_lock = true;
                    register_lock[31]++; //锁ra
                    idex_register.rs.w_data_unsigned = register_slot[34].w_data_unsigned;
                    idex_register.address = ifid_register.current_command.address;
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case jalr_:
                    if (register_lock[ifid_register.current_command.rs])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    control_lock = true;
                    register_lock[31]++; //锁ra
                    idex_register.rs.w_data_unsigned = register_slot[34].w_data_unsigned;
                    idex_register.address = register_slot[ifid_register.current_command.rs];
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case li_:
                    register_lock[ifid_register.current_command.rd]++; //rd寄存器加锁
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case move_:
                    //判断读取的寄存器有没有被加锁
                    if (ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    register_lock[ifid_register.current_command.rd]++;
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case mfhi_:
                    if (register_lock[32])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    register_lock[ifid_register.current_command.rd]++;
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rs = register_slot[32];
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case mflo_:
                    if (register_lock[33])
                    {
                        data_lock = true;
                        memset(&idex_register, 0, sizeof(idex_register)); //复位idex寄存器
                        break;
                    }
                    register_lock[ifid_register.current_command.rd]++;
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rs = register_slot[33];
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case nop_:
                    ifid_register = register_ifid(); //复位ifid寄存器
                    break;
                case syscall_:
                    syscall_ID();
                    break;
                case label:
                    break;
                case _align:
                case _ascii:
                case _asciiz:
                case _byte:
                case _half:
                case _word:
                case _space:
                case _data:
                case _text:
                    std::cerr << "Error occured in ID" << std::endl;
                    break;
            }
        }
        
        void Execution()
        {
            exmem_register.OPT = idex_register.OPT;
            if (idex_register.OPT == 0) //本行被stall了
                return;
            switch (static_cast<CommandType>(idex_register.OPT))
            {
                case add_:
                    exmem_register.reg_data.w_data_signed = idex_register.rs.w_data_signed + idex_register.rt_imm_num.w_data_signed;
                    exmem_register.rd = idex_register.rd_num;
                    break;
                case addu_:
                    exmem_register.reg_data.w_data_unsigned = idex_register.rs.w_data_unsigned + idex_register.rt_imm_num.w_data_unsigned;
                    exmem_register.rd = idex_register.rd_num;
                    break;
                case addiu_:
                    exmem_register.reg_data.w_data_unsigned = idex_register.rs.w_data_unsigned + idex_register.rt_imm_num.w_data_unsigned;
                    exmem_register.rd = idex_register.rd_num;
                    break;
                case sub_:
                    exmem_register.reg_data.w_data_signed = idex_register.rs.w_data_signed - idex_register.rt_imm_num.w_data_signed;
                    exmem_register.rd = idex_register.rd_num;
                    break;
                case subu_:
                    exmem_register.reg_data.w_data_unsigned = idex_register.rs.w_data_unsigned - idex_register.rt_imm_num.w_data_unsigned;
                    exmem_register.rd = idex_register.rd_num;
                    break;
                case mul_:
                    if (idex_register.rd_num == 65) //二参数
                    {
                        exmem_register.rd = 65;
                        mul_overflow(idex_register.rs, idex_register.rt_imm_num, exmem_register.reg_data, exmem_register.reg_data2);
                    }
                    else
                    {
                        exmem_register.rd = idex_register.rd_num;
                        exmem_register.reg_data.w_data_signed = idex_register.rs.w_data_signed * idex_register.rt_imm_num.w_data_signed;
                    }
                    break;
                case mulu_:
                    if (idex_register.rd_num == 65) //二参数
                    {
                        exmem_register.rd = 65;
                        mulu_overflow(idex_register.rs, idex_register.rt_imm_num, exmem_register.reg_data, exmem_register.reg_data2);
                    }
                    else
                    {
                        exmem_register.rd = idex_register.rd_num;
                        exmem_register.reg_data.w_data_unsigned = idex_register.rs.w_data_unsigned * idex_register.rt_imm_num.w_data_unsigned;
                    }
                    break;
                case div_:
                    if (idex_register.rd_num == 65) //二参数
                    {
                        exmem_register.rd = 65;
                        div_overflow(idex_register.rs, idex_register.rt_imm_num, exmem_register.reg_data, exmem_register.reg_data2);
                    }
                    else
                    {
                        exmem_register.rd = idex_register.rd_num;
                        exmem_register.reg_data.w_data_signed = idex_register.rs.w_data_signed / idex_register.rt_imm_num.w_data_signed;
                    }
                    break;
                case divu_:
                    if (idex_register.rd_num == 65) //二参数
                    {
                        exmem_register.rd = 65;
                        divu_overflow(idex_register.rs, idex_register.rt_imm_num, exmem_register.reg_data, exmem_register.reg_data2);
                    }
                    else
                    {
                        exmem_register.rd = idex_register.rd_num;
                        exmem_register.reg_data.w_data_unsigned = idex_register.rs.w_data_unsigned / idex_register.rt_imm_num.w_data_unsigned;
                    }
                    break;
                case xor_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = idex_register.rs.w_data_signed ^ idex_register.rt_imm_num.w_data_signed;
                    break;
                case xoru_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_unsigned = idex_register.rs.w_data_unsigned ^ idex_register.rt_imm_num.w_data_unsigned;
                    break;
                case neg_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = -idex_register.rs.w_data_signed;
                    break;
                case negu_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_unsigned = ~idex_register.rs.w_data_unsigned;
                    break;
                case rem_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = idex_register.rs.w_data_signed % idex_register.rt_imm_num.w_data_signed;
                    break;
                case remu_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_unsigned = idex_register.rs.w_data_unsigned % idex_register.rt_imm_num.w_data_unsigned;
                    break;
                case li_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data = idex_register.rt_imm_num;
                    break;
                case seq_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = static_cast<int>(idex_register.rs.w_data_signed == idex_register.rt_imm_num.w_data_signed);
                    break;
                case sge_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = static_cast<int>(idex_register.rs.w_data_signed >= idex_register.rt_imm_num.w_data_signed);
                    break;
                case sgt_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = static_cast<int>(idex_register.rs.w_data_signed > idex_register.rt_imm_num.w_data_signed);
                    break;
                case sle_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = static_cast<int>(idex_register.rs.w_data_signed <= idex_register.rt_imm_num.w_data_signed);
                    break;
                case slt_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = static_cast<int>(idex_register.rs.w_data_signed < idex_register.rt_imm_num.w_data_signed);
                    break;
                case sne_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data.w_data_signed = static_cast<int>(idex_register.rs.w_data_signed != idex_register.rt_imm_num.w_data_signed);
                    break;
                case b_:
                    register_slot[34] = idex_register.address; //直接写入pc
                    control_lock = false; //解开分支跳转锁
                    break;
                case beq_:
                    if (idex_register.rs.w_data_signed == idex_register.rt_imm_num.w_data_signed)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case bne_:
                    if (idex_register.rs.w_data_signed != idex_register.rt_imm_num.w_data_signed)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case bge_:
                    if (idex_register.rs.w_data_signed >= idex_register.rt_imm_num.w_data_signed)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case ble_:
                    if (idex_register.rs.w_data_signed <= idex_register.rt_imm_num.w_data_signed)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case bgt_:
                    if (idex_register.rs.w_data_signed > idex_register.rt_imm_num.w_data_signed)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case blt_:
                    if (idex_register.rs.w_data_signed < idex_register.rt_imm_num.w_data_signed)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case beqz_:
                    if (idex_register.rs.w_data_signed == 0)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case bnez_:
                    if (idex_register.rs.w_data_signed != 0)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case blez_:
                    if (idex_register.rs.w_data_signed <= 0)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case bgez_:
                    if (idex_register.rs.w_data_signed >= 0)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case bgtz_:
                    if (idex_register.rs.w_data_signed > 0)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case bltz_:
                    if (idex_register.rs.w_data_signed < 0)
                        register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case j_:
                    register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case jr_:
                    register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case jal_:
                    exmem_register.rd = 31;
                    exmem_register.reg_data = idex_register.rs;
                    register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case jalr_:
                    exmem_register.rd = 31;
                    exmem_register.reg_data = idex_register.rs;
                    register_slot[34] = idex_register.address;
                    control_lock = false;
                    break;
                case la_:
                case lb_:
                case lh_:
                case lw_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.address.w_data_address = idex_register.address.w_data_unsigned + idex_register.offset.w_data_signed;
                    break;
                case sb_:
                case sh_:
                case sw_:
                    exmem_register.reg_data = idex_register.rs;
                    exmem_register.address.w_data_address = idex_register.address.w_data_unsigned + idex_register.offset.w_data_signed;
                    break;
                case move_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data = idex_register.rs;
                    break;
                case mfhi_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data = idex_register.rs;
                    break;
                case mflo_:
                    exmem_register.rd = idex_register.rd_num;
                    exmem_register.reg_data = idex_register.rs;
                    break;
                case nop_:
                    break;
                case syscall_:
                    syscall_EX();
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
                    std::cerr << "Error occured in the pipeline execution" << std::endl;
                    break;
            }
            memset(&idex_register, 0, sizeof(idex_register));
        }
        
        void Memory_Access(unsigned int &data_memory_pos, char *data_memory_bottom)
        {
            memwb_register.OPT = exmem_register.OPT;
            if (exmem_register.OPT == 0) //命令stall或分支条件不成立，直接跳过
                return;
            switch (static_cast<CommandType>(exmem_register.OPT))
            {
                case add_:
                case addu_:
                case addiu_:
                case sub_:
                case subu_:
                case xor_:
                case xoru_:
                case neg_:
                case negu_:
                case rem_:
                case remu_:
                case li_:
                case seq_:
                case sge_:
                case sgt_:
                case sle_:
                case slt_:
                case sne_:
                case move_:
                case mfhi_:
                case mflo_:
                    memwb_register.rd = exmem_register.rd;
                    memwb_register.reg_data = exmem_register.reg_data;
                    break;
                case mul_:
                case mulu_:
                case div_:
                case divu_:
                    if (exmem_register.rd == 65) //同时写入lo 和 hi
                    {
                        memwb_register.rd = 65;
                        memwb_register.reg_data = exmem_register.reg_data;
                        memwb_register.reg_data2 = exmem_register.reg_data2;
                    }
                    else
                    {
                        memwb_register.rd = exmem_register.rd;
                        memwb_register.reg_data = exmem_register.reg_data;
                    }
                    break;
                case jal_:
                case jalr_:
                    memwb_register.rd = 31;
                    memwb_register.reg_data = exmem_register.reg_data;
                    break;
                case la_:
                    memwb_register.rd = exmem_register.rd;
                    memwb_register.reg_data = exmem_register.address;
                    break;
                case lb_:
                    memwb_register.rd = exmem_register.rd;
                    memset(memwb_register.reg_data.b, 0, sizeof(word));
                    memcpy(memwb_register.reg_data.b, exmem_register.address.w_data_address + data_memory_bottom, 1);
                    break;
                case lh_:
                    memwb_register.rd = exmem_register.rd;
                    memset(memwb_register.reg_data.b, 0, sizeof(word));
                    memcpy(memwb_register.reg_data.b, exmem_register.address.w_data_address + data_memory_bottom, 2);
                    break;
                case lw_:
                    memwb_register.rd = exmem_register.rd;
                    memset(memwb_register.reg_data.b, 0, sizeof(word));
                    memcpy(memwb_register.reg_data.b, exmem_register.address.w_data_address + data_memory_bottom, 4);
                    break;
                case sb_:
                    memcpy(exmem_register.address.w_data_address + data_memory_bottom, exmem_register.reg_data.b, 1);
                    mem_lock = true;
                    break;
                case sh_:
                    memcpy(exmem_register.address.w_data_address + data_memory_bottom, exmem_register.reg_data.b, 2);
                    mem_lock = true;
                    break;
                case sw_:
                    memcpy(exmem_register.address.w_data_address + data_memory_bottom, exmem_register.reg_data.b, 4);
                    mem_lock = true;
                    break;
                case syscall_:
                    syscall_MA(data_memory_bottom, data_memory_pos);
                    break;
                    
                case b_:
                case beq_:
                case bne_:
                case bge_:
                case ble_:
                case bgt_:
                case blt_:
                case beqz_:
                case bnez_:
                case blez_:
                case bgez_:
                case bgtz_:
                case bltz_:
                case j_:
                case jr_:
                case nop_:
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
                    std::cerr << "Error occured in MA" << std::endl;
                    break;
            }
            memset(&exmem_register, 0, sizeof(exmem_register));
        }
        
        void Write_Back()
        {
            if (memwb_register.OPT == 0) //stall或分支条件不成立，直接跳过
                return;
//            cycle_cnt++;
//            std::cout << "cycle:" << cycle_cnt << std::endl;
//            for (int i = 0; i < 34; i++)
//                std::cout << register_slot[i].w_data_unsigned << ' ';
//            std::cout << std::endl;
            switch (static_cast<CommandType>(memwb_register.OPT))
            {
                case add_:
                case addu_:
                case addiu_:
                case sub_:
                case subu_:
                case xor_:
                case xoru_:
                case neg_:
                case negu_:
                case rem_:
                case remu_:
                case li_:
                case seq_:
                case sge_:
                case sgt_:
                case sle_:
                case slt_:
                case sne_:
                case move_:
                case mfhi_:
                case mflo_:
                    register_slot[memwb_register.rd] = memwb_register.reg_data;
                    register_lock[memwb_register.rd]--; //寄存器写入完成，解锁寄存器
                    break;
                case mul_:
                case mulu_:
                case div_:
                case divu_:
                    if (memwb_register.rd == 65) //同时写入lo 和 hi
                    {
                        register_slot[32] = memwb_register.reg_data;
                        register_slot[33] = memwb_register.reg_data2;
                        register_lock[32]--;
                        register_lock[33]--;
                    }
                    else
                    {
                        register_slot[memwb_register.rd] = memwb_register.reg_data;
                        register_lock[memwb_register.rd]--;
                    }
                    break;
                case la_:
                case lb_:
                case lh_:
                case lw_:
                    register_slot[memwb_register.rd] = memwb_register.reg_data;
                    register_lock[memwb_register.rd]--;
                    break;
                case sb_:
                case sh_:
                case sw_:
                    mem_lock = false;
                    break;
                case jal_:
                case jalr_:
                    register_slot[31] = memwb_register.reg_data;
                    register_lock[31]--;
                    break;
                case syscall_:
                    syscall_WB();
                    break;
                    
                case nop_:
                case b_:
                case beq_:
                case bne_:
                case bge_:
                case ble_:
                case bgt_:
                case blt_:
                case beqz_:
                case bnez_:
                case blez_:
                case bgez_:
                case bgtz_:
                case bltz_:
                case j_:
                case jr_:
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
                    std::cerr << "Error occured in WB" << std::endl;
                    break;
            }
            data_lock = false;
            memset(&memwb_register, 0, sizeof(memwb_register));
        }
        
    public:
        decoder(unsigned int start_line, word *obj_register_slot)
        {
            register_slot = obj_register_slot;
            memset(&idex_register, 0, sizeof(idex_register));
            memset(&exmem_register, 0, sizeof(exmem_register));
            memset(&memwb_register, 0, sizeof(memwb_register));
            memset(register_lock, 0, 36 * sizeof(char));
            data_lock = false;
            mem_lock = false;
            control_lock = false;
            register_slot[34].w_data_unsigned = start_line;
            tmpstr = "";
            //             for debug
            cycle_cnt = 0;
            debug_map[0] = "label";
            debug_map[1] = "align";
            debug_map[2] = "ascii";
            debug_map[3] = "asciiz";
            debug_map[4] = "byte";
            debug_map[5] = "half";
            debug_map[6] = "word";
            debug_map[7] = "space";
            debug_map[8] = "data";
            debug_map[9] = "text";
            debug_map[10] = "add";
            debug_map[11] = "addu";
            debug_map[12] = "addiu";
            debug_map[13] = "sub";
            debug_map[14] = "subu";
            debug_map[15] = "mul";
            debug_map[16] = "mulu";
            debug_map[17] = "div";
            debug_map[18] = "divu";
            debug_map[19] = "xor";
            debug_map[20] = "xoru";
            debug_map[21] = "neg";
            debug_map[22] = "negu";
            debug_map[23] = "rem";
            debug_map[24] = "remu";
            debug_map[25] = "li";
            debug_map[26] = "seq";
            debug_map[27] = "sge";
            debug_map[28] = "sgt";
            debug_map[29] = "sle";
            debug_map[30] = "slt";
            debug_map[31] = "sne";
            debug_map[32] = "b";
            debug_map[33] = "beq";
            debug_map[34] = "bne";
            debug_map[35] = "bge";
            debug_map[36] = "ble";
            debug_map[37] = "bgt";
            debug_map[38] = "blt";
            debug_map[39] = "beqz";
            debug_map[40] = "bnez";
            debug_map[41] = "blez";
            debug_map[42] = "bgez";
            debug_map[43] = "bgtz";
            debug_map[44] = "bltz";
            debug_map[45] = "j";
            debug_map[46] = "jr";
            debug_map[47] = "jal";
            debug_map[48] = "jalr";
            debug_map[49] = "la";
            debug_map[50] = "lb";
            debug_map[51] = "lh";
            debug_map[52] = "lw";
            debug_map[53] = "sb";
            debug_map[54] = "sh";
            debug_map[55] = "sw";
            debug_map[56] = "move";
            debug_map[57] = "mfhi";
            debug_map[58] = "mflo";
            debug_map[59] = "nop";
            debug_map[60] = "syscall";
        
        }
        ~decoder() = default;
    
        void pipeline(unsigned int &data_memory_pos, char *data_memory_bottom, std::vector<command> &text_memory)
        {
            while (true)
            {
                Write_Back();
                Memory_Access(data_memory_pos, data_memory_bottom);
                Execution();
                Instruction_Decode_Data_Preparation();
                Instruction_Fetch(text_memory);
            }
        }
        
    };
}
#endif //MIPS_SIMULATOR_PIPELINE_H
