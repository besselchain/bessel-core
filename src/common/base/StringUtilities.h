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

#ifndef BESSEL_BASICS_STRINGUTILITIES_H_INCLUDED
#define BESSEL_BASICS_STRINGUTILITIES_H_INCLUDED

#include <sstream>
#include <string>
#include <boost/format.hpp>

#include <common/base/ByteOrder.h>
#include <common/base/Blob.h>
#include <common/base/strHex.h>

namespace bessel {

// NIKB TODO Remove the need for all these overloads. Move them out of here.
inline const std::string strHex (std::string const& strSrc)
{
    return strHex (strSrc.begin (), strSrc.size ());
}

inline std::string strHex (Blob const& vucData)
{
    return strHex (vucData.begin (), vucData.size ());
}

inline std::string strHex (const std::uint64_t uiHost)
{
    uint64_t    uBig    = htobe64 (uiHost);

    return strHex ((unsigned char*) &uBig, sizeof (uBig));
}

inline static std::string sqlEscape (std::string const& strSrc)
{
    static boost::format f ("X'%s'");
    return str (boost::format (f) % strHex (strSrc));
}

inline static std::string sqlEscape (Blob const& vecSrc)
{
    size_t size = vecSrc.size ();

    if (size == 0)
        return "X''";

    std::string j (size * 2 + 3, 0);

    unsigned char* oPtr = reinterpret_cast<unsigned char*> (&*j.begin ());
    const unsigned char* iPtr = &vecSrc[0];

    *oPtr++ = 'X';
    *oPtr++ = '\'';

    for (int i = size; i != 0; --i)
    {
        unsigned char c = *iPtr++;
        *oPtr++ = charHex (c >> 4);
        *oPtr++ = charHex (c & 15);
    }

    *oPtr++ = '\'';
    return j;
}

int strUnHex (std::string& strDst, std::string const& strSrc);

uint64_t uintFromHex (std::string const& strSrc);

std::pair<Blob, bool> strUnHex (std::string const& strSrc);

Blob strCopy (std::string const& strSrc);
std::string strCopy (Blob const& vucSrc);

bool parseIpPort (std::string const& strSource, std::string& strIP, int& iPort);

bool parseUrl (std::string const& strUrl, std::string& strScheme,
               std::string& strDomain, int& iPort, std::string& strPath);

} // bessel

#endif
