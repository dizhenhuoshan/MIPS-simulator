//
// Scanner for the mips-scanner
//

#ifndef MIPS_SIMULATOR_SCANNER_H
#define MIPS_SIMULATOR_SCANNER_H

#include <fstream>
#include <sstream>
#include "paser.h"
#include "constant.h"
#include "codeline.h"

namespace mips
{
    class scanner
    {
    private:
        std::fstream file; //读入的代码文件的名字
        std::ostringstream oss; //将读入的代码文件存入字符串流
        char *source_code; //整体读入的源代码
        char *outer_ptr; //用于辅助strtok
        int line_cnt; //当前代码储存器中的行号
        std::string current_line; //当前行读到的的代码
        std::map<int, std::string> unknown_lable; //储存当前未定义的lable
        char dtflag; //数据区 or 代码区标记 1-数据区 2-代码区
        paser mips_paser;
        
        bool get_line() // 从当前的源代码中拿出一行
        {
            char *current_code = strtok_r(source_code, "\n", &outer_ptr);
            if (current_code != NULL)
            {
                line_cnt++;
                current_line = current_code;
                return true;
            }
            else return false;
        }
        
    public:
        scanner(const char* file_name):file(file_name)
        {
            oss << file.rdbuf();
            std::string tmpstr = oss.str();
            source_code = new char[tmpstr.size() + 10];
            strcmp(source_code, tmpstr.c_str());
            line_cnt = 0;
            outer_ptr = NULL;
            dtflag = 1;
        }
        
        ~scanner()
        {
            delete [] source_code;
            delete outer_ptr;
        }
        
        // 扫描处理源代码，并将它里面的data和text存入相关的位置
        bool scan_code(std::vector<mips::command> &text_memory, char *data_memory_bottom, char *data_memory_pos)
        {
            while (get_line())
            {
                mips_paser.paser_code(line_cnt, current_line, text_memory, data_memory_bottom, data_memory_pos, unknown_lable, dtflag);
            }
        }
    };
    
}
#endif //MIPS_SIMULATOR_SCANNER_H
