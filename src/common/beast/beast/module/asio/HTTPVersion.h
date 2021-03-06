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

#ifndef BEAST_MODULE_ASIO_HTTPVERSION_H_INCLUDED
#define BEAST_MODULE_ASIO_HTTPVERSION_H_INCLUDED

#include <beast/strings/String.h>

namespace beast {

/** The HTTP version. This is the major.minor version number. */
class HTTPVersion
{
public:
    HTTPVersion ();
    HTTPVersion (unsigned short major_, unsigned short minor_);
    HTTPVersion (HTTPVersion const& other);
    HTTPVersion& operator= (HTTPVersion const& other);
    String toString () const;
    unsigned short vmajor () const;
    unsigned short vminor () const;
    bool operator== (HTTPVersion const& rhs) const;
    bool operator!= (HTTPVersion const& rhs) const;
    bool operator>  (HTTPVersion const& rhs) const;
    bool operator>= (HTTPVersion const& rhs) const;
    bool operator<  (HTTPVersion const& rhs) const;
    bool operator<= (HTTPVersion const& rhs) const;

private:
    unsigned short m_major;
    unsigned short m_minor;
};

}

#endif
