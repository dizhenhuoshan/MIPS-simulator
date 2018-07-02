//
// Scanner for the mips-scanner
//

#ifndef MIPS_SIMULATOR_SCANNER_H
#define MIPS_SIMULATOR_SCANNER_H

#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include "constant.h"
#include "codeline.h"

namespace mips
{
    class scanner
    {
    private:
        int line_cnt; //当前代码储存器中的行号
        std::string current_line; //当前行读到的的代码
        std::map<int, std::string> unknown_lable; //储存当前未定义的lable
        
        void get_line() // 从当前的源代码中拿出一行
        {
        
        }
        
        void check_vaild() // 判断当前拿出的代码是否有效(排除注释等无效代码)
        {
        
        }
    public:
        scanner()
        {
            line_cnt = 0;
        }
        
        ~scanner() = default;
        
        // 扫描处理源代码，并将它里面的data和text存入相关的位置
        bool scan_code(std::vector<mips::command> &text_memery, char *date_memory, const std::string &source_code)
        {}
    };
    
}
#endif //MIPS_SIMULATOR_SCANNER_H
