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

#ifndef SKYWELL_PROTOCOL_STTX_H_INCLUDED
#define SKYWELL_PROTOCOL_STTX_H_INCLUDED

#include <protocol/STObject.h>
#include <protocol/TxFormats.h>
#include <protocol/STArray.h>
#include <boost/logic/tribool.hpp>
#include <common/base/Log.h>
#include <set>

namespace skywell {

//  TODO replace these macros with language constants
#define TXN_SQL_NEW         'N'
#define TXN_SQL_CONFLICT    'C'
#define TXN_SQL_HELD        'H'
#define TXN_SQL_VALIDATED   'V'
#define TXN_SQL_INCLUDED    'I'
#define TXN_SQL_UNKNOWN     'U'

class STTx final
    : public STObject
    , public CountedObject <STTx>
{
public:
    static char const* getCountedObjectName () { return "STTx"; }

    typedef std::shared_ptr<STTx>        pointer;
    typedef const std::shared_ptr<STTx>& ref;

public:
    STTx () = delete;
    STTx& operator= (STTx const& other) = delete;

    STTx (STTx const& other) = default;

    explicit STTx (SerialIter& sit);
    explicit STTx (TxType type);

    explicit STTx (STObject&& object);

    STBase*
    copy (std::size_t n, void* buf) const override
    {
        return emplace(n, buf, *this);
    }

    STBase*
    move (std::size_t n, void* buf) override
    {
        return emplace(n, buf, std::move(*this));
    }

    // STObject functions
    SerializedTypeID getSType () const override
    {
        return STI_TRANSACTION;
    }
    std::string getFullText () const override;

    // outer transaction functions / signature functions
    Blob getSignature () const;

    uint256 getSigningHash () const;

    TxType getTxnType () const
    {
        return tx_type_;
    }
    STAmount getTransactionFee () const
    {
        return getFieldAmount (sfFee);
    }
    void setTransactionFee (const STAmount & fee)
    {
        setFieldAmount (sfFee, fee);
    }
    //-----------delete
    SkywellAddress getOperationAccount() const
    {
        return getFieldAccount(sfAccount);
    }

    SkywellAddress getSourceAccount () const
    {
        return getFieldAccount (sfAccount);
    }
    Blob getSigningPubKey () const
    {
        return getFieldVL (sfSigningPubKey);
    }
    void setSigningPubKey (const SkywellAddress & naSignPubKey);
    void setSourceAccount (const SkywellAddress & naSource);

    std::uint32_t getTimestamp () const
    {
        return getFieldU32 (sfTimestamp);
    }

    std::uint32_t getSequence () const
    {
        return getFieldU32 (sfSequence);
    }
    void setSequence (std::uint32_t seq)
    {
        return setFieldU32 (sfSequence, seq);
    }
    
    //Batch operation,get all transaction operations
    void getSTTxs(std::vector<STTx>& txs) const
    {
        std::vector<STTx> sttxs;

        if (getTxnType() != ttOPERATION)
        {
            txs.push_back(*this);
        }
        else
        {
            try
            {
                STArray const& stArr = getFieldArray(sfOperations);
                for (auto i : stArr)
                {
                    STTx::pointer stx = std::make_shared<STTx>(std::move(i));
                    txs.push_back(*stx);
                }
            }
            catch (...)
            {
                WriteLog(lsWARNING, STTx) << "Transaction not legal for format";

                throw std::runtime_error("invalid transaction type");
            }
        }
    }

    void getSignAccount(std::set<Account> &vecAccount) const
    {
        if (isFieldPresent(sfSigns) && getTxnType() == ttOPERATION)
        {
            STArray const& stArr = getFieldArray(sfSigns);
            for (auto i : stArr)
            {
                SkywellAddress acc = SkywellAddress::createAccountPublic(i.getFieldVL(sfSigningPubKey));
                vecAccount.emplace(acc.getAccountID());
            }
        }
        else
        {
            SkywellAddress acc = SkywellAddress::createAccountPublic(getFieldVL(sfSigningPubKey));
            vecAccount.emplace(acc.getAccountID());
        }
    }

    std::vector<SkywellAddress> getMentionedAccounts () const;

    //Batch operation,
    std::map<int, std::vector<Account> > getMentionedAccounts(SkywellAddress const& account);

    uint256 getTransactionID () const;
    Account getContract() const;

    virtual Json::Value getJson (int options) const override;
    virtual Json::Value getJson (int options, bool binary) const;

    bool isMutiSign() const;

    void sign (SkywellAddress const& private_key);

    void sign(SkywellAddress const&, std::map<Account, std::pair<KeyPair, bool> >const&, Json::Value&);

    bool checkSign () const;

    bool isKnownGood () const
    {
        return (sig_state_ == true);
    }
    bool isKnownBad () const
    {
        return (sig_state_ == false);
    }
    void setGood () const
    {
        sig_state_ = true;
    }
    void setBad () const
    {
        sig_state_ = false;
    }

    // SQL Functions with metadata
    static
    std::string const&
    getMetaSQLInsertReplaceHeader ();

    std::string getMetaSQL (std::uint32_t inLedger, std::string const& escapedMetaData) const;

    std::string getMetaSQL (
        Serializer rawTxn,
        std::uint32_t inLedger,
        char status,
        std::string const& escapedMetaData) const;

private:
    TxType tx_type_;

    mutable boost::tribool sig_state_;
};

bool passesLocalChecks(STObject const& st, std::string&);

} // skywell

#endif
