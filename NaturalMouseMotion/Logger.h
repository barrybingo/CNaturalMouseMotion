#pragma once

#include <memory>
#include <string>
#include <cstdarg>
#include <list>
#include <functional>

namespace NaturalMouseMotion
{

using LoggerPrinterFunc = std::function<void(const std::string)>;

struct Logger
{
    static void Print(LoggerPrinterFunc printer, const std::string fmt, ...)
    {
        if (!printer)
            return;

        std::string formated;

        char buf[256];

        va_list args;
        va_start(args, fmt);
        const auto r = std::vsnprintf(buf, sizeof buf, fmt.c_str(), args);
        va_end(args);

        if (r < 0)
            // conversion failed
            return;

        const size_t len = r;
        if (len < sizeof buf)
        {
            formated = buf;
        }
        else
        {
    #if __cplusplus >= 201703L
            // C++17: Create a string and write to its underlying array
            std::string s(len, '\0');
            va_start(args, fmt);
            std::vsnprintf(s.data(), len + 1, fmt, args);
            va_end(args);

            formated = s;
    #else
            // C++11 or C++14: We need to allocate scratch memory
            auto vbuf = std::unique_ptr<char[]>(new char[len + 1]);
            va_start(args, fmt);
            std::vsnprintf(vbuf.get(), len + 1, fmt.c_str(), args);
            va_end(args);

            formated = vbuf.get();
    #endif
        }

        printer(formated);
    }
};
} // namespace NaturalMouseMotion