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
#include <network/peerfinder/impl/SourceStrings.h>
#include <boost/asio.hpp>
#include <common/misc/Utility.h>

namespace bessel {
namespace PeerFinder {

class SourceStringsImp : public SourceStrings
{
public:
    SourceStringsImp (std::string const& name, Strings const& strings)
        : m_name (name)
        , m_strings (strings)
    {
    }

    ~SourceStringsImp ()
    {
    }

    std::string const& name ()
    {
        return m_name;
    }

    void fetch (Results& results, beast::Journal journal)
    {
        results.addresses.resize (0);
        results.addresses.reserve (m_strings.size());

        for (int i = 0; i < m_strings.size (); ++i)
        {
            boost::asio::ip::tcp::endpoint ep (endpoint_from_string (m_strings [i]).first);
            if (ep.address().is_unspecified ())
                std::runtime_error("IP address is unspecified");

            if (! ep.address().is_unspecified ())
                results.addresses.push_back (ep);
        }
    }

private:
    std::string m_name;
    Strings m_strings;
};

//------------------------------------------------------------------------------

std::shared_ptr <Source>
SourceStrings::New (std::string const& name, Strings const& strings)
{
    return std::make_shared<SourceStringsImp>(name, strings);
//    return new SourceStringsImp (name, strings);
}

}
}
