//------------------------------------------------------------------------------
/*
    This file is part of skywelld: https://github.com/skywell/skywelld
    Copyright (c) 2012, 2013 Skywell Labs Inc.

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

#ifndef SKYWELLD_SKYWELL_WEBSOCKET_WEBSOCKET04_H
#define SKYWELLD_SKYWELL_WEBSOCKET_WEBSOCKET04_H

#include <services/websocket/Config04.h>
#include <services/websocket/WebSocket.h>
#include <boost/make_shared.hpp>

namespace skywell {
namespace websocket {

struct WebSocket04
{
    using EndpointBase = websocketpp::server <Config04>;
    
    using Connection = EndpointBase::connection_type;
    using ConnectionPtr = std::shared_ptr<Connection>;
    using ConnectionWeakPtr = std::weak_ptr<Connection>;
    using ErrorCode = std::error_code;
    using Message = Connection::message_type;
    using MessagePtr = Message::ptr;

    class Handler
    {
    public:
        virtual void on_open (ConnectionPtr) = 0;
        virtual void on_close (ConnectionPtr) = 0;
        virtual void on_fail (ConnectionPtr) = 0;
        virtual void on_pong (ConnectionPtr, std::string data) = 0;
        virtual bool http (ConnectionPtr) = 0;
        virtual void on_message (ConnectionPtr, MessagePtr) = 0;
        // This is a new method added by Skywell.
        virtual void on_send_empty (ConnectionPtr) = 0;
    };

    using HandlerPtr = std::shared_ptr<Handler>;

    class Endpoint : public EndpointBase
    {
    public:
        using ptr = std::shared_ptr<Endpoint>;

        Endpoint (HandlerPtr handler) : handler_ (handler)
        {
        }

        HandlerPtr const& handler() { return handler_; }

    private:
        HandlerPtr handler_;
    };

    using EndpointPtr = std::shared_ptr<Endpoint>;
    /** The name of this WebSocket version. */
    static
    char const* versionName();

    /** Handle a connection that was cut off from the other side. */
    static
    void handleDisconnect (Connection&);

    /** Close a client that is too slow to respond. */
    static
    void closeTooSlowClient (
        Connection&,
        unsigned int timeout,
        std::string const& message = "Client is too slow.");

    /** Return true if the WebSocket message is a TEXT message. */
    static
    bool isTextMessage (Message const&);

    /** Create a new Handler. */
    static
    HandlerPtr makeHandler (ServerDescription const&);

    /** Make a connection endpoint from a handler. */
    static 
    EndpointPtr makeEndpoint (HandlerPtr&&);

    /** Get the ASIO strand that this connection lives on. */
    static
    boost::asio::io_service::strand& getStrand (Connection&);
};

} // websocket
} // skywell

#endif
