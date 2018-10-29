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

#include <BeastConfig.h>
#include <protocol/STAccount.h>

namespace bessel {

STAccount::STAccount (SerialIter& sit, SField const& name)
    : STAccount(name, sit.getVLBuffer())
{
}

std::string STAccount::getText () const
{
    Account u;
    BesselAddress a;

    if (!getValueH160 (u))
        return STBlob::getText ();

    a.setAccountID (u);
    return a.humanAccountID ();
}

STAccount*
STAccount::construct (SerialIter& u, SField const& name)
{
    return new STAccount (name, u.getVLBuffer ());
}

STAccount::STAccount (SField const& n, Account const& v)
        : STBlob (n, v.data (), v.size ())
{
}

bool STAccount::isValueH160 () const
{
    return peekValue ().size () == (160 / 8);
}

BesselAddress STAccount::getValueNCA () const
{
    BesselAddress a;
    Account account;

    if (getValueH160 (account))
        a.setAccountID (account);

    return a;
}

void STAccount::setValueNCA (BesselAddress const& nca)
{
    setValueH160 (nca.getAccountID ());
}

} // bessel
