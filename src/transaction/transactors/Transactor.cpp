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
#include <common/core/Config.h>
#include <transaction/transactors/Transactor.h>
#include <protocol/Indexes.h>
#include <main/Application.h>
#include <ledger/LedgerMaster.h>

namespace skywell {

TER transact_Payment        (STTx const& txn, TransactionEngineParams params, TransactionEngine* engine);

TER
Transactor::transact(STTx const& txn, TransactionEngineParams params, TransactionEngine* engine)
{
    switch (txn.getTxnType())
    {
    case ttPAYMENT:
        return transact_Payment(txn, params, engine);

    default:
        return temUNKNOWN;
    }
}

Transactor::Transactor(STTx const& txn,
                       TransactionEngineParams params,
                       TransactionEngine* engine,    
                       beast::Journal journal)
    : mTxn(txn)
    , mEngine(engine)
    , mParams(params)
    , mHasAuthKey(false)
    , mSigMaster(false)  
    , m_journal (journal)
{
}

void Transactor::calculateFee ()
{
    mFeeDue = STAmount (mEngine->getLedger ()->scaleFeeLoad (calculateBaseFee (), mParams & tapADMIN));
}

std::uint64_t Transactor::calculateBaseFee ()
{
    // Returns the fee in fee units
    return getConfig ().TRANSACTION_FEE_BASE;
}

TER Transactor::payFee ()
{
    STAmount saPaid = mTxn.getTransactionFee ();

    if (!isLegalNet (saPaid))
        return temBAD_AMOUNT;

    // Only check fee is sufficient when the ledger is open.
    if ((mParams & tapOPEN_LEDGER) && saPaid < mFeeDue)
    {
        m_journal.trace << "Insufficient fee paid: " 
                        << saPaid.getText () 
                        << "/" 
                        << mFeeDue.getText ();

        return telINSUF_FEE_P;
    }

    if (saPaid < zero || !saPaid.isNative ())
        return temBAD_FEE;

    if (!saPaid)
        return tesSUCCESS;

    if (mSourceBalance < saPaid)
    {
        m_journal.trace << "Insufficient balance:" 
                        << " balance=" 
                        << mSourceBalance.getText () 
                        << " paid="                      
                        << saPaid.getText ();

        if ((mSourceBalance > zero) && (!(mParams & tapOPEN_LEDGER)))
        {
            // Closed ledger, non-zero balance, less than fee
            mSourceBalance.clear ();
            mTxnAccount->setFieldAmount (sfBalance, mSourceBalance);

            return tecINSUFF_FEE;
        }

        return terINSUF_FEE_B;
    }

    // Deduct the fee, so it's not available during the transaction.
    // Will only write the account back, if the transaction succeeds.
    mSourceBalance -= saPaid;
    mTxnAccount->setFieldAmount (sfBalance, mSourceBalance);
    mFeeAccount->setFieldAmount (sfBalance, mFeeAccount->getFieldAmount (sfBalance)+saPaid);

    return tesSUCCESS;
}

TER Transactor::checkSig ()
{
    // Consistency: Check signature and verify the transaction's signing public
    // key is the key authorized for signing.
 
	if (mParams & tapPRE_CHECKED)
	{
		if (mParams & tapMASTER_SIGN)
		{
			mSigMaster = true;
		}

		return tesSUCCESS;
	}
	else
	{
		auto const signing_account = mSigningPubKey.getAccountID();

		if (signing_account == mTxnAccountID)
		{
			if (mTxnAccount->isFlag(lsfDisableMaster))
				return tefMASTER_DISABLED;

			mSigMaster = true;

			return tesSUCCESS;
		}

		if (!mHasAuthKey)
		{
			m_journal.trace << "Invalid: Not authorized to use account.";

			return temBAD_AUTH_MASTER;
		}

		if (signing_account == mTxnAccount->getFieldAccount160(sfRegularKey))
			return tesSUCCESS;

		m_journal.trace << "Delay: Not authorized to use account.";

		return tefBAD_AUTH;
	}
	
}

TER Transactor::checkSeq ()
{
    std::uint32_t const t_seq = mTxn.getSequence ();
    std::uint32_t const a_seq = mTxnAccount->getFieldU32 (sfSequence);

    if (t_seq != a_seq)
    {
        if (a_seq < t_seq)
        {
            m_journal.trace << "Transaction has future sequence number " 
                            << "a_seq=" << a_seq 
                            << " t_seq=" << t_seq;

            return terPRE_SEQ;
        }

        if (mEngine->getLedger ()->hasTransaction (mTxn.getTransactionID ()))
            return tefALREADY;

        m_journal.trace << "Transaction has past sequence number " 
                        << "a_seq=" << a_seq 
                        << " t_seq=" << t_seq;

        return tefPAST_SEQ;
    }

    // if (skywell::legacy::emulate027 (mEngine->getLedger()) &&
    //         mTxn.isFieldPresent (sfPreviousTxnID) &&
    //         (mTxnAccount->getFieldH256 (sfPreviousTxnID) != mTxn.getFieldH256 (sfPreviousTxnID)))
    //     return tefWRONG_PRIOR;

    if (mTxn.isFieldPresent (sfAccountTxnID) &&
            (mTxnAccount->getFieldH256 (sfAccountTxnID) != mTxn.getFieldH256 (sfAccountTxnID)))
        return tefWRONG_PRIOR;

    if (mTxn.isFieldPresent (sfLastLedgerSequence) &&
            (mEngine->getLedger()->getLedgerSeq() > mTxn.getFieldU32 (sfLastLedgerSequence)))
        return tefMAX_LEDGER;
	
    mTxnAccount->setFieldU32 (sfSequence, t_seq + 1);

	m_journal.trace << "Inc account: " << mTxnAccount->getFieldH256(sfAccountTxnID)
            		<< " seq to " << t_seq + 1;

    if (mTxnAccount->isFieldPresent (sfAccountTxnID))
        mTxnAccount->setFieldH256 (sfAccountTxnID, mTxn.getTransactionID ());
	
    return tesSUCCESS;
}

// check stuff before you bother to lock the ledger
TER Transactor::preCheck ()
{	
    mTxnAccountID = mTxn.getSourceAccount ().getAccountID ();

    Ledger::pointer lpClosed = getApp().getLedgerMaster ().getClosedLedger();
    mFeeAccountID = lpClosed->getFeeAccountID ();

     if (!mTxnAccountID)  // NULL
    {
        m_journal.warning << "apply: bad transaction source id";

        return temBAD_SRC_ACCOUNT;
    }

    if(getApp().getLedgerMaster ().getClosedLedger()->checkBlackList(mTxnAccountID))
    {
       // You're signing yourself a payment.
       // If bPaths is true, you might be trying some arbitrage.
       m_journal.warning << "BlackList transaction: " << "from blacklist account " << to_string (mTxnAccountID) ;

	   return telBLKLIST;
    }

    // Extract signing key
    // Transactions contain a signing key.  This allows us to trivially verify a
    // transaction has at least been properly signed without going to disk.
    // Each transaction also notes a source account id. This is used to verify
    // that the signing key is associated with the account.
    // XXX This could be a lot cleaner to prevent unnecessary copying.
	mSigningPubKey = SkywellAddress::createAccountPublic(mTxn.getSigningPubKey());
    // Consistency: really signed.
    if (!mTxn.isKnownGood ())
    {
        if (mTxn.isKnownBad () ||
            (!(mParams & tapNO_CHECK_SIGN) && !mTxn.checkSign()))
        {
            mTxn.setBad ();

            m_journal.debug << "apply: Invalid transaction (bad signature)";

            return temINVALID;
        }

        mTxn.setGood ();
    }

    return tesSUCCESS;
}

TER Transactor::apply ()
{
	m_journal.trace << "apply to account: " << mTxnAccountID; 

	TER terResult(preCheck());

	if (terResult != tesSUCCESS)
		return terResult;

	// Find source account
	mTxnAccount = mEngine->view().entryCache(ltACCOUNT_ROOT, getAccountRootIndex(mTxnAccountID));
	mFeeAccount = mEngine->view().entryCache(ltACCOUNT_ROOT, getAccountRootIndex(mFeeAccountID));

	calculateFee();

	// If are only forwarding, due to resource limitations, we might verifying
	// only some transactions, this would be probabilistic.
	if (!mTxnAccount)
	{
		if (mustHaveValidAccount())
		{
			m_journal.trace << "apply: delay transaction: source account does not exist " << mTxn.getSourceAccount().humanAccountID();

			return terNO_ACCOUNT;
		}
	}
	else
	{
		mPriorBalance  = mTxnAccount->getFieldAmount(sfBalance);
		mSourceBalance = mPriorBalance;
		mHasAuthKey    = mTxnAccount->isFieldPresent(sfRegularKey);
	}

	
	if (!(mParams & tapPRE_CHECKED))
	{
		terResult = checkSeq();		

		if (terResult != tesSUCCESS) return (terResult);

		if (!mFeeAccount)
		{
			// no fee acoount,create it
			// Create the account.
			auto const newIndex = getAccountRootIndex(mFeeAccountID);
			mFeeAccount = mEngine->view().entryCreate(ltACCOUNT_ROOT, newIndex);
			mFeeAccount->setFieldAccount(sfAccount, mFeeAccountID);
			mFeeAccount->setFieldU32(sfSequence, 1);
		}

		terResult = payFee();
	}

	if (terResult != tesSUCCESS) return (terResult);

	terResult = checkSig();

	if (terResult != tesSUCCESS) return (terResult);

	if (mTxnAccount)
		mEngine->view().entryModify(mTxnAccount);

	if (mFeeAccount)
		mEngine->view().entryModify(mFeeAccount);

	return doApply();
}

}//skywell
