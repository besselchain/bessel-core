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

#ifndef BESSELD_BESSEL_RPC_VERSIONS_H
#define BESSELD_BESSEL_RPC_VERSIONS_H

#include <protocol/JsonFields.h>
#include <common/json/Object.h>
#include <common/misc/SemanticVersion.h>

namespace bessel {
namespace RPC {

extern bessel::SemanticVersion const firstVersion;
extern bessel::SemanticVersion const goodVersion;
extern bessel::SemanticVersion const lastVersion;

template <class Object>
void setVersion(Object& parent)
{
    auto&& object = addObject (parent, jss::version);
    object[jss::first] = firstVersion.print();
    object[jss::good] = goodVersion.print();
    object[jss::last] = lastVersion.print();
}

} // RPC
} // bessel

#endif
