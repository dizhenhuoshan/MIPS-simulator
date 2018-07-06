//
// Paser for the mips-simulator
//
#ifndef MIPS_SIMULATOR_PASER_H
#define MIPS_SIMULATOR_PASER_H

#include "codeline.h"
#include "constant.h"
#include <cmath>
#include <map>
#include <vector>
#include <sstream>

namespace mips
{
    class paser
    {
    private:
        int arg_num; //该命令中有多少个参数
        std::string *args; //各个参数
        char* inner_ptr; // 辅助strtok
        command tmpcommand; //编码后的代码
        unsigned int text_cnt; //代码区地址(行号 1-based)
        char *data_memory_bottom; //暂存假内存头地址
        std::string tmplabel; //暂存label
        bool text_label_flag; //上一行是不是一个text的label，如果是，本行的操作应写入text的map
        std::map<std::string, CommandType> f_command; //操作符映射表
        std::map<std::string, unsigned char> f_register; //寄存器编号映射表

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
        void load_f_register()
        {
            f_register["$0"] = f_register["$zero"] = 0;
            f_register["$1"] = f_register["$at"] = 1;
            f_register["$2"] = f_register["$v0"] = 2;
            f_register["$3"] = f_register["$v1"] = 3;
            f_register["$4"] = f_register["$a0"] = 4;
            f_register["$5"] = f_register["$a1"] = 5;
            f_register["$6"] = f_register["$a2"] = 6;
            f_register["$7"] = f_register["$a3"] = 7;
            f_register["$8"] = f_register["$t0"] = 8;
            f_register["$9"] = f_register["$t1"] = 9;
            f_register["$10"] = f_register["$t2"] = 10;
            f_register["$11"] = f_register["$t3"] = 11;
            f_register["$12"] = f_register["$t4"] = 12;
            f_register["$13"] = f_register["$t5"] = 13;
            f_register["$14"] = f_register["$t6"] = 14;
            f_register["$15"] = f_register["$t7"] = 15;
            f_register["$16"] = f_register["$s0"] = 16;
            f_register["$17"] = f_register["$s1"] = 17;
            f_register["$18"] = f_register["$s2"] = 18;
            f_register["$19"] = f_register["$s3"] = 19;
            f_register["$20"] = f_register["$s4"] = 20;
            f_register["$21"] = f_register["$s5"] = 21;
            f_register["$22"] = f_register["$s6"] = 22;
            f_register["$23"] = f_register["$s7"] = 23;
            f_register["$24"] = f_register["$t8"] = 24;
            f_register["$25"] = f_register["$t9"] = 25;
            f_register["$26"] = f_register["$k0"] = 26;
            f_register["$27"] = f_register["$k1"] = 27;
            f_register["$28"] = f_register["$gp"] = 28;
            f_register["$29"] = f_register["$sp"] = 29;
            f_register["$30"] = f_register["$fp"] = 30;
            f_register["$31"] = f_register["$ra"] = 31;
            f_register["$hi"] = 32;
            f_register["$lo"] = 33;
            f_register["$pc"] = 34;
        }

        void clear_commmand(command &obj)
        {
            obj.OPT = label;
            obj.rt = obj.rs = obj.rd = 255;
            obj.address.w_data_address = -1;
            obj.offset.w_data_signed = 0;
            obj.imm_num.w_data_signed = 0;
        }

        //判断是register还是label 1-register 0-label
        bool check_register(std::string &obj)
        {
            return f_register.find(obj) != f_register.end();
        }

        void special_split(const std::string &currentline)
        {
            arg_num = 2;
            std::istringstream ss(currentline);
            ss >> args[0];
            unsigned long start = currentline.find_first_of('\"');
            unsigned long length = currentline.find_last_of('\"') - start + 1;
            args[1] = currentline.substr(start, length);
        }

        void split_line(const std::string &current_line)
        {
            arg_num = 0;
            char *tmpline = new char[current_line.size() + 10];
            char *p = tmpline;
            strcpy(tmpline, current_line.c_str());
            char *token = strtok_r(tmpline, ", \t", &inner_ptr);
            if (token == NULL) //空行
                return;
            if (strcmp(token, ".ascii") == 0 || strcmp(token, ".asciiz") == 0)
            {
                special_split(current_line);
            }
            else
            {
                while (token != NULL)
                {
                    if (token[0] == '#')
                    {
                        break;
                    }
                    arg_num++;
                    args[arg_num - 1] = token;
                    tmpline = NULL;
                    token = strtok_r(tmpline, ", \t", &inner_ptr);
                }
            }
            delete [] p;
        }

