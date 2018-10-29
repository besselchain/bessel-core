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

#ifndef BESSEL_PEERFINDER_STORE_H_INCLUDED
#define BESSEL_PEERFINDER_STORE_H_INCLUDED

#include <boost/asio.hpp>

namespace bessel {
namespace PeerFinder {

/** Abstract persistence for PeerFinder data. */
class Store
{
public:
    virtual ~Store () { }

    // load the bootstrap cache
    typedef std::function <void(boost::asio::ip::tcp::endpoint, int)> load_callback;
    virtual std::size_t load(load_callback const& cb) = 0;

    // save the bootstrap cache
    struct Entry
    {
        boost::asio::ip::tcp::endpoint endpoint;
        int valence;
    };

    virtual void save (std::vector<Entry> const& v) = 0;
};

}
}

#endif
