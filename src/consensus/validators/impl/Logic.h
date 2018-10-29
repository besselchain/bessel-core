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

#ifndef BESSEL_VALIDATORS_LOGIC_H_INCLUDED
#define BESSEL_VALIDATORS_LOGIC_H_INCLUDED

#include <boost/unordered_map.hpp>
#include <beast/utility/Journal.h>
#include <boost/container/flat_set.hpp>
#include <memory>
#include <mutex>
#include <protocol/Protocol.h>
#include <common/base/hardened_hash.h>
#include <common/base/seconds_clock.h>
#include <protocol/BesselLedgerHash.h>
#include <consensus/validators/impl/Store.h>
#include <consensus/validators/impl/Tuning.h>


namespace bessel {
namespace Validators {

class ConnectionImp;

class Logic
{
public:
    using clock_type = beast::abstract_clock<std::chrono::steady_clock>;

private:
    struct LedgerMeta
    {
        std::uint32_t seq_no = 0;
        std::unordered_set<BesselAddress, hardened_hash<>> keys;
    };

    class Policy
    {
    public:
        /** Returns `true` if we should accept this as the last validated. */
        bool
        acceptLedgerMeta (std::pair<LedgerHash const, LedgerMeta> const& value)
        {
            return value.second.keys.size() >= 3; // quorum
        }

        bool
        acceptLedgerMeta (LedgerMeta const& value)
        {
            return value.keys.size() >= 3; // quorum
        }
    };

    std::mutex mutex_;
    //Store& store_;
    beast::Journal journal_;

    Policy policy_;
    
    
    class LedgerMap
    {
    public:
        LedgerMap(std::chrono::steady_clock::time_point t)
        {
            tp_ = t;
        }
        struct data
        {
            LedgerMeta meta_;
            std::chrono::steady_clock::time_point tp_;
        };

        std::chrono::steady_clock::time_point tp_;
        boost::unordered_map<LedgerHash, data, hardened_hash<>> map_;
    };
    void expire(std::chrono::seconds m)
    {
        for(auto it=ledgers_.map_.begin(); it != ledgers_.map_.end();)
        {
            auto tp = it->second.tp_;
            if(m <= ledgers_.tp_ - tp)
            {
                it = ledgers_.map_.erase(it);
            }
        }
    }
    
    LedgerMap ledgers_;
    std::pair<LedgerHash, LedgerMeta> latest_; // last fully validated
    boost::container::flat_set<ConnectionImp*> connections_;
    
    //boost::container::flat_set<

public:
    explicit
    Logic (Store& store, beast::Journal journal);

    beast::Journal const&
    journal() const
    {
        return journal_;
    }

    void stop();

    void load();

    void
    add (ConnectionImp& c);

    void
    remove (ConnectionImp& c);

    bool
    isStale (STValidation const& v);

    void
    onTimer();

    void
    onValidation (STValidation const& v);

    void
    onLedgerClosed (LedgerIndex index, LedgerHash const& hash, LedgerHash const& parent);
};

}
}

#endif