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
        unsigned int current_line; //当前执行到的行号
        command current_command;
        bool flag;
        
        //加减运算
        void command_add_sub(bool unsigned_flag)
        {}
        
    public:
        decoder()
        {
            current_line = 0;
            flag = true;
        }
        ~decoder() = default;
        
        void decode_command(std::vector<mips::command> &text_memory, char *&data_memory_bottom, char *&data_memory_pos, word *register_slot, std::map<std::string, char*> &data_label, std::map<std::string, label_info> &text_label)
        {
            while (flag)
            {
                current_command = text_memory[current_line];
                switch (current_command.OPT)
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
                    case label:break;
                }
            }
        }
    };
};


#endif //MIPS_SIMULATOR_DECODER_H
