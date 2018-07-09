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
        char stall_cycle; //当前需要stall的周期数
        bool reg_lock[36]; //寄存器锁
        bool mem_lock; //总线内存锁
        bool control_lock; //分支语句锁
    
        bool check_control_lock(const CommandType &cmd)
        {
            return cmd == b_ || cmd == beq_ || cmd == bne_ || cmd == bge_ || cmd == ble_ || cmd == bgt_ || cmd == blt_ || cmd == beqz_ || cmd == bnez_ || cmd == blez_ || cmd == bgez_ || cmd == bgtz_ || cmd == bltz_ || cmd == j_ || cmd == jr_ || cmd == jal_ || cmd == jalr_;
        }
        
        void Instruction_Fetch(std::vector<command> &text_memory)
        {
            if (mem_lock || control_lock) //内存总线锁和分支跳转锁
            {
                ifid_register.current_command.OPT = 0;
                return;
            }
            //寄存器锁check
            if (reg_lock[text_memory[register_slot[34].w_data_unsigned].rs] || (text_memory[register_slot[34].w_data_unsigned].rt != 255 && reg_lock[text_memory[register_slot[34].w_data_unsigned].rt]))
            {
                ifid_register.current_command.OPT = 0;
                return;
            }
            //注意lo和hi的锁
            if ((text_memory[register_slot[34].w_data_unsigned].OPT == mfhi_ && reg_lock[32]) || (text_memory[register_slot[34].w_data_unsigned].OPT == mflo_ && reg_lock[33]))
            {
                ifid_register.current_command.OPT = 0;
                return;
            }
            
            ifid_register.current_command = text_memory[register_slot[34].w_data_unsigned];
            if(check_control_lock(static_cast<CommandType>(ifid_register.current_command.OPT))) //如果是分支跳转语句，上锁
                control_lock = true;
        }
        
        void Instruction_Decode_Data_Preparation()
        {
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
                    idex_register.rd_num = ifid_register.current_command.rd;
                    reg_lock[ifid_register.current_command.rd] = true; //即将写入rd寄存器，rd寄存器加锁
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //判断第三个参数是立即数还是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    break;
                case neg_:
                case negu_:
                    break;
                case mul_:
                case mulu_:
                case div_:
                case divu_:
                    if(ifid_register.current_command.rd == 255) //判断二参数乘除还是三参数乘除
                    {
                        reg_lock[32] = true; // 即将写入hi寄存器，hi寄存器加锁
                        reg_lock[33] = true; // 即将写入lo寄存器，lo寄存器加锁
                        idex_register.rd_num = 65;
                    }
                    else
                    {
                        reg_lock[ifid_register.current_command.rd] = true;
                        idex_register.rd_num = ifid_register.current_command.rd;
                    }
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //判断第三个参数是立即数还是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    break;
                case beq_:
                case bne_:
                case bge_:
                case ble_:
                case bgt_:
                case blt_:
                    idex_register.rd_num = 34; //分支跳转命令将会写入pc寄存器
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    if (ifid_register.current_command.rt != 255) //第二个参数是寄存器
                        idex_register.rt_imm_num = register_slot[ifid_register.current_command.rt];
                    else idex_register.rt_imm_num = ifid_register.current_command.imm_num;
                    break;
                case beqz_:
                case bnez_:
                case blez_:
                case bgez_:
                case bgtz_:
                case bltz_:
                    idex_register.rd_num = 34;
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    break;
                case la_:
                case lb_:
                case lh_:
                case lw_:
                    idex_register.rd_num = ifid_register.current_command.rd;
                    idex_register.address = ifid_register.current_command.address;
                    idex_register.offset = ifid_register.current_command.offset;
                    break;
                case sb_:
                case sh_:
                case sw_:
                    idex_register.rs = register_slot[ifid_register.current_command.rs];
                    idex_register.address = ifid_register.current_command.address;
                    idex_register.offset = ifid_register.current_command.offset;
                    break;
                case b_:
                case j_:
                    idex_register.address = ifid_register.current_command.address;
                    break;
                case jr_:
                    idex_register.address = register_slot[ifid_register.current_command.rs];
                    break;
                case jal_:
                    reg_lock[34] = true; //锁pc寄存器
                    idex_register.address = ifid_register.current_command.address;
                    break;
                case jalr_:
                    reg_lock[34] = true;
                    idex_register.address = register_slot[ifid_register.current_command.rs];
                    break;
                case li_:
                case move_:
                    reg_lock[ifid_register.current_command.rs] = true;
                    break;
                case mfhi_:break;
                case mflo_:break;
                case nop_:break;
                case syscall_:break;
                
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
            switch (static_cast<CommandType>(idex_register.OPT))
            {
                case add_:break;
                case addu_:break;
                case addiu_:break;
                case sub_:break;
                case subu_:break;
                case mul_:break;
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
