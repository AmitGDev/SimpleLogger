// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <memory>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>

// It is important to arrange that the logger is destructed LAST (static objects may log during exit-process).
// In C++, the order of destruction for static objects is the reverse of their order of construction. However,
// the order of construction (and thus destruction) of static objects defined in different translation units
// (i.e., different .cpp files) is not specified by the C++ standard, leading to what is known as the “static
// initialization order fiasco”.
#include "SimpleLogger.h"


// (localtime_s on Win32 and localtime_r on Linux)
#ifndef _WIN32
#define localtime_s(x, y) localtime_r(y, x)
#endif


// Date & Time prefix (Example)
static std::wstring DateTimePrefix() 
{
    const auto now{ std::chrono::system_clock::now() };
    const auto in_time_t{ std::chrono::system_clock::to_time_t(now) };

    std::tm time_info{};
    localtime_s(&time_info, &in_time_t);

    std::wstringstream wstring_stream{};
    wstring_stream << std::put_time(&time_info, L"%d-%m-%Y %X ");

    return wstring_stream.str();
}


int main()
{
    try
    {
        auto out_stream{ std::make_unique<std::wofstream>("Log.txt", std::ios::app) };

        if ((*out_stream.get()).fail()) {
            throw std::runtime_error("failed to open file");
        }

        SimpleLogger::SetOstream(std::move(out_stream));

        std::vector<SimpleLogger::PrefixFunction> prefix_list{ DateTimePrefix };
        SimpleLogger::SetPrefixList(prefix_list);

    } catch (const std::exception& e) {
        std::cerr << "caught exception: " << e.what() << std::endl;
    }

    LOG(DEBUG) << L"Line " << 1;
    LOG(INFO) << L"Line " << 2;
    LOG(WARNING) << L"Pi = " << 3.14159265359;
    LOG(ERROR) << L"Divide by zero";
    LOG(CRITICAL) << L"Line " << L"End";
}