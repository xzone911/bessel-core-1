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
#include <transaction/paths/cursor/PathCursor.h>
#include <common/base/Log.h>
#include <tuple>

namespace skywell {
namespace path {

TER PathCursor::forwardLiquidity () const
{
    if (node().isAccount())
        return forwardLiquidityForAccount ();

    // Otherwise, node is an offer.
    if (previousNode().account_ == zero)
        return tesSUCCESS;

    // Previous is an account node, resolve its deliver.
    STAmount saInAct;
    STAmount saInFees;

    auto resultCode = deliverNodeForward (
        previousNode().account_,
        previousNode().saFwdDeliver, // Previous is sending this much.
        saInAct,
        saInFees);

    assert (resultCode != tesSUCCESS ||
            previousNode().saFwdDeliver == saInAct + saInFees);
    return resultCode;
}

} // path
} // skywell

// Original comments:

// Called to drive the from the first offer node in a chain.
//
// - Offer input is in issuer/limbo.
// - Current offers consumed.
//   - Current offer owners debited.
//   - Transfer fees credited to issuer.
//   - Payout to issuer or limbo.
// - Deliver is set without transfer fees.
