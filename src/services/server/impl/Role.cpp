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

#include <BeastConfig.h>
#include <services/server/Role.h>
#include <boost/asio.hpp>

namespace skywell {

bool
passwordUnrequiredOrSentCorrect (HTTP::Port const& port,
                                 Json::Value const& params) 
{

    assert(! port.admin_ip.empty ());

    bool const passwordRequired = (!port.admin_user.empty() || !port.admin_password.empty());

    return !passwordRequired  ||
            ((params["admin_password"].isString() &&
              params["admin_password"].asString() == port.admin_password) &&
             (params["admin_user"].isString() &&
              params["admin_user"].asString() == port.admin_user));
}

bool
ipAllowed (boost::asio::ip::address const& remoteIp, std::vector<boost::asio::ip::address> const& adminIp)
{
    return std::find_if (adminIp.begin (), adminIp.end (),
        [&remoteIp](boost::asio::ip::address const& ip) 
        { 
            boost::asio::ip::address address(boost::asio::ip::address::from_string("0.0.0.0"));
            return ip == address || ip == remoteIp; 

        }) != adminIp.end ();
}

bool
isAdmin (HTTP::Port const& port, Json::Value const& params, boost::asio::ip::address const& remoteIp)
{
    return ipAllowed (remoteIp, port.admin_ip) && passwordUnrequiredOrSentCorrect (port, params);
}

Role
requestRole (Role const& required, 
             HTTP::Port const& port,
             Json::Value const& params, 
             boost::asio::ip::tcp::endpoint const& remoteIp)
{
    Role role (Role::GUEST);

    if (isAdmin(port, params, remoteIp.address ()))
        role = Role::ADMIN;

    if (required == Role::ADMIN && role != required)
        role = Role::FORBID;

    return role;
}

}
