#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>

constexpr auto PRINT_STATUS_EVERY = std::chrono::seconds(10);

constexpr uint8_t OPCODE_RETURN = 0x03;
constexpr uint8_t OPCODE_GOTO_IF = 0x06;
constexpr uint8_t OPCODE_LOAD_BYTE_FROM_PTR = 0x12;
constexpr uint8_t OPCODE_SET_PTR_BYTE = 0x13;
constexpr uint8_t OPCODE_SETVAR = 0x16;
constexpr uint8_t OPCODE_ADDVAR = 0x17;
constexpr uint8_t OPCODE_SUBVAR = 0x18;
constexpr uint8_t OPCODE_COPYVAR = 0x19;
constexpr uint8_t OPCODE_COMPARE_VAR_TO_VALUE = 0x21;

constexpr uint8_t CONDITION_NOT_EQUALS = 0x05;

constexpr uint32_t BASE_BYTECODE_ADDR = 0x0201835E;
constexpr uint32_t BASE_xVAR_OPCODES_ADDR = (0x020375DA - (0x8001 * 2));
constexpr uint32_t BASE_KEYBOARD_BUFFER_ADDR = 0x02021DC4;

constexpr char BYTECODE[] = "160180391B1200C41D020213007283010217018000001200DA7503021300998301021200DB75030213009A83010216018000001602804900170180000018028001002102800000060596830102170180DF181200C51D02021300BF83010217018000001200DA7503021300E68301021200DB7503021300E7830102160180000016028061001701800000180280010021028000000605E3830102170180EB131200C61D020213000C84010217018000001200DA7503021300338401021200DB75030213003484010216018000001602800D00170180000018028001002102800000060530840102170180EF111200C71D020213005984010217018000001200DA7503021300808401021200DB7503021300818401021601800000160280290017018000001802800100210280000006057D84010217018045111200C81D02021300A684010217018000001200DA7503021300CD8401021200DB7503021300CE840102160180000016028043001701800000180280010021028000000605CA840102170180DF121200C91D02021300F384010217018000001200DA75030213001A8501021200DB75030213001B85010216018000001602806500170180000018028001002102800000060517850102170180FD0D1200CA1D020213004085010217018000001200DA7503021300678501021200DB75030213006885010216018000001602805900170180000018028001002102800000060564850102170180AF131200CB1D020213008D85010217018000001200DA7503021300B48501021200DB7503021300B585010216018000001602808B001701800000180280010021028000000605B18501021701809F141200CC1D02021300DA85010217018000001200DA7503021300018601021200DB750302130002860102160180000016028047001701800000180280010021028000000605FE850102170180EF0F1200CD1D020213002786010217018000001200DA75030213004E8601021200DB75030213004F8601021601800000160280530017018000001802800100210280000006054B860102170180B50F190380018016018039051200C41D020213007E86010217018000001200DA7503021300A58601021200DB7503021300A686010216018000001602803B001701800000180280010021028000000605A2860102170180750E1200C51D02021300CB86010217018000001200DA7503021300F28601021200DB7503021300F38601021601800000160280B5001701800000180280010021028000000605EF860102170180FB111200C61D020213001887010217018000001200DA75030213003F8701021200DB75030213004087010216018000001602807F0017018000001802800100210280000006053C87010217018037121200C71D020213006587010217018000001200DA75030213008C8701021200DB75030213008D8701021601800000160280A3001701800000180280010021028000000605898701021701805F121200C81D02021300B287010217018000001200DA7503021300D98701021200DB7503021300DA870102160180000016028067001701800000180280010021028000000605D68701021701807B101200C91D02021300FF87010217018000001200DA7503021300268801021200DB7503021300278801021601800000160280A30017018000001802800100210280000006052388010217018051191200CA1D020213004C88010217018000001200DA7503021300738801021200DB75030213007488010216018000001602809500170180000018028001002102800000060570880102170180471B1200CB1D020213009988010217018000001200DA7503021300C08801021200DB7503021300C18801021601800000160280C1001701800000180280010021028000000605BD8801021701801F151200CC1D02021300E688010217018000001200DA75030213000D8901021200DB75030213000E8901021601800000160280D30017018000001802800100210280000006050A890102170180B1141200CD1D020213003389010217018000001200DA75030213005A8901021200DB75030213005B89010216018000001602809700170180000018028001002102800000060557890102170180EB13210380EFB006058D890102210180B9D406058D890102160D80010003160D80000003";
std::map<uint32_t, uint8_t> g_memory;

