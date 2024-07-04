#ifndef AMITG_FC_SIMPLELOGGER
#define AMITG_FC_SIMPLELOGGER

/*
  SimpleLogger.h
  Copyright (c) 2024, Amit Gefen

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <functional>
#include <shared_mutex>
#include <syncstream>
#include <iostream>
#include <array>


// Macros for logging__

#define LOG(severity) SimpleLogger(severity)

// Severities:
#define DEBUG SimpleLogger::Severity::kDebug
#define INFO SimpleLogger::Severity::kInfo
#define WARNING SimpleLogger::Severity::kWarning
#define ERROR SimpleLogger::Severity::kError
#define CRITICAL SimpleLogger::Severity::kCritical

// __Macros for logging


// SimpleLogger class provides a simple logging utility for C++20 programs. 
// It allows logging messages to an output stream with optional prefixes.
// SimpleLogger class supports synchronized output and customization of the
// output stream and prefix functions.

class SimpleLogger
{
public:

    // Prefix function signature
    using PrefixFunction = std::function<std::wstring()>;

    enum class Severity : uint8_t { kDebug, kInfo, kWarning, kError, kCritical }; // Bounds: 0-4.
    const std::array<std::wstring, 5> severity_map{ L"DEBUG", L"INFO", L"WARNING", L"ERROR", L"CRITICAL" };


    // Constructor
    SimpleLogger(Severity severity = Severity::kDebug, bool newline = true) : newline_(newline)
    {
        std::shared_lock lock(mutex_);

        if (out_stream_valid_) {
            // In the context of a logging utility, it’s generally not a good idea to throw exceptions because it could lead
            // to the termination of the entire process if not caught. Logging should be a non-intrusive operation and should  
            // not affect the normal flow of the program.
            try {
                // May throw std::bad_alloc or any exception thrown by the constructor of T. If an exception is thrown, make_unique has no effect.
                out_sync_stream_ = std::make_unique<std::wosyncstream>(*out_stream_.get());
                out_sync_stream_valid_ = true;

                for (const auto& prefix : prefix_function_list_) {
                    *out_sync_stream_.get() << prefix();
                }

                *out_sync_stream_.get() << severity_map.at(static_cast<size_t>(severity)) << L": ";

            } catch (const std::exception& e) {
                std::cerr << "caught exception: " << e.what() << std::endl;
            }
        }
    }


    // Destructor
    virtual ~SimpleLogger() // (Destructors are implicitly declared with noexcept)
    {
        // The destruction of out_sync_stream_ involves releasing the ownership of the managed
        // std::wosyncstream object, which indirectly accesses the associated output stream (out_stream_).
        std::shared_lock lock(mutex_);

        if (newline_ && out_sync_stream_valid_) { // (Short-circuit evaluation; Evaluates operands from left to right.)
            *out_sync_stream_.get() << std::endl;
        }

        // Calling reset() on out_sync_stream_ while holding the mutex locked, releases ownership 
        // of the std::wosyncstream object, ensuring exclusive access and preventing simultaneous 
        // modifications to the associated output stream (out_stream_) by other threads.
        out_sync_stream_.reset();
    }


    SimpleLogger(const SimpleLogger&) = delete; // Copy constructor.
    SimpleLogger& operator=(const SimpleLogger&) = delete; // Copy assignment operator.
    SimpleLogger(SimpleLogger&&) = delete; // Move constructor.
    SimpleLogger& operator=(SimpleLogger&&) = delete; // Move assignment operator.


    // Operator <<
    // On use of the operator<< function with an argument of a certain type, T is replaced with that type.
    template <typename T>
    SimpleLogger& operator<<(const T& value) noexcept
    {
        if (out_sync_stream_valid_) {
            *out_sync_stream_.get() << value;
        }

        return *this; // Allow chaining for convenience (optional).
    }


    // Setters__

    // Set the out stream
    static void SetOstream(std::unique_ptr<std::wostream> out_stream) noexcept
    {
        std::lock_guard lock(mutex_);

        out_stream_ = std::move(out_stream);
        out_stream_valid_ = out_stream_.get() != nullptr && (*out_stream_.get()).good(); // (Short-circuit evaluation; Evaluates operands from left to right.)
    }


    // Set the prefix list
    static void SetPrefixList(const std::vector<PrefixFunction>& prefix_list) noexcept
    {
        std::lock_guard lock(mutex_);

        prefix_function_list_ = prefix_list; // (Copy. Take ownership)
    }

    // __Setters

private:

    bool newline_{ true };

    // Synchronized Output Stream:
    // (Provides a mechanism to synchronize threads writing to the same stream.)
    std::unique_ptr<std::wosyncstream> out_sync_stream_{ nullptr };
    bool out_sync_stream_valid_{ false };

    // Statics__

    // Shared among all instances of the SimpleLogger class across the entire process.
    // Even if there are no instances of the class SimpleLogger in existence, the static member  
    // variables will still exist and retain their values until the program terminates.

    // The prefix functions (pointers):
    inline static std::vector<PrefixFunction> prefix_function_list_{};

    // Output Stream:
    inline static std::unique_ptr<std::wostream> out_stream_{ nullptr };
    inline static bool out_stream_valid_{ false };

    // Synchronizes access to all static member variables:
    // Allows multiple threads to concurrently read shared resources
    // while preventing concurrent writes or read and write operations.
    // Assumes predominantly read operations, predicting negligible
    // impact on performance.
    inline static std::shared_mutex mutex_{};

    // __Statics
};

#endif