        //计算出对齐内存时应该对应的偏移量(相对于pos)
        unsigned int get_align_pos(const unsigned int &offset, const unsigned int &n)
        {
            unsigned int block_length = static_cast<unsigned int>(pow(2, n));
            if (offset % block_length == 0)
                return 0;
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
                        case '\"':
                            tmpstr[length++] = '\"';
                            break;
                        default:
                            std::cerr << obj << std::endl;
                            std::cerr << "string_modify in paser decode error" << std::endl;
                            break;
                    }
                }
                else
                    tmpstr[length++] = obj[i];
            }
            return tmpstr.substr(0, static_cast<unsigned long>(length));
        }

        //处理代码中的label
        void handle_code_label(std::string &label_arg, std::map<std::string, label_info> &text_label, std::map<std::string, unsigned int> &data_label, std::map<std::string, std::vector<int>> &unknown_label)
        {
            if (text_label[label_arg].start_line == 0)
            {
                unknown_label[label_arg].push_back(text_cnt); //本行代码的label未定义
            }
            else
            {
                tmpcommand.address.w_data_unsigned = text_label[label_arg].start_line; //本行代码的label已定义，将label地址的起始行号存入command
            }
        }

        void handle_global_text_label(std::string &label_arg, std::vector<command> &text_memory, std::map<std::string, label_info> &text_label, std::map<std::string, std::vector<int>> &unknown_label)
        {
            label_info tmp;
            tmp.start_line = text_cnt;
            text_label_flag = true;
            text_label[label_arg] = tmp;
            if (!unknown_label[label_arg].empty())
            {
                std::vector<int> *tmp = &unknown_label[label_arg];
                for (int i : *tmp)
                {
                    text_memory[i - 1].address.w_data_unsigned = text_cnt; //之前未定义address的label完善address
                }
            }
        }

        void handle_global_data_label(unsigned int &data_memory_pos, std::string &label_arg, std::map<std::string, unsigned int> &data_label, std::vector<command> &text_memory, std::map<std::string, std::vector<int>> &unknown_label)
        {
            data_label[label_arg] = data_memory_pos;
            if (!unknown_label[label_arg].empty())
            {
                std::vector<int> *tmp = &unknown_label[label_arg];
                for (int i : *tmp)
                {
                    text_memory[i - 1].address.w_data_address = data_memory_pos; //之前未定义address的label完善address
                }
            }
        }

        //获取地址和偏移量
        void cal_offset_address(int &offset, unsigned char &register_num, std::string &add_arg)
        {
            std::string off_set = add_arg.substr(0, add_arg.find_first_of('('));
            std::string add_tmp = add_arg.substr(add_arg.find_first_of('(') + 1, add_arg.find_first_of(')') - add_arg.find_first_of('(') - 1);
            offset = std::stoi(off_set);
            register_num = f_register[add_tmp];
        }

        //处理代码中的address
        void handle_code_address(std::string &add_arg, std::map<std::string, unsigned int> &data_label, std::map<std::string, std::vector<int>> &unknown_label)
        {
            if (add_arg[add_arg.size() - 1] == ')')
            {
                    int off_set = 0;
                    unsigned char register_num;
                    cal_offset_address(off_set, register_num, add_arg);
                    tmpcommand.offset.w_data_signed = off_set;
                    tmpcommand.rt = register_num;
            }
            else
            {
                if (data_label[add_arg] == -1)
                {
                    unknown_label[add_arg].push_back(text_cnt); //本行代码的label未定义
                }
                else
                {
                    tmpcommand.address.w_data_address = data_label[add_arg];
                }
            }
        }

        //内存分配函数
        void paser_align(char *data_memory_bottom, unsigned int&data_memory_pos)
        {
            unsigned int n = static_cast<unsigned int> (std::stoi(args[arg_num - 1]));
            unsigned int offset = get_align_pos(data_memory_pos, n);
            memset(data_memory_bottom + data_memory_pos, 0, offset);
            data_memory_pos += offset;
        }

        //所有对data区域进行的操作， 函数返回值均返回该数据的起始地址
        //字符串处理写入函数
        unsigned int paser_ascii(unsigned int &data_memory_pos)
        {
            std::string tmpstr = string_modify(args[arg_num - 1]);
            memcpy(data_memory_bottom + data_memory_pos, tmpstr.c_str(), tmpstr.size());
            unsigned int tmp = data_memory_pos;
            data_memory_pos += tmpstr.size();
            return tmp;
        }

        unsigned int paser_asciiz(unsigned int &data_memory_pos)
        {
            std::string tmpstr = string_modify(args[arg_num - 1]);
            size_t size = tmpstr.size();
            memcpy(data_memory_bottom + data_memory_pos, tmpstr.c_str(), size);
            (data_memory_bottom + data_memory_pos)[size] = '\0';
            unsigned int tmp = data_memory_pos;
            data_memory_pos += tmpstr.size() + 1;
            return tmp;
        }

        //byte half word写入函数
        unsigned int paser_byte(unsigned int &data_memory_pos)
        {
            byte *byte_arr = new byte[arg_num - 1];
            for (int i = 1; i < arg_num; i++) //注意0号是命令
            {
                byte_arr[i - 1].b_data = static_cast<char>(std::stoi(args[i]));
            }
            memcpy(data_memory_bottom + data_memory_pos, byte_arr, sizeof(byte) * (arg_num - 1));
            unsigned int tmp = data_memory_pos;
            data_memory_pos += sizeof(byte) * (arg_num - 1);
            return tmp;
        }

        unsigned int paser_half(unsigned int &data_memory_pos)
        {
            half *half_arr = new half[arg_num - 1];
            for (int i = 1; i < arg_num; i++)
            {
                half_arr[i - 1].h_data_signed = static_cast<short>(std::stoi(args[i]));
            }
            memcpy(data_memory_bottom + data_memory_pos, half_arr, sizeof(half) * (arg_num - 1));
            unsigned int tmp = data_memory_pos;
            data_memory_pos += sizeof(half) * (arg_num - 1);
            return tmp;
        }

        unsigned int paser_word(unsigned int &data_memory_pos)
        {
            word *word_arr = new word[arg_num - 1];
            for (int i = 1; i < arg_num; i++)
            {
                word_arr[i - 1].w_data_signed = std::stoi(args[i]);
            }
            memcpy(data_memory_bottom + data_memory_pos, word_arr, sizeof(word) * (arg_num - 1));
            unsigned int tmp = data_memory_pos;
            data_memory_pos += sizeof(word) * (arg_num - 1);
            delete [] word_arr;
            return tmp;
        }

        //处理空内存分配
        unsigned int paser_space(unsigned int &data_memory_pos)
        {
            unsigned int tmp = data_memory_pos;
            memset(data_memory_bottom + data_memory_pos, 0, static_cast<size_t>(std::stoi(args[arg_num - 1])));
            return tmp;
        }

        //指令写入代码区
        void write_command(std::vector<command> &text_memory)
        {
            text_memory.push_back(tmpcommand);
            text_cnt++;
        }

        //3参数1型指令(Rd Rt Rs/Imm)处理
        void paser_3_args_rs(bool unsigned_flag)
        {
            tmpcommand.OPT = f_command[args[0]];
            tmpcommand.rd = f_register[args[1]]; //rd为左值寄存器下标
            tmpcommand.rs = f_register[args[2]]; //rs为第一存器下标
            if (check_register(args[3]))
                tmpcommand.rt = f_register[args[3]]; //第三个是寄存器
            else if (!unsigned_flag) //立即数（有符号）
                tmpcommand.imm_num.w_data_signed = std::stoi(args[3]);
            else tmpcommand.imm_num.w_data_unsigned = static_cast<unsigned int>(std::stoul(args[3]));
        }

        //3参数2型指令(Rd Rs label)处理
        void paser_3_args_label(std::map<std::string, label_info> &text_label, std::map<std::string, unsigned int> &data_label, std::map<std::string, std::vector<int>> &unknown_label)
        {
            tmpcommand.OPT = f_command[args[0]];
            tmpcommand.rs = f_register[args[1]]; //rs为右一寄存器下标
            if (check_register(args[2]))
                tmpcommand.rt = f_register[args[2]]; //第二个参数是寄存器
            else tmpcommand.imm_num.w_data_signed = std::stoi(args[2]);
            handle_code_label(args[3], text_label, data_label, unknown_label);
        }

        //2参数1型指令(Rs Rt/Imm)处理
        void paser_2_args_rs(bool unsigned_flag)
        {
            tmpcommand.OPT = f_command[args[0]];
            tmpcommand.rs = f_register[args[1]]; //不使用结果寄存器， 便于区分mul/div 的两种情况
            if (check_register(args[2])) //第二个参数是寄存器
                tmpcommand.rt = f_register[args[2]];
            else if (!unsigned_flag) //立即数（有符号）
                tmpcommand.imm_num.w_data_signed = std::stoi(args[2]);
            else tmpcommand.imm_num.w_data_unsigned = static_cast<unsigned int>(std::stoul(args[2]));
        }

        //2参数2型指令(Rs label)处理
        void paser_2_args_label(std::map<std::string, label_info> &text_label, std::map<std::string, unsigned int> &data_label, std::map<std::string, std::vector<int>> &unknown_label)
        {
            tmpcommand.OPT = f_command[args[0]];
            tmpcommand.rs = f_register[args[1]]; //rs为左值寄存器下标
            handle_code_label(args[2], text_label, data_label, unknown_label);
        }

        //2参数3型指令(Rs address)处理
        void paser_2_args_address(std::map<std::string, unsigned int> &data_label, std::map<std::string, std::vector<int>> &unknown_label)
        {
            tmpcommand.OPT = f_command[args[0]];
            tmpcommand.rs = f_register[args[1]]; //rs为左值寄存器下标
                handle_code_address(args[2], data_label, unknown_label);
        }

        //单参数1型指令(Rs)处理
        void paser_1_args_rs()
        {
            tmpcommand.OPT = f_command[args[0]];
            tmpcommand.rs = f_register[args[1]];
        }

        //单参数2型指令(label)处理
        void paser_1_args_label(std::map<std::string, label_info> &text_label, std::map<std::string, unsigned int> &data_label, std::map<std::string, std::vector<int>> &unknown_label)
        {
            tmpcommand.OPT = f_command[args[0]];
            handle_code_label(args[1], text_label, data_label, unknown_label);
        }

        //无参数指令处理
        void paser_0_args()
        {
            tmpcommand.OPT = f_command[args[0]];
        }

    public:
        paser(char *data_memory)
        {
            load_f_command();
            load_f_register();
            inner_ptr = NULL;
            arg_num = 0;
            text_cnt = 1;
            args = new std::string[10];
            data_memory_bottom = data_memory;
        }
        ~paser()
        {
            delete [] args;
        }

        void encoder(std::string &current_line, std::vector<command> &text_memory, unsigned int &data_memory_pos, std::map<std::string, unsigned int> &data_label, std::map<std::string, label_info> &text_label, std::map<std::string, std::vector<int>> &unknown_label, char &dtflag)
        {
            split_line(current_line);
            if (arg_num == 0)
                return; //当前读到的为纯注释行，可以丢弃
            clear_commmand(tmpcommand); //初始化tmpcommand
            switch (f_command[args[0]])
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
                    paser_half(data_memory_pos);
                    break;
                case _word:
                    paser_word(data_memory_pos);
                    break;
                case _space:
                    paser_space(data_memory_pos);
                    break;
                case _data:
                    dtflag = 1;
                    break;
                case _text:
                    dtflag = 2;
                    break;
                case add_:
                case sub_:
                case xor_:
                case rem_:
                case seq_:
                case sge_:
                case sgt_:
                case sle_:
                case slt_:
                case sne_:
                    paser_3_args_rs(false);
                    write_command(text_memory);
                    break;
                case addu_:
                case addiu_:
                case subu_:
                case xoru_:
                case remu_:
                    paser_3_args_rs(true);
                    write_command(text_memory);
                    break;
                case mul_:
                case div_:
                    if (arg_num == 4)
                        paser_3_args_rs(false);
                    else paser_2_args_rs(false);
                    write_command(text_memory);
                    break;
                case mulu_:
                case divu_:
                    if (arg_num == 4)
                        paser_3_args_rs(true);
                    else paser_2_args_rs(true);
                    write_command(text_memory);
                    break;
                case beq_:
                case bne_:
                case bge_:
                case ble_:
                case bgt_:
                case blt_:
                    paser_3_args_label(text_label, data_label, unknown_label);
                    write_command(text_memory);
                    break;
                case move_:
                case li_:
                case neg_:
                    paser_2_args_rs(false);
                    write_command(text_memory);
                    break;
                case negu_:
                    paser_2_args_rs(true);
                    write_command(text_memory);
                    break;
                case beqz_:
                case bnez_:
                case blez_:
                case bgez_:
                case bgtz_:
                case bltz_:
                    paser_2_args_label(text_label, data_label, unknown_label);
                    write_command(text_memory);
                    break;
                case la_:
                case lb_:
                case lh_:
                case lw_:
                case sb_:
                case sh_:
                case sw_:
                    paser_2_args_address(data_label, unknown_label);
                    write_command(text_memory);
                    break;
                case jr_:
                case jalr_:
                case mfhi_:
                case mflo_:
                    paser_1_args_rs();
                    write_command(text_memory);
                    break;
                case b_:
                case j_:
                case jal_:
                    paser_1_args_label(text_label, data_label, unknown_label);
                    write_command(text_memory);
                    break;
                case nop_:
                case syscall_:
                    paser_0_args();
                    write_command(text_memory);
                    break;
                case label:
                    std::string tmp_arg = args[0].substr(0, args[0].size() - 1);
                    if (dtflag == 1)
                        handle_global_data_label(data_memory_pos, tmp_arg, data_label, text_memory, unknown_label);
                    else
                    {
                        handle_global_text_label(tmp_arg, text_memory, text_label, unknown_label);
                    }
                    tmplabel = tmp_arg;
                    break;
            }
        }
    };
};
#endif //MIPS_SIMULATOR_PASER_H
