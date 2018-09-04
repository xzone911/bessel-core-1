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

#ifndef SKYWELL_APP_PEERS_PEERSET_H_INCLUDED
#define SKYWELL_APP_PEERS_PEERSET_H_INCLUDED

#include <common/base/Log.h>
#include <common/core/Job.h>
#include <network/overlay/Peer.h>
#include <beast/chrono/abstract_clock.h>
#include <beast/utility/Journal.h>
#include <boost/asio/deadline_timer.hpp>

namespace skywell {

/** Supports data retrieval by managing a set of peers.

    When desired data (such as a ledger or a transaction set)
    is missing locally it can be obtained by querying connected
    peers. This class manages common aspects of the retrieval.
    Callers maintain the set by adding and removing peers depending
    on whether the peers have useful information.

    This class is an "active" object. It maintains its own timer
    and dispatches work to a job queue. Implementations derive
    from this class and override the abstract hook functions in
    the base.

    The data is represented by its hash.
*/
class PeerSet
{
public:
    typedef beast::abstract_clock <std::chrono::steady_clock> clock_type;

    /** Returns the hash of the data we want. */
    uint256 const& getHash () const
    {
        return mHash;
    }

    /** Returns true if we got all the data. */
    bool isComplete () const
    {
        return mComplete;
    }

    /** Returns false if we failed to get the data. */
    bool isFailed () const
    {
        return mFailed;
    }

    /** Returns the number of times we timed out. */
    int getTimeouts () const
    {
        return mTimeouts;
    }

    bool isActive ();

    /** Called to indicate that forward progress has been made. */
    void progress ()
    {
        mProgress = true;
        mAggressive = false;
    }

    void touch ()
    {
        mLastAction = m_clock.now ();
    }

    clock_type::time_point getLastAction () const
    {
        return mLastAction;
    }

    /** Insert a peer to the managed set.
        This will call the derived class hook function.
        @return `true` If the peer was added
    */
    bool insert (Peer::ptr const&);
    
    virtual bool isDone () const
    {
        return mComplete || mFailed;
    }

private:
    static void TimerEntry (std::weak_ptr<PeerSet>, const boost::system::error_code& result);
    static void TimerJobEntry (Job&, std::shared_ptr<PeerSet>);

protected:
    //  TODO try to make some of these private
    typedef SkywellRecursiveMutex LockType;
    typedef std::unique_lock <LockType> ScopedLockType;

    PeerSet (uint256 const& hash
          , int interval, bool txnData
          , clock_type& clock
          , beast::Journal journal);

    virtual ~PeerSet () = 0;

    virtual void newPeer (Peer::ptr const&) = 0;

    virtual void onTimer (bool progress, ScopedLockType&) = 0;

    virtual std::weak_ptr<PeerSet> pmDowncast () = 0;

    bool isProgress ()
    {
        return mProgress;
    }

    void setComplete ()
    {
        mComplete = true;
    }
    void setFailed ()
    {
        mFailed = true;
    }

    void invokeOnTimer ();

    void sendRequest (const protocol::TMGetLedger& message);

    void sendRequest (const protocol::TMGetLedger& message, Peer::ptr const& peer);

    void setTimer ();

    std::size_t getPeerCount () const;

protected:
    beast::Journal m_journal;
    clock_type& m_clock;

    LockType mLock;

    uint256 mHash;
    int mTimerInterval;
    int mTimeouts;
    bool mComplete;
    bool mFailed;
    bool mAggressive;
    bool mTxnData;
    clock_type::time_point mLastAction;
    bool mProgress;

    //  TODO move the responsibility for the timer to a higher level
    boost::asio::deadline_timer mTimer;

    //  TODO Verify that these are used in the way that the names suggest.
    typedef Peer::id_t PeerIdentifier;
    typedef int ReceivedChunkCount;
    typedef hash_map<PeerIdentifier, ReceivedChunkCount> PeerSetMap;

    PeerSetMap mPeers;
};

} // skywell

#endif