constexpr uint8_t ALLOWED_CHARS[] = {
        0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2,
        0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA,
        0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2,
        0xD3, 0xD4, // A -> Z
        0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC,
        0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
        0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC,
        0xED, 0xEE, // a -> z
        0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8,
        0xA9, 0xAA, // 0 -> 9
        0xAD, 0xB8, // . ,
        0xFF             // <empty char>
};

using KeyboardBuffer_t = std::array<uint8_t, 10>;

struct ExecutionContext_t {
    uint32_t pc;
    int8_t compare_result;
};

void load_bytecode_into_memory()
{
    for (size_t i = 0; BYTECODE[(i * 2)] != '\00'; ++i)
    {
        char byte[3];
        byte[0] = BYTECODE[i * 2];
        byte[1] = BYTECODE[(i * 2) + 1];
        byte[2] = 0x00;

        g_memory[BASE_BYTECODE_ADDR + i] = std::stoi(byte, nullptr, 16);
    }
}

void load_keyboard_buffer_into_memory(const KeyboardBuffer_t& keyboard_buffer)
{
    for (size_t i = 0; i < keyboard_buffer.size(); ++i)
    {
        g_memory[BASE_KEYBOARD_BUFFER_ADDR + i] = keyboard_buffer[i];
    }
}

void load_keyboard_indices(const KeyboardBuffer_t& keyboard_buffer_indices, KeyboardBuffer_t& keyboard_buffer)
{
    for (size_t i = 0; i < keyboard_buffer_indices.size(); ++i)
    {
        keyboard_buffer[i] = ALLOWED_CHARS[keyboard_buffer_indices[i]];
    }

    load_keyboard_buffer_into_memory(keyboard_buffer);
}

void print_keyboard_buffer(const KeyboardBuffer_t& keyboard_buffer)
{
    for (auto c: keyboard_buffer)
    {
        if (c >= 0xBB && c <= 0xD4)
        {
            std::cout << (char)((c - 0xBB) + 'A');
        } else if (c >= 0xD5 && c <= 0xEE)
        {
            std::cout << (char)((c - 0xD5) + 'a');
        } else if (c >= 0xA1 && c <= 0xAA)
        {
            std::cout << (char)((c - 0xA1) + '0');
        } else if (c == 0xAD)
        {
            std::cout << '.';
        } else if (c == 0xB8)
        {
            std::cout << ',';
        } else
        {
            std::cout << '/';
        }
    }
}

void ascii_to_keyboard_buffer(const std::string& in, KeyboardBuffer_t& keyboard_buffer)
{
    keyboard_buffer.fill(0xFF);

    for (size_t i = 0; i < std::min(in.size(), keyboard_buffer.size()); ++i)
    {
        char c = in[i];

        if (c >= 'A' && c <= 'Z')
        {
            keyboard_buffer[i] = ((c - 'A') + 0xBB);
        } else if (c >= 'a' && c <= 'z')
        {
            keyboard_buffer[i] = ((c - 'a') + 0xD5);
        } else if (c >= '0' && c <= '9')
        {
            keyboard_buffer[i] = ((c - '0') + 0xA1);
        } else if (c == '.')
        {
            keyboard_buffer[i] = 0xAD;
        } else if (c == ',')
        {
            keyboard_buffer[i] = 0xB8;
        } else
        {
            keyboard_buffer[i] = 0x00;
        }
    }
}

uint8_t fetch8(uint32_t& pc)
{
    try {
        return g_memory.at(pc++);
    } catch(const std::out_of_range& e)
    {
        std::cerr << "At fetch8: pc = 0x"
                  << std::hex << std::uppercase
                  << pc
                  << std::endl;

        throw;
    }
}

uint16_t fetch16(uint32_t& pc)
{
    uint8_t firstByte = fetch8(pc);
    uint8_t secondByte = fetch8(pc);

    return (firstByte | (secondByte << 8));
}

