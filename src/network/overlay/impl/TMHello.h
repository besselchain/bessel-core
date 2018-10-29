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

#ifndef BESSEL_OVERLAY_TMHELLO_H_INCLUDED
#define BESSEL_OVERLAY_TMHELLO_H_INCLUDED

#include <main/Application.h>
#include <protocol/BuildInfo.h>
#include <protocol/BesselAddress.h>
#include <protocol/UintTypes.h>
#include <beast/http/message.h>
#include <beast/utility/Journal.h>
#include <utility>
#include <boost/asio/ssl.hpp>
#include <network/bessel.pb.h>

namespace bessel {

enum
{
    // The clock drift we allow a remote peer to have
    clockToleranceDeltaSeconds = 20
};

/** Computes a shared value based on the SSL connection state.
    When there is no man in the middle, both sides will compute the same
    value. In the presence of an attacker, the computed values will be
    different.
    If the shared value generation fails, the link MUST be dropped.
    @return A pair. Second will be false if shared value generation failed.
*/
std::pair<uint256, bool>
makeSharedValue (SSL* ssl, beast::Journal journal);

/** Build a TMHello protocol message. */
protocol::TMHello
buildHello (uint256 const& sharedValue, Application& app);

/** Insert HTTP headers based on the TMHello protocol message. */
void
appendHello (beast::http::message& m, protocol::TMHello const& hello);

/** Parse HTTP headers into TMHello protocol message.
    @return A pair. Second will be false if the parsing failed.
*/
std::pair<protocol::TMHello, bool>
parseHello (beast::http::message const& m, beast::Journal journal);

/** Validate and store the public key in the TMHello.
    This includes signature verification on the shared value.
    @return A pair. Second will be false if the check failed.
*/
std::pair<BesselAddress, bool>
verifyHello (protocol::TMHello const& h
        , uint256 const& sharedValue
        , beast::Journal journal
        , Application& app);

/** Parse a set of protocol versions.
    The returned list contains no duplicates and is sorted ascending.
    Any strings that are not parseable as RTXP protocol strings are
    excluded from the result set.
*/
std::vector<ProtocolVersion>
parse_ProtocolVersions (std::string const& s);

}

#endif