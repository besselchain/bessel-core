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

#ifndef BESSEL_SERVER_SERVERHANDLER_H_INCLUDED
#define BESSEL_SERVER_SERVERHANDLER_H_INCLUDED

#include <common/base/BasicConfig.h>
#include <services/server/Port.h>
#include <services/rpc/Yield.h>
#include <network/overlay/Overlay.h>
#include <beast/utility/Journal.h>
#include <beast/utility/PropertyStream.h>
#include <boost/asio/ip/address.hpp>
#include <common/misc/Utility.h>
#include <vector>

namespace bessel {

class ServerHandler
    : public beast::Stoppable
    , public beast::PropertyStream::Source
{
protected:
    ServerHandler (Stoppable& parent);

public:
    struct Setup
    {
        std::vector<HTTP::Port> ports;

        // Memberspace
        struct client_t
        {
            bool secure = false;
            std::string ip;
            std::uint16_t port = 0;
            std::string user;
            std::string password;
            std::string admin_user;
            std::string admin_password;
        };

        // Configuration when acting in client role
        client_t client;

        // Configuration for the Overlay
        struct overlay_t
        {
            boost::asio::ip::address ip;
            std::uint16_t port = 0;
        };

        overlay_t overlay;
        RPC::YieldStrategy yieldStrategy;

        void
        makeContexts();
    };

    virtual
    ~ServerHandler() = default;

    /** Opens listening ports based on the Config settings
        This is implemented outside the constructor to support
        two-stage initialization in the Application object.
    */
    virtual
    void
    setup (Setup const& setup, beast::Journal journal) = 0;

    /** Returns the setup associated with the handler. */
    virtual
    Setup const&
    setup() const = 0;

    /** Fills in boilerplate HTTP header field values. */
    static
    void
    appendStandardFields (beast::http::message& message);
};

//------------------------------------------------------------------------------

ServerHandler::Setup
setup_ServerHandler (BasicConfig const& c, std::ostream& log);

} // bessel

#endif
