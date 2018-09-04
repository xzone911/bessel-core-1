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

#ifndef SKYWELLD_SKYWELL_WEBSOCKET_WEBSOCKET_H
#define SKYWELLD_SKYWELL_WEBSOCKET_WEBSOCKET_H

#include <services/websocket/MakeServer.h>
#include <common/misc/Utility.h>

namespace skywell {
namespace websocket {

using ScopedLockType = std::lock_guard <std::mutex>;

// std::unique_ptr<beast::Stoppable> makeServer02 (ServerDescription const&);
std::unique_ptr<beast::Stoppable> makeServer04 (ServerDescription const&);

} // websocket
} // skywell

#endif
