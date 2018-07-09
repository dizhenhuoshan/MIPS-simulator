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
        char *data_memory; //假内存的头指针
        register_ifid ifid_register; // IF/ID寄存器
        register_idex idex_register; // ID/EX寄存器 注意：如果是对low，high寄存器同时操作，rd_num为 32 + 33 = 65
        register_exmem exmem_register; // EX/MEM寄存器
        register_memwb memwb_register; // MEM/WB寄存器
        bool register_lock[36]; //寄存器锁
        bool mem_lock; //总线内存锁
        bool control_lock; //分支语句锁
    
        bool check_control_lock(const CommandType &cmd)
        {
            return cmd == b_ || cmd == beq_ || cmd == bne_ || cmd == bge_ || cmd == ble_ || cmd == bgt_ || cmd == blt_ || cmd == beqz_ || cmd == bnez_ || cmd == blez_ || cmd == bgez_ || cmd == bgtz_ || cmd == bltz_ || cmd == j_ || cmd == jr_ || cmd == jal_ || cmd == jalr_;
        }
        
        bool syscall_ID()
        {
            if (register_lock[2])
            {
                ifid_register.current_command.OPT = 0;
                idex_register.OPT = 0;
                return false;
            }
            idex_register.OPT = syscall_;
            switch (register_slot[2].w_data_unsigned)
            {
                case 1:
                    if (register_lock[4])
                    {
                        ifid_register.current_command.OPT = 0;
                        idex_register.OPT = 0;
                        return false;
                    }
                    idex_register.offset.w_data_unsigned = 1;
                    idex_register.rt_imm_num = register_slot[4];
                    return true;
                case 4:
                    if (register_lock[4])
                    {
                        ifid_register.current_command.OPT = 0;
                        idex_register.OPT = 0;
                        return false;
                    }
                    idex_register.offset.w_data_unsigned = 4;
                    idex_register.address = register_slot[4];
                    return true;
                case 5:
                    register_lock[2] = true; //v0寄存器上锁
                    idex_register.offset.w_data_unsigned = 5;
                    return true;
                case 8:
                    if (register_lock[4] || register_lock[5]) //检查a0、a1有没有被锁
                    {
                        ifid_register.current_command.OPT = 0;
                        idex_register.OPT = 0;
                        return false;
                    }
                    idex_register.offset.w_data_unsigned = 8;
                    idex_register.address = register_slot[4];
                    idex_register.rt_imm_num = register_slot[5];
                    return true;
                case 9:
                    if (register_lock[4])
                    {
                        ifid_register.current_command.OPT = 0;
                        idex_register.OPT = 0;
                        return false;
                    }
                    register_lock[2] = true;
                    idex_register.offset.w_data_unsigned = 9;
                    idex_register.rt_imm_num = register_slot[4];
                    return true;
                case 10:
                    exit(0);
                case 17:
                    if (register_lock[4])
                    {
                        ifid_register.current_command.OPT = 0;
                        idex_register.OPT = 0;
                        return false;
                    }
                    exit(register_slot[4].w_data_signed);
                default:
                    std::cerr << "Error occured in syscall_ID" << std::endl;
                    break;
            }
        }
        
        bool Instruction_Fetch(std::vector<command> &text_memory)
        {
            if (control_lock || mem_lock) //内存锁和控制线锁
            {
                ifid_register.current_command.OPT = 0;
                return false;
            }
            ifid_register.current_command = text_memory[register_slot[35].w_data_unsigned];
            register_slot[35].w_data_unsigned++;
            return true;
        }
        
        bool Instruction_Decode_Data_Preparation()
        {
            bool flag = true; //返回值
            if (ifid_register.current_command.OPT == label) //本行被stall了，不执行命令
                flag = true;
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
                    if ((ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs]) || (ifid_register.current_command.rt != 255 && register_lock[ifid_register.current_command.rt]))
                    {
                        ifid_register.current_command.OPT = 0; //复位ifid寄存器
                        idex_register.OPT = 0; //复位idex寄存器
                        flag = false;
                        break;
                    }
                    idex_register.rd_num = ifid_register.current_command.rd;
                    register_lock[ifid_register.current_command.rd] = true; //即将写入rd寄存器，rd寄存器加锁
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //判断第三个参数是立即数还是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case neg_:
                case negu_:
                    break;
                case mul_:
                case mulu_:
                case div_:
                case divu_:
                    //判断读取的寄存器有没有被加锁
                    if ((ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs]) || (ifid_register.current_command.rt != 255 && register_lock[ifid_register.current_command.rt]))
                    {
                        ifid_register.current_command.OPT = 0; //复位ifid寄存器
                        idex_register.OPT = 0; //复位idex寄存器
                        flag = false;
                        break;
                    }
                    if(ifid_register.current_command.rd == 255) //判断二参数乘除还是三参数乘除
                    {
                        register_lock[32] = true; // 即将写入hi寄存器，hi寄存器加锁
                        register_lock[33] = true; // 即将写入lo寄存器，lo寄存器加锁
                        idex_register.rd_num = 65;
                    }
                    else
                    {
                        register_lock[ifid_register.current_command.rd] = true; //rd寄存器加锁
                        idex_register.rd_num = ifid_register.current_command.rd;
                    }
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //判断第三个参数是立即数还是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    ifid_register.current_command.OPT = 0; //ifid寄存器复位
                    flag = true;
                    break;
                case beq_:
                case bne_:
                case bge_:
                case ble_:
                case bgt_:
                case blt_:
                    //判断读取的寄存器有没有被加锁
                    if ((ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs]) || (ifid_register.current_command.rt != 255 && register_lock[ifid_register.current_command.rt]))
                    {
                        ifid_register.current_command.OPT = 0; //复位ifid寄存器
                        idex_register.OPT = 0; //复位idex寄存器
                        flag = false;
                        break;
                    }
                    control_lock = true; //分支跳转锁
                    idex_register.rd_num = 34; //分支跳转命令将会写入pc寄存器
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //第二个参数是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    ifid_register.current_command.OPT = 0; //ifid寄存器复位
                    flag = true;
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
                        ifid_register.current_command.OPT = 0; //复位ifid寄存器
                        idex_register.OPT = 0; //复位idex寄存器
                        flag = false;
                        break;
                    }
                    control_lock = true;
                    idex_register.rd_num = 34;
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    ifid_register.current_command.OPT = 0;
                    flag = true;
                    break;
                case la_:
                case lb_:
                case lh_:
                case lw_:
                    register_lock[ifid_register.current_command.rd] = true; //即将写入rd寄存器，加锁
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.address = ifid_register.current_command.address;
                    idex_register.offset = ifid_register.current_command.offset;
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case sb_:
                case sh_:
                case sw_:
                    //判断读取的寄存器有没有被加锁
                    if (ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs])
                    {
                        ifid_register.current_command.OPT = 0; //复位ifid寄存器
                        idex_register.OPT = 0; //复位idex寄存器
                        flag = false;
                        break;
                    }
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    idex_register.address = ifid_register.current_command.address;
                    idex_register.offset = ifid_register.current_command.offset;
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case b_:
                case j_:
                    control_lock = true; // 分支跳转锁
                    idex_register.address = ifid_register.current_command.address;
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case jr_:
                    control_lock = true; // 分支跳转锁
                    idex_register.address = register_slot[ifid_register.current_command.rs];
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case jal_:
                    control_lock = true;
                    register_lock[34] = true; //锁pc寄存器
                    idex_register.address = ifid_register.current_command.address;
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case jalr_:
                    control_lock = true;
                    register_lock[34] = true; //锁pc
                    idex_register.address = register_slot[ifid_register.current_command.rs];
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case li_:
                    register_lock[ifid_register.current_command.rd] = true; //rd寄存器加锁
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                case move_:
                    //判断读取的寄存器有没有被加锁
                    if (ifid_register.current_command.rs != 255 && register_lock[ifid_register.current_command.rs])
                    {
                        ifid_register.current_command.OPT = 0; //复位ifid寄存器
                        idex_register.OPT = 0; //复位idex寄存器
                        flag = false;
                        break;
                    }
                    register_lock[ifid_register.current_command.rs] = true;
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rt_imm_num = register_slot[ifid_register.current_command.rs];
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case mfhi_:
                    if (register_lock[32])
                    {
                        ifid_register.current_command.OPT = 0;
                        idex_register.OPT = 0;
                        flag = false;
                        break;
                    }
                    register_lock[ifid_register.current_command.rd] = true;
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rt_imm_num = register_slot[32];
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case mflo_:
                    if (register_lock[33])
                    {
                        ifid_register.current_command.OPT = 0;
                        idex_register.OPT = 0;
                        flag = false;
                        break;
                    }
                    register_lock[ifid_register.current_command.rd] = true;
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.rt_imm_num = register_slot[33];
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case nop_:
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
                    flag = true;
                    break;
                case syscall_:
                    flag = syscall_ID();
                    ifid_register.current_command.OPT = 0; //复位ifid寄存器
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
            return flag;
        }
        
        bool Execution()
        {
            bool flag = true;
            exmem_register.OPT = idex_register.OPT;
            if (idex_register.OPT == 0)
                return true;
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
                    if (idex_register.rd_num == 65)
                    {
                        exmem_register.rd = 65;
                    }
                    break;
                case mulu_:break;
                case div_:break;
                case divu_:break;
                case xor_:break;
                case xoru_:break;
                case neg_:break;
                case negu_:break;
                case rem_:break;
                case remu_:break;
                case li_:break;
                case seq_:break;
                case sge_:break;
                case sgt_:break;
                case sle_:break;
                case slt_:break;
                case sne_:break;
                case b_:break;
                case beq_:break;
                case bne_:break;
                case bge_:break;
                case ble_:break;
                case bgt_:break;
                case blt_:break;
                case beqz_:break;
                case bnez_:break;
                case blez_:break;
                case bgez_:break;
                case bgtz_:break;
                case bltz_:break;
                case j_:break;
                case jr_:break;
                case jal_:break;
                case jalr_:break;
                case la_:break;
                case lb_:break;
                case lh_:break;
                case lw_:break;
                case sb_:break;
                case sh_:break;
                case sw_:break;
                case move_:break;
                case mfhi_:break;
                case mflo_:break;
                case nop_:break;
                case syscall_:break;
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
        }
        
        void Memory_Access()
        {
        
        }
        
        void Write_Back()
        {
        
        }
        
    public:
        decoder()
        {
        
        }
        ~decoder()
        {
        
        }
        
        void pipeline()
        {
        
        }
        
    };
}
#endif //MIPS_SIMULATOR_PIPELINE_H