uint32_t fetch32(uint32_t& pc)
{
    uint16_t firstWord = fetch16(pc);
    uint16_t secondWord = fetch16(pc);

    return (firstWord | (secondWord << 16));
}

uint8_t peek8(uint32_t addr)
{
    try {
        return g_memory.at(addr);
    } catch(const std::out_of_range& e)
    {
        std::cerr << "At peek8: addr = 0x"
                  << std::hex << std::uppercase
                  << addr
                  << std::endl;

        throw;
    }
}

uint16_t peek16(uint32_t addr)
{
    uint8_t firstByte = peek8(addr);
    uint8_t secondByte = peek8(addr + 1);

    return (firstByte | (secondByte << 8));
}

uint32_t peek32(uint32_t addr)
{
    uint16_t firstWord = peek16(addr);
    uint16_t secondWord = peek16(addr + 2);

    return (firstWord | (secondWord << 16));
}

void opcode_goto_if(uint8_t condition, uint32_t destination, ExecutionContext_t& execution_context)
{
    switch (condition)
    {
    case CONDITION_NOT_EQUALS:
    {
        if (execution_context.compare_result != 0x00)
        {
            execution_context.pc = destination;
        }

        break;
    }

    default:
    {
        std::stringstream error_message;
        error_message << "Unknown condition 0x"
                      << std::hex << std::uppercase
                      << (unsigned)condition;

        throw std::runtime_error(error_message.str());
    }
    }
}

void opcode_load_byte_from_ptr(uint8_t destIndex, uint32_t source)
{
    try {
        g_memory[destIndex] = g_memory.at(source);
    } catch(const std::out_of_range& e)
    {
        std::cerr << "At opcode_load_byte_from_ptr: destIndex = 0x"
                  << std::hex << std::uppercase
                  << (unsigned)destIndex
                  << "; source = 0x"
                  << source
                  << std::endl;

        throw;
    }
}

void opcode_set_ptr_byte(uint8_t srcIndex, uint32_t destination)
{
    try {
        g_memory[destination] = g_memory.at(srcIndex);
    } catch(const std::out_of_range& e)
    {
        std::cerr << "At opcode_set_ptr_byte: srcIndex = 0x"
                  << std::hex << std::uppercase
                  << (unsigned)srcIndex
                  << "; destination = 0x"
                  << destination
                  << std::endl;

        throw;
    }
}

void opcode_setvar(uint16_t destination, uint16_t value)
{
    uint32_t address = (BASE_xVAR_OPCODES_ADDR + (destination * 2));

    g_memory[address] = (value & 0xFF);
    g_memory[address + 1] = ((value >> 8) & 0xFF);
}

void opcode_addvar(uint16_t destination, uint16_t value)
{
    uint32_t address = (BASE_xVAR_OPCODES_ADDR + (destination * 2));
    uint16_t oldValue;

    try {
        oldValue = peek16(address);
    } catch(const std::exception& e)
    {
        std::cerr << "At opcode_addvar" << std::endl;
        throw;
    }

    opcode_setvar(destination, oldValue + value);
}

void opcode_subvar(uint16_t destination, uint16_t value)
{
    uint32_t address = (BASE_xVAR_OPCODES_ADDR + (destination * 2));
    uint16_t oldValue;

    try {
        oldValue = peek16(address);
    } catch(const std::exception& e)
    {
        std::cerr << "At opcode_subvar" << std::endl;
        throw;
    }

    opcode_setvar(destination, oldValue - value);
}

void opcode_copyvar(uint16_t destination, uint16_t source)
{
    uint32_t address = (BASE_xVAR_OPCODES_ADDR + (source * 2));
    uint16_t value;

    try {
        value = peek16(address);
    } catch(const std::exception& e)
    {
        std::cerr << "At opcode_copyvar" << std::endl;
        throw;
    }

    opcode_setvar(destination, value);
}

int8_t opcode_compare_var_to_value(uint16_t var, uint16_t value)
{
    uint32_t address = (BASE_xVAR_OPCODES_ADDR + (var * 2));
    uint16_t var_val;

    try {
        var_val = peek16(address);
    } catch(const std::exception& e)
    {
        std::cerr << "At opcode_compare_var_to_value" << std::endl;
        throw;
    }

    if (var_val > value)
    {
        return 1;
    } else if (var_val < value)
    {
        return -1;
    }

    return 0;
}

