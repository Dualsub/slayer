#pragma once

#include <iostream>

namespace Slayer
{
    // Static functions for logging.
    class Log
    {
    public:
        template <typename ...Ts>
        static void Info(const Ts& ...args)
        {
            std::cout << "\033[34m[INFO] \033[0m";
            ((std::cout << args << ' '), ...);
            std::cout << std::endl;
        }

        template <typename ...Ts>
        static void Warn(const Ts& ...args)
        {
            std::cout << "\033[33m[WARN] \033[0m";
            ((std::cout << args << ' '), ...);
            std::cout << std::endl;
        }

        template <typename ...Ts>
        static void Error(const Ts& ...args)
        {
            std::cout << "\033[31m[ERROR] \033[0m";
            ((std::cout << args << ' '), ...);
            std::cout << std::endl;
        }

        template <typename ...Ts>
        static void Critical(const Ts& ...args)
        {
            std::cout << "\033[31m[CRITICAL] \033[0m";
            ((std::cout << args << ' '), ...);
            std::cout << std::endl;
        }
    };
}