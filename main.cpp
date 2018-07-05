#include "scanner.h"
#include "memeory.h"
#include "register.h"
#include "decoder.h"

using namespace mips;
int main(int argc, char **argv)
{
    std::vector<mips::command> text_memory;
    std::map<std::string, char*> data_label;
    std::map<std::string, label_info> text_label;
    mips_memeory memory;
    mips_register fake_register;
    char *heap_pos = memory.get_bottom();
    char *stack_pos = memory.get_bottom() + 6 * 1024 * 1024;
    scanner mips_scanner("test.s");
    mips_scanner.scan_code(text_memory, memory.get_bottom(), heap_pos, data_label, text_label);
    fake_register.set_stack(stack_pos);
    decoder mips_decoder(text_label["main"].start_line);
    mips_decoder.decode_command(text_memory, memory.get_bottom(), heap_pos, fake_register.get_register());
    return 0;
}