void run_bytecode()
{
    // Setup the context
    bool running = true;
    ExecutionContext_t execution_context {
        BASE_BYTECODE_ADDR,
        0x00
    };

    // Run the bytecode
    while (running)
    {
        uint8_t opcode = fetch8(execution_context.pc);

        switch (opcode)
        {
        case OPCODE_RETURN:
        {
            running = false;
            break;
        }

        case OPCODE_GOTO_IF:
        {
            uint8_t condition = fetch8(execution_context.pc);
            uint32_t destination = fetch32(execution_context.pc);
            opcode_goto_if(condition, destination, execution_context);

            break;
        }

        case OPCODE_LOAD_BYTE_FROM_PTR:
        {
            uint8_t destIndex = fetch8(execution_context.pc);
            uint32_t source = fetch32(execution_context.pc);
            opcode_load_byte_from_ptr(destIndex, source);

            break;
        }

        case OPCODE_SET_PTR_BYTE:
        {
            uint8_t srcIndex = fetch8(execution_context.pc);
            uint32_t destination = fetch32(execution_context.pc);
            opcode_set_ptr_byte(srcIndex, destination);

            break;
        }

        case OPCODE_SETVAR:
        {
            uint16_t destination = fetch16(execution_context.pc);
            uint16_t value = fetch16(execution_context.pc);
            opcode_setvar(destination, value);

            break;
        }

        case OPCODE_ADDVAR:
        {
            uint16_t destination = fetch16(execution_context.pc);
            uint16_t value = fetch16(execution_context.pc);
            opcode_addvar(destination, value);

            break;
        }

        case OPCODE_SUBVAR:
        {
            uint16_t destination = fetch16(execution_context.pc);
            uint16_t value = fetch16(execution_context.pc);
            opcode_subvar(destination, value);

            break;
        }

        case OPCODE_COPYVAR:
        {
            uint16_t destination = fetch16(execution_context.pc);
            uint16_t source = fetch16(execution_context.pc);
            opcode_copyvar(destination, source);

            break;
        }

        case OPCODE_COMPARE_VAR_TO_VALUE:
        {
            uint16_t var = fetch16(execution_context.pc);
            uint16_t value = fetch16(execution_context.pc);
            execution_context.compare_result = opcode_compare_var_to_value(var, value);

            break;
        }

        default:
        {
            std::stringstream error_message;
            error_message << "Unknown opcode 0x"
                          << std::hex << std::uppercase
                          << (unsigned)opcode
                          << " at 0x"
                          << (execution_context.pc - 1);

            throw std::runtime_error(error_message.str());
        }
        }
    }
}

void run()
{
    // Setup everything
    auto last_status_print_time = std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration::zero());
    auto launch_time = std::chrono::steady_clock::now();
    size_t computed_hashes = 0;

    KeyboardBuffer_t keyboard_buffer{};
    KeyboardBuffer_t keyboard_buffer_indices{};
    keyboard_buffer_indices.fill(0);

    load_keyboard_indices(keyboard_buffer_indices, keyboard_buffer);
    load_bytecode_into_memory();

    // Run the bytecode until we find a solution
    while (true)
    {
        run_bytecode();
        ++computed_hashes;

        // Check if a solution was found
        if (peek16(0x020375F2) != 0x0000)
        {
            std::cout << "==== FOUND A SOLUTION!!! ====" << std::endl;

            std::cout << "8001h: 0x" << std::hex << std::uppercase << peek16(0x020375DA) << std::endl;
            std::cout << "8003h: 0x" << std::hex << std::uppercase << peek16(0x020375DE) << std::endl;
            std::cout << "800Dh: 0x" << std::hex << std::uppercase << peek16(0x020375F2) << std::endl;
            std::cout << std::nouppercase << std::dec;

            std::cout << "Keyboard buffer: ";
            print_keyboard_buffer(keyboard_buffer);
            std::cout << std::endl;

            std::cout << "Took "
                      << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - launch_time).count()
                      << "s" << std::endl;

            break;
        }

        // Print status every once in a while
        if (std::chrono::steady_clock::now() - last_status_print_time > PRINT_STATUS_EVERY)
        {
            std::cout << "Running for "
                      << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - launch_time).count()
                      << "s" << std::endl;

            auto status_print_freq_as_seconds = std::chrono::duration_cast<std::chrono::seconds>(PRINT_STATUS_EVERY);
            std::cout << "Rate: "
                      << (computed_hashes / status_print_freq_as_seconds.count())
                      << " hash/s" << std::endl;

            std::cout << "Keyboard buffer: ";
            print_keyboard_buffer(keyboard_buffer);
            std::cout << std::endl << std::endl;

            last_status_print_time = std::chrono::steady_clock::now();
            computed_hashes = 0;
        }

        // Next string
        size_t i = 0;

        while (i < keyboard_buffer.size())
        {
            ++keyboard_buffer_indices[i];

            if (keyboard_buffer_indices[i] == sizeof(ALLOWED_CHARS))
            {
                keyboard_buffer_indices[i] = 0;
            } else
            {
                break;
            }

            ++i;
        }

        if (i == keyboard_buffer.size())
        {
            // No solution was found :(
            std::cout << "No solution found :(" << std::endl;
            break;
        }

        load_keyboard_indices(keyboard_buffer_indices, keyboard_buffer);
    }
}

