//------------------------------------------------------------------------------
//*
    This file is part of Bessel Chain Project: https://github.com/Besselfoundation/bessel-core
    Copyright (c) 2018 BESSEL.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#include <common/misc/FatalError.h>
#include <atomic>
#include <exception>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <iterator> //std::begin std::end std::distance
#include <execinfo.h> //backtrace
#include <memory> //std::unique_ptr

namespace bessel {

std::vector<std::string>
getStackBacktrace()
{
    std::vector<std::string> result;

    void* stack[128];
    int frames = backtrace(stack, std::distance(std::begin(stack), std::end(stack)));
    std::unique_ptr<char*[], void(*)(void*)> frame(backtrace_symbols(stack, frames), std::free);

    for(int i=0; i<frames; ++i)
        result.push_back(frame[i]);

    return result;
}

//------------------------------------------------------------------------------
void
FatalError (char const* message, char const* file, int line)
{
    static std::atomic <int> error_count (0);
    static std::recursive_mutex gate;

    // We only allow one thread to report a fatal error. Other threads that
    // encounter fatal errors while we are reporting get blocked here.
    std::lock_guard<std::recursive_mutex> lock(gate);

    // If we encounter a recursive fatal error, then we want to terminate
    // unconditionally.
    if (error_count++ != 0)
        return std::terminate ();

    // We protect this entire block of code since writing to cerr might trigger
    // exceptions.
    try
    {
        std::cerr << "An error has occurred. The application will terminate.\n";

        if (message != nullptr && message [0] != 0)
            std::cerr << "Message: " << message << '\n';

        if (file != nullptr && file [0] != 0)
            std::cerr << "   File: " << file << ":" << line << '\n';

        auto const backtrace = bessel::getStackBacktrace ();

        if (!backtrace.empty ())
        {
            std::cerr << "  Stack:" << std::endl;

            for (auto const& frame : backtrace)
                std::cerr << "    " << frame << '\n';
        }
    }
    catch (...)
    {
        // nothing we can do - just fall through and terminate
    }

    return std::terminate ();
}

} // bessel