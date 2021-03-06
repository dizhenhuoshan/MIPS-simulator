#include "scanner.h"
#include "memeory.h"
#include "register.h"
#include "pipeline.h"
#include <iostream>

using namespace mips;
int main(int argc, char **argv)
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    std::cout.tie(0);
    std::vector<mips::command> text_memory;
    std::map<std::string, unsigned int> data_label;
    std::map<std::string, label_info> text_label;
    mips_memeory memory;
    mips_register fake_register;
    unsigned int heap_pos = 0;
    unsigned int stack_pos = 6 * 1024 * 1024;
    scanner mips_scanner(argv[1], memory.get_bottom());
    mips_scanner.scan_code(text_memory, memory.get_bottom(), heap_pos, data_label, text_label);
    fake_register.set_stack(stack_pos);
    decoder mips_decoder(text_label["main"].start_line, fake_register.get_register());
    mips_decoder.pipeline(heap_pos, memory.get_bottom(), text_memory);
    return 0;
}