void run_interactive()
{
    // Setup everything
    KeyboardBuffer_t keyboard_buffer{};
    load_bytecode_into_memory();

    while (true)
    {
        // Get a string from the user
        std::string input;
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input.empty())
        {
            break;
        }

        // Convert it to a keyboard buffer
        ascii_to_keyboard_buffer(input, keyboard_buffer);

        // Run the bytecode
        load_keyboard_buffer_into_memory(keyboard_buffer);
        run_bytecode();

        // Dump the state
        std::cout << "8001h: 0x" << std::hex << std::uppercase << peek16(0x020375DA) << std::endl;
        std::cout << "8003h: 0x" << std::hex << std::uppercase << peek16(0x020375DE) << std::endl;
        std::cout << "800Dh: 0x" << std::hex << std::uppercase << peek16(0x020375F2) << std::endl;
        std::cout << std::nouppercase << std::dec;

        std::cout << "Keyboard buffer: ";
        print_keyboard_buffer(keyboard_buffer);
        std::cout << std::endl << std::endl;
    }
}

void run_diff()
{
    // Setup everything
    KeyboardBuffer_t keyboard_buffer{};
    keyboard_buffer.fill(ALLOWED_CHARS[0]);

    load_bytecode_into_memory();

    constexpr uint16_t base_first_val = 0xA4F1;
    constexpr uint16_t base_second_val = 0x33B1;

    for (size_t i = 0; i < keyboard_buffer.size(); ++i)
    {
        // Change the current char
        keyboard_buffer[i] = 1;

        // Run the bytecode
        load_keyboard_buffer_into_memory(keyboard_buffer);
        run_bytecode();

        // Dump the state
        uint16_t first_val = peek16(0x020375DA);
        uint16_t second_val = peek16(0x020375DE);

        std::cout << "8001h: 0x" << std::hex << std::uppercase << first_val << std::endl;
        std::cout << "8003h: 0x" << std::hex << std::uppercase << second_val << std::endl;
        std::cout << "800Dh: 0x" << std::hex << std::uppercase << peek16(0x020375F2) << std::endl;
        std::cout << std::nouppercase << std::dec;

        std::cout << "Keyboard buffer: ";
        print_keyboard_buffer(keyboard_buffer);
        std::cout << std::endl;

        // Print the differences
        std::cout << "8001h diff: 0x" << std::hex << std::uppercase << (uint16_t)(first_val - base_first_val) << std::endl;
        std::cout << "8003h diff: 0x" << std::hex << std::uppercase << (uint16_t)(second_val - base_second_val) << std::endl;
        std::cout << std::nouppercase << std::dec << std::endl;

        // Reset the current char
        keyboard_buffer[i] = 0;
    }
}

int main()
{
    try
    {
//        run();
//        run_interactive();
        run_diff();
    } catch(const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    return 0;
}
