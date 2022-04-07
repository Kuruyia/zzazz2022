#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>

constexpr auto PRINT_STATUS_EVERY = std::chrono::seconds(10);

constexpr uint16_t FIRST_DIFF[] = {0x107F, 0xD08D, 0x6F79, 0x9407,
                                   0x414D, 0x502B, 0x82D9, 0xD435,
                                   0x7C75, 0x0097};

constexpr uint16_t SECOND_DIFF[] = {0x9A0D, 0x13A5, 0x69C5, 0x6A99,
                                    0xA4F1, 0x7CBB, 0x669F, 0x7FB7,
                                    0x1705, 0x0053};

constexpr uint16_t FIRST_BASE_VALUE = 0xA4F1;
constexpr uint16_t SECOND_BASE_VALUE = 0x33B1;

constexpr uint8_t ALLOWED_MULTIPLICATORS[] = {
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

using MultiplicatorArray_t = std::array<unsigned , 10>;

void load_multiplicator_indices(const MultiplicatorArray_t& multiplicators_indices, MultiplicatorArray_t& multiplicators)
{
    for (size_t i = 0; i < multiplicators_indices.size(); ++i)
    {
        multiplicators[i] = ALLOWED_MULTIPLICATORS[multiplicators_indices[i]];
    }
}

void print_keyboard_buffer(const MultiplicatorArray_t & keyboard_buffer)
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

void run()
{
    // Setup everything
    auto last_status_print_time = std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration::zero());
    auto launch_time = std::chrono::steady_clock::now();
    size_t computed_hashes = 0;

    MultiplicatorArray_t multiplicators{};
    MultiplicatorArray_t multiplicators_indices{};
    multiplicators_indices.fill(0);
    load_multiplicator_indices(multiplicators_indices, multiplicators);

    // Run everything until we get a solution
    while (true)
    {
        // Compute the final values
        uint16_t first_value = FIRST_BASE_VALUE;
        uint16_t second_value = SECOND_BASE_VALUE;

        for (size_t i = 0; i < multiplicators.size(); ++i)
        {
            first_value += (FIRST_DIFF[i] * multiplicators[i]);
        }

        for (size_t i = 0; i < multiplicators.size(); ++i)
        {
            second_value += (SECOND_DIFF[i] * multiplicators[i]);
        }

        ++computed_hashes;

        // Check if a solution was found
        if (first_value == 0xD4B9 && second_value == 0xB0EF)
        {
            std::cout << "==== FOUND A SOLUTION!!! ====" << std::endl;

            std::cout << "8001h: 0x" << std::hex << std::uppercase << first_value << std::endl;
            std::cout << "8003h: 0x" << std::hex << std::uppercase << second_value << std::endl;
            std::cout << std::nouppercase << std::dec;

            std::cout << "Text: ";
            print_keyboard_buffer(multiplicators);
            std::cout << std::endl << std::endl;

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

            std::cout << "Text: ";
            print_keyboard_buffer(multiplicators);
            std::cout << std::endl << std::endl;

            last_status_print_time = std::chrono::steady_clock::now();
            computed_hashes = 0;
        }

        // Next multiplicator
        size_t i = 0;

        while (i < multiplicators_indices.size())
        {
            ++multiplicators_indices[i];

            if (multiplicators_indices[i] == sizeof(ALLOWED_MULTIPLICATORS))
            {
                multiplicators_indices[i] = 0;
            } else
            {
                break;
            }

            ++i;
        }

        if (i == multiplicators_indices.size())
        {
            // No solution was found :(
            std::cout << "No solution found :(" << std::endl;
            break;
        }

        load_multiplicator_indices(multiplicators_indices, multiplicators);
    }
}

int main()
{
    run();

    return 0;
}
