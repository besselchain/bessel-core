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

#ifndef BESSEL_APP_PATHS_BESSELLINECACHE_H_INCLUDED
#define BESSEL_APP_PATHS_BESSELLINECACHE_H_INCLUDED

#include <transaction/paths/BesselState.h>
#include <common/base/hardened_hash.h>
#include <cstddef>
#include <memory>
#include <vector>

namespace bessel {

// Used by Pathfinder
class BesselLineCache
{
public:
    typedef std::vector <BesselState::pointer> BesselStateVector;
    typedef std::shared_ptr <BesselLineCache> pointer;
    typedef pointer const& ref;

    explicit BesselLineCache (Ledger::ref l);

    Ledger::ref getLedger () //  TODO const?
    {
        return mLedger;
    }

    std::vector<BesselState::pointer> const&
    getBesselLines (Account const& accountID);

private:
    typedef BesselMutex LockType;
    typedef std::lock_guard <LockType> ScopedLockType;
    LockType mLock;

    bessel::hardened_hash<> hasher_;
    Ledger::pointer mLedger;

    struct AccountKey
    {
        Account account_;
        std::size_t hash_value_;

        AccountKey (Account const& account, std::size_t hash)
            : account_ (account)
            , hash_value_ (hash)
        { }

        AccountKey (AccountKey const& other) = default;

        AccountKey&
        operator=(AccountKey const& other) = default;

        bool operator== (AccountKey const& lhs) const
        {
            return hash_value_ == lhs.hash_value_ && account_ == lhs.account_;
        }

        std::size_t
        get_hash () const
        {
            return hash_value_;
        }

        struct Hash
        {
            std::size_t
            operator () (AccountKey const& key) const noexcept
            {
                return key.get_hash ();
            }
        };
    };

    hash_map <AccountKey, BesselStateVector, AccountKey::Hash> mRLMap;
};

} // bessel

#endif
