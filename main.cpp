#include "scanner.h"
#include "memeory.h"
#include "register.h"

using namespace mips;
int main(int argc, char **argv)
{
    std::vector<mips::command> text_memory;
    std::map<std::string, char*> data_label;
    std::map<std::string, label_info> text_label;
    mips_memeory memory;
    mips_register fake_register;
    char *data_pos = memory.get_bottom();
    char *heap_pos;
    char *stack_pos;
    scanner mips_scanner("test.s");
    mips_scanner.scan_code(text_memory, memory.get_bottom(), data_pos, data_label, text_label);
    return 0;
}