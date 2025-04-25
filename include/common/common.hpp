#pragma once
#include <string>
#include <iostream>

namespace common
{

    inline void assert(bool cond, const std::string& msg) noexcept
    {
        if (!cond) [[unlikely]]
        {
            std::cerr << msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline void fatal(const std::string& msg) noexcept
    {
        std::cerr << msg << std::endl;
        exit(EXIT_FAILURE);
    } 

} // namespace common
