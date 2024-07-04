**SimpleLogger v1.0.0**

**Author:** Amit Gefen

**License:** MIT License

<br>

**Overview**

A Cross-Platform Simple Logger for C++20 applications (Header-Only Class), offering synchronized output and customizable prefixing. It allows logging messages to an output stream with optional prefixes, supporting various data types.

<br>

**Features**

- Logging utility for C++20 programs.
- Synchronized output for thread-safe logging.
- Customizable output stream and prefix functions.
- Supports logging messages of various data types.
- Exception handling for non-intrusive logging.
- Macro LOG for convenient logging.
- Dynamic setting of output stream and prefix list.
- Supports chaining of log messages.

<br>

**Example Usage**

```cpp
#include <memory>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
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
    }
    catch (const std::exception& e)
    {
        std::osyncstream sync_cerr(std::cerr);
        sync_cerr << "caught exception: " << e.what() << std::endl;
    }

    LOG(DEBUG) << L"Line " << 1;
    LOG(INFO) << L"Line " << 2;
    LOG(WARNING) << L"Pi = " << 3.14159265359;
    LOG(ERROR) << L"Divide by zero";
    LOG(CRITICAL) << L"Line " << L"End";
}
```