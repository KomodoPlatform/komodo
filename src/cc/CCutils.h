/******************************************************************************
 * Copyright © 2014-2021 The SuperNET Developers and others.                  *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/
#pragma once
#include "cryptoconditions.h" // CC
#include "script/script.h" // CScript

CC* GetCryptoCondition(CScript const& scriptSig);
bool IsCCInput(CScript const& scriptSig);
