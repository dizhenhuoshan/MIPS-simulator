//
// Paser for the mips-simulator
//
#ifndef MIPS_SIMULATOR_PASER_H
#define MIPS_SIMULATOR_PASER_H

#include "codeline.h"
#include "constant.h"
#include <map>
#include <cmath>
namespace mips
{
    class paser
    {
    private:
        int arg_num; //该命令中有多少个参数
        std::string *args; //各个参数
        char* inner_ptr; // 辅助strtok
        command tmpcommand; //编码后的代码
        std::map<std::string, CommandType> f_command; //操作符映射表
        
        void load_f_command()
        {
            f_command[".align"] = _align;
            f_command[".ascii"] = _ascii;
            f_command[".asciiz"] = _asciiz;
            f_command[".byte"] = _byte;
            f_command[".half"] = _half;
            f_command[".word"] = _word;
            f_command[".space"] = _space;
            f_command[".data"] = _data;
            f_command[".text"] = _text;
            f_command["add"] = add_;
            f_command["addu"] = addu_;
            f_command["addiu"] = addiu_;
            f_command["sub"] = sub_;
            f_command["subu"] = subu_;
            f_command["mul"] = mul_;
            f_command["mulu"] = mulu_;
            f_command["div"] = div_;
            f_command["divu"] = divu_;
            f_command["xor"] = xor_;
            f_command["xoru"] = xoru_;
            f_command["neg"] = neg_;
            f_command["negu"] = negu_;
            f_command["rem"] = rem_;
            f_command["remu"] = remu_;
            f_command["li"] = li_;
            f_command["seq"] = seq_;
            f_command["sge"] = sge_;
            f_command["sgt"] = sgt_;
            f_command["sle"] = sle_;
            f_command["slt"] = slt_;
            f_command["sne"] = sne_;
            f_command["b"] = b_;
            f_command["beq"] = beq_;
            f_command["bne"] = bne_;
            f_command["bge"] = bge_;
            f_command["ble"] = ble_;
            f_command["bgt"] = bgt_;
            f_command["blt"] = blt_;
            f_command["beqz"] = beqz_;
            f_command["bnez"] = bnez_;
            f_command["blez"] = blez_;
            f_command["bgez"] = bgez_;
            f_command["bgtz"] = bgtz_;
            f_command["bltz"] = bltz_;
            f_command["j"] = j_;
            f_command["jr"] = jr_;
            f_command["jal"] = jal_;
            f_command["jalr"] = jalr_;
            f_command["la"] = la_;
            f_command["lb"] = lb_;
            f_command["lh"] = lh_;
            f_command["lw"] = lw_;
            f_command["sb"] = sb_;
            f_command["sh"] = sh_;
            f_command["sw"] = sw_;
            f_command["move"] = move_;
            f_command["mfhi"] = mfhi_;
            f_command["mflo"] = mflo_;
            f_command["nop"] = nop_;
            f_command["syscall"] = syscall_;
        }
        void split_line(std::string current_line)
        {
            char *tmpline = new char[current_line.size() + 10];
            strcpy(tmpline, current_line.c_str());
            char *token = strtok_r(tmpline, ", ", &inner_ptr);
            while (token != NULL)
            {
                if (token[0] == '#')
                    break;
                arg_num++;
                args[arg_num] = token;
                //注意存字符串时如果字符串里有奇怪的东西要想办法整理一下
//                if (strcmp(token, ".ascii") == 0 || strcmp(token, ".asciiz") == 0)
//                {
//
//                }
                token = strtok_r(tmpline, ", ", &inner_ptr);
            }
        }
        
        CommandType get_command_type()
        {
            return f_command[args[0]];
        }
        
        //计算出对齐内存时应该对应的偏移量(相对于pos)
        unsigned int get_align_pos(const unsigned int &offset, const unsigned int &n)
        {
            unsigned int block_length = static_cast<unsigned int>(pow(2, n));
            return (offset / block_length + 1) * block_length - offset;
        }
        
        //去除string里面一些奇奇怪怪的东西(转义字符) (test passed!
        std::string string_modify(std::string &obj)
        {
            int length = 0;
            std::string tmpstr = obj;
            for (int i = 1; i < obj.size() - 1; i++) //在循环中去掉双引号
            {
                if (obj[i] == '\\')
                {
                    i++;
                    switch (obj[i])
                    {
                        case 'n':
                            tmpstr[length++] = '\n';
                            break;
                        case 't':
                            tmpstr[length++] = '\t';
                            break;
                        case '\\':
                            tmpstr[length++] = '\\';
                            break;
                        case 'b':
                            tmpstr[length++] = '\b';
                            break;
                        case 'r':
                            tmpstr[length++] = '\r';
                            break;
                        case '\?':
                            tmpstr[length++] = '\?';
                            break;
                        default:
                            std::cerr << "string_modify in paser decode error" << std::endl;
                            break;
                    }
                }
                else
                    tmpstr[length++] = obj[i];
            }
            return tmpstr.substr(0, static_cast<unsigned long>(length));
        }
        
