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

#ifndef BESSEL_MODULE_CORE_DIAGNOSTIC_FATALERROR_H_INCLUDED
#define BESSEL_MODULE_CORE_DIAGNOSTIC_FATALERROR_H_INCLUDED

namespace bessel
{

/** Signal a fatal error.

    A fatal error indicates that the program has encountered an unexpected
    situation and cannot continue safely. Reasons for raising a fatal error
    would be to protect data integrity, prevent valuable resources from being
    wasted, or to ensure that the user does not experience undefined behavior.

    If multiple threads raise an error, only one will succeed while the others
    will be blocked before the process terminates.
*/
void
FatalError (char const* message, char const* file = nullptr, int line = 0);

} // bessel

#endif