        void paser_align(char *data_memory_bottom, char *&data_memory_pos)
        {
            unsigned int n = static_cast<unsigned int> (std::stoi(args[arg_num]));
            unsigned int offset = get_align_pos(static_cast<const unsigned int>(data_memory_pos - data_memory_bottom), n);
            memset(data_memory_pos, 0, offset);
            data_memory_pos += offset;
        }
        
        void paser_ascii(char *&data_memory_pos)
        {
            std::string tmpstr = string_modify(args[arg_num]);
            memcpy(data_memory_pos, tmpstr.c_str(), tmpstr.size());
            data_memory_pos += tmpstr.size();
        }
        
        void paser_asciiz(char *&data_memory_pos)
        {
            std::string tmpstr = string_modify(args[arg_num]);
            memcpy(data_memory_pos, tmpstr.c_str(), tmpstr.size() + 1);
            data_memory_pos += tmpstr.max_size() + 1;
        }
        
        void paser_byte(char *&data_memory_pos)
        {
            byte *byte_arr = new byte[arg_num];
            for (int i = 0; i < arg_num; i++)
            {
                byte_arr[i].b_data = static_cast<char>(std::stoi(args[i]));
            }
            memcpy(data_memory_pos, byte_arr, sizeof(byte_arr));
            data_memory_pos += sizeof(byte_arr);
        }
        
    public:
        paser()
        {
            load_f_command();
            inner_ptr = NULL;
            arg_num = -1;
            args = new std::string[10];
        }
        ~paser() = default;
    
        void paser_code(const int &obj_linenumber, std::string &obj_current_line, std::vector<command> &text_memory, char *data_memory_bottom, char *data_memory_pos, std::map<int, std::string> &unknown_table, char &dtflag)
        {
            split_line(obj_current_line);
            if (arg_num == -1)
                return; //当前读到的为纯注释行，可以丢弃
            command tmp_command;
            switch (get_command_type())
            {
                case _align:
                    paser_align(data_memory_bottom, data_memory_pos);
                    break;
                case _ascii:
                    paser_ascii(data_memory_pos);
                    break;
                case _asciiz:
                    paser_asciiz(data_memory_pos);
                    break;
                case _byte:
                    paser_byte(data_memory_pos);
                    break;
                case _half:
                    paser_half();
                    break;
                case _word:
                    paser_word();
                    break;
                case _space:
                    paser_space();
                    break;
                case _data:
                    paser_data();
                    break;
                case _text:
                    paser_text();
                    break;
                case add_:
                    paser_add();
                    break;
                case addu_:
                    paser_addu();
                    break;
                case addiu_:
                    paser_addiu();
                    break;
                case sub_:
                    paser_sub();
                    break;
                case subu_:
                    paser_subu();
                    break;
                case mul_:
                    paser_mul();
                    break;
                case mulu_:
                    paser_mulu();
                    break;
                case div_:
                    paser_div();
                    break;
                case divu_:
                    paser_divu();
                    break;
                case xor_:
                    paser_xor();
                    break;
                case xoru_:
                    paser_xoru();
                    break;
                case neg_:
                    paser_neg();
                    break;
                case negu_:
                    paser_negu();
                    break;
                case rem_:
                    paser_rem();
                    break;
                case remu_:
                    paser_remu();
                    break;
                case li_:
                    paser_li();
                    break;
                case seq_:
                    paser_seq();
                    break;
                case sge_:
                    paser_sge();
                    break;
                case sgt_:
                    paser_sgt();
                    break;
                case sle_:
                    paser_sle();
                    break;
                case slt_:
                    paser_slt();
                    break;
                case sne_:
                    paser_sne();
                    break;
                case b_:
                    paser_b();
                    break;
                case beq_:
                    paser_beq();
                    break;
                case bne_:
                    paser_bne();
                    break;
                case bge_:
                    paser_bge();
                    break;
                case ble_:
                    paser_ble();
                    break;
                case bgt_:
                    paser_bgt();
                    break;
                case blt_:
                    paser_blt();
                    break;
                case beqz_:
                    paser_beqz();
                    break;
                case bnez_:
                    paser_bnez();
                    break;
                case blez_:
                    paser_blez();
                    break;
                case bgez_:
                    paser_bgez();
                    break;
                case bgtz_:
                    paser_bgtz();
                    break;
                case bltz_:
                    paser_bltz();
                    break;
                case j_:
                    paser_j();
                    break;
                case jr_:
                    paser_jr();
                    break;
                case jal_:
                    paser_jal();
                    break;
                case jalr_:
                    paser_jalr();
                    break;
                case la_:
                    paser_la();
                    break;
                case lb_:
                    paser_lb();
                    break;
                case lh_:
                    paser_lh();
                    break;
                case lw_:
                    paser_lw();
                    break;
                case sb_:
                    paser_sb();
                    break;
                case sh_:
                    paser_sh();
                    break;
                case sw_:
                    paser_sw();
                    break;
                case move_:
                    paser_move();
                    break;
                case mfhi_:
                    paser_mfhi();
                    break;
                case mflo_:
                    paser_mflo();
                    break;
                case nop_:
                    paser_nop();
                    break;
                case syscall_:
                    paser_syscall();
                    break;
            }
        }
    };
};
#endif //MIPS_SIMULATOR_PASER_H
