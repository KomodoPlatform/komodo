#!/usr/bin/env python3

from lib import rpclib, tuilib
import os
import time
import json
from slickrpc.exc import RpcException

# test creates fungible and non-fungible tokens
# performs transfers and multiple transfers
# then runs assets cc ask/bids tests

header = "komodo assets cc tokenask/bid v2 test\n"

def get_result_error(r):
    if isinstance(r, dict) :
        if r.get('result') == "error" :
            return r.get('error')
    return ''

def check_tx(r):
    if isinstance(r, str) :
        # print("r is str, true")
        return True
    elif isinstance(r, dict) :
        # print("r is dict")
        str_hex = r.get('hex')
        if isinstance(str_hex, str) :
            # print("r has hex, true")
            if str_hex.find('coins') >= 0 or str_hex.find('tokens') >= 0 or str_hex.find('assets') >= 0 :  # try to detect old code returning no coins err as success
                print("bad tx result:", r)
                return False
            else :
                return True
        if r.get('result') == "error" :
            print("bad tx result:", r)
            return False
    # print("r has unknown, true")
    return True
    
def check_txid(txid) :
    if isinstance(txid, str) :
        if len(txid) == 64 :
            return True
    return False

def run_tokens_create(rpc):

    # set your own two node params
    # DIMXY20 
    rpc1 = rpclib.rpc_connect("user3088995989", "pass53c05895f37aa1eda9b0de63944275668e1956abb8c38f186132262fe53ae9be7c", 14723)
    rpc2 = rpclib.rpc_connect("user2898668153", "passe99167496d2bbe43876e60430d52eb37b547fa1085b741a15776454eb126d9a603", 15723)
    rpc3 = rpclib.rpc_connect("user972794450", "passe7eb16f5c015a53463cc5f27a004854cb76f4ec5c9aece177f01d8b3d13119e445", 16723)


    ## for v in ["", "v2"] :
    for v in ["v2"] :
        print("creating fungible token 1...")
        result = call_rpc(rpc1, "token"+v+"create", "T1", str(0.000001))  # 100
        assert(check_tx(result))
        tokenid1 = rpc1.sendrawtransaction(result['hex'])
        print("created token:", tokenid1)
        
        print("creating fungible token 2...")
        result = call_rpc(rpc1, "token"+v+"create", "T2", str(0.1))
        assert(check_tx(result))
        tokenid2 = rpc1.sendrawtransaction(result['hex'])
        print("created token:", tokenid2)

        print("creating tokel NFT 1 with empty data...")
        result = call_rpc(rpc1, "token"+v+"createtokel", "NFT-00-1", str(0.00000001), "nft tokel empty", '{}')
        assert(check_tx(result))
        nft00id1 = rpc1.sendrawtransaction(result['hex'])
        print("created token:", nft00id1)

        print("creating tokel NFT 2 with arbitrary data...")
        result = call_rpc(rpc1, "token"+v+"createtokel", "NFT-00-2", str(0.00000001), "nft tokel arbitrary", '{"arbitrary":"010203"}')
        assert(check_tx(result))
        nft00id2 = rpc1.sendrawtransaction(result['hex'])
        print("created token:", nft00id2)

        #  tokel nft data F7 evalcode
        print("creating tokel NFT with royalty 0...")
        result = call_rpc(rpc1, "token"+v+"createtokel", "NFT-F7-1", str(0.00000001), "nft tokel royalty=0", '{"royalty":0}')
        assert(check_tx(result))
        nftf7id1 = rpc1.sendrawtransaction(result['hex'])
        print("created token:", nftf7id1)

        print("creating tokel NFT with royalty 99...")
        result = call_rpc(rpc1, "token"+v+"createtokel", "NFT-F7-2", str(0.00000001), "nft tokel royalty=99", '{"royalty":99}')
        assert(check_tx(result))
        nftf7id2 = rpc1.sendrawtransaction(result['hex'])
        print("created token:", nftf7id2)

        print("creating tokel NFT with royalty 1 (to test dust)...")
        result = call_rpc(rpc1, "token"+v+"createtokel", "NFT-F7-3", str(0.00000001), "nft tokel royalty=1", '{"royalty":1}')
        assert(check_tx(result))
        nftf7id3 = rpc1.sendrawtransaction(result['hex'])
        print("created token:", nftf7id3)

        print("creating tokel NFT with royalty 500 (to test dust)...")
        result = call_rpc(rpc1, "token"+v+"createtokel", "NFT-F7-4", str(0.00000001), "nft tokel royalty=500", '{"royalty":500}')
        assert(check_tx(result))
        nftf7id4 = rpc1.sendrawtransaction(result['hex'])
        print("created token:", nftf7id4)

        # first try transfer tokens to a pk and back, then run assets tests
        print("starting transfer tests for tokenid version=" + v + "...")
        run_transfers(rpc1, rpc2, v, tokenid1, tokenid2, 10)
        print("starting transfer tests for nft00id version=" + v + "...")
        run_transfers(rpc1, rpc2, v, nft00id1, nft00id2, 1)
        print("starting transfer tests for nftf7id version=" + v + "...")
        run_transfers(rpc1, rpc2, v, nftf7id1, nftf7id2, 1)
        print("token transfers tests finished okay")
        time.sleep(3)

        # assets cc tests:
        print("starting assets tests for tokenid1 version=" + v + "...")
        run_assets_orders(rpc1, rpc2, v, tokenid1, 10, 8, 0.0001, False)
        print("starting assets tests for nft00id1 version=" + v + "...")
        run_assets_orders(rpc1, rpc2, v, nft00id1, 1, 1, 0.0001, True)
        print("starting assets tests for nftf7id1 version=" + v + "...")
        run_assets_orders(rpc1, rpc2, v, nftf7id1, 1, 1, 0.0001, True)
        print("starting assets tests for nftf7id2 version=" + v + "...")
        run_assets_orders(rpc1, rpc2, v, nftf7id2, 1, 1, 0.0001, True)
        print("starting assets tests for nftf7id3 version=" + v + "...")
        run_assets_orders(rpc1, rpc2, v, nftf7id3, 1, 1, 0.0001, True)
        print("starting assets tests for nftf7id4 with unit price creating dust royalty and checking that round correct, version=" + v + "...")
        run_assets_orders(rpc1, rpc2, v, nftf7id4, 1, 1, 0.0000_1001, True)


        if v == "v2" :  # MofN supported for tokens cc v2 only
            print("running MofN tests for tokens v2:")
            print("starting MofN tests for tokenid1...")
            run_MofN_transfers(rpc1, rpc2, rpc3, tokenid1, 10)
            print("starting MofN tests for nft00id1...")
            run_MofN_transfers(rpc1, rpc2, rpc3, nft00id1, 1)
            print("starting MofN tests for nftf7id1...")
            run_MofN_transfers(rpc1, rpc2, rpc3, nftf7id1, 1)        

    print("token/assets tests finished okay")
    time.sleep(3)
    exit

def call_rpc(rpc, rpcname, *args) :
    rpcfunc = getattr(rpc, rpcname)
    return rpcfunc(*args)

def call_token_rpc_create_tx(rpc, rpcname, stop_error, *args) :
    retries = 24
    delay = 10
    rpcfunc = getattr(rpc, rpcname)
    for i in range(retries):
        print("calling " + rpcname)
        result = rpcfunc(*args)
        print(rpcname + " create tx result:", result, " arg=", args[0])
        if  check_tx(result):
            break
        if stop_error and get_result_error(result) == stop_error :  
            print(rpcname + " retrying stopped because of expected stop error received: " + stop_error)
            return result
        if i < retries-1:
            print("retrying " + rpcname + '...')
            time.sleep(delay)                
    assert(check_tx(result))
    print(rpcname + " tx created")
    return result

def call_token_rpc_send_tx(rpc, rpcname, stop_error, *args) :

    for i in range(3) :
        result = call_token_rpc_create_tx(rpc, rpcname, stop_error, *args)
        if check_tx(result) :
            try :
                txid = rpc.sendrawtransaction(result['hex'])
                print("sendrawtransaction result: ", txid)
                assert(check_tx(txid))            
                print(rpcname + " tx sent")
                return txid
            except RpcException as e :
                if e.message.find('replacement in mempool') >= 0 or e.message.find('missing inputs') >= 0 :
                    print('double spending in mempool', e.message, 'retrying...')
                    pass
                else :
                    assert False, e
        else :
            return result
    assert False, 'sendrawtransaction no more retries'

def call_rpc_retry(rpc, rpcname, stop_error, *args) :
    retries = 24
    delay = 10
    rpcfunc = getattr(rpc, rpcname)
    for i in range(retries):
        print("calling " + rpcname)
        result = rpcfunc(*args)
        print(rpcname + " result:", result)
        if  check_tx(result):
            break
        if stop_error and get_result_error(result) == stop_error :  
            print(rpcname + " retrying stopped because of expected stop error received: " + stop_error)
            return result
        if i < retries-1:
            print("retrying " + rpcname + '...')
            time.sleep(delay)                
    assert(check_tx(result))
    print(rpcname + " tx created")
    return result

def call_token_rpc(rpc, rpcname, stop_error, *args) :
    print("calling " + rpcname + " for tokenid=", args[0])
    return call_rpc_retry(rpc, rpcname, stop_error, *args)

def run_transfers(rpc1, rpc2, v, tokenid1, tokenid2, amount):
   
    pubkey1 = rpc1.getinfo()['pubkey']
    pubkey2 = rpc2.getinfo()['pubkey']

    # rpc1.setgenerate(True, 2)

    if amount == 1 :
        print("creating token"+v+"transfer tokenid1 amount tx...")
        call_token_rpc_send_tx(rpc1, "token"+v+"transfer", '', tokenid1, pubkey2,  str(amount))
    else :
        # try two transfers
        print("creating token"+v+"transfer tokenid1 amount-1 tx...")
        txid = call_token_rpc_send_tx(rpc1, "token"+v+"transfer", '', tokenid1, pubkey2,  str(amount-1))
        assert(check_txid(txid))   

        print("creating token"+v+"transfer tokenid1 1 tx...")
        txid = call_token_rpc_send_tx(rpc1, "token"+v+"transfer", '', tokenid1, pubkey2,  str(1))
        assert(check_txid(txid))   

    print("creating token"+v+"transfer tokenid1 tx back...")
    txid = call_token_rpc_send_tx(rpc2, "token"+v+"transfer", '', tokenid1, pubkey1,  str(amount))
    assert(check_txid(txid))   

    print("creating token"+v+"transfermany tokenid1 tokenid2 tx...")
    txid = call_token_rpc_send_tx(rpc1, "token"+v+"transfermany", '', tokenid1, tokenid2, pubkey2,  str(amount))
    assert(check_txid(txid))   

    print("creating token"+v+"transfermany tokenid1 tokenid2 tx back...")
    txid = call_token_rpc_send_tx(rpc2, "token"+v+"transfermany", '', tokenid1, tokenid2, pubkey1,  str(amount))
    assert(check_txid(txid))   


def run_MofN_transfers(rpc1, rpc2, rpc3, tokenid, amount):
   
    pubkey1 = rpc1.getinfo()['pubkey']
    pubkey2 = rpc2.getinfo()['pubkey']
    pubkey3 = rpc3.getinfo()['pubkey']


    if  amount > 1 :
        amountback = int(amount / 2) # if not nft send half amount back twice
    else :
        amountback = amount

    time.sleep(1) # pause to allow tx in mempool to propagate to prevent double spending in mempool               

    # try 1of2
    param = {}
    param["tokenid"] = tokenid
    param["destpubkeys"] = [ pubkey1, pubkey2 ]
    param["M"] = 1
    param["amount"] = amount
    print("creating tokenv2transfer tokenid amount tx to 1of2 tx...", json.dumps(param))
    tx1of2 = call_token_rpc_create_tx(rpc1, "tokenv2transfer", '', json.dumps(param))
    txid = rpc1.sendrawtransaction(tx1of2['hex'])
    print("sendrawtransaction result: ", txid)
    assert(check_txid(txid))   
    ccaddr1of2 = rpc1.decoderawtransaction(tx1of2['hex'])['vout'][0]['scriptPubKey']['addresses'][0]
    
    time.sleep(1) # pause to allow tx in mempool to propagate to prevent double spending in mempool               

    # spend 1of2 to pk1 or pk2
    param = {}
    param["tokenid"] = tokenid
    param["ccaddressMofN"] = ccaddr1of2
    param["M"] = 1
    param["amount"] = amountback
    if amount > 1 :  # if not nft first send half to pk2 (to test lib cc fulfilment ordering)
        # try firs1 to pk2
        param["destpubkeys"] = [ pubkey2 ]
        print("creating tokenv2transfer tokenid1 tx spending 1of2 back to pk2...", json.dumps(param))
        txid = call_token_rpc_send_tx(rpc2, "tokenv2transfer", '', json.dumps(param))
        assert(check_txid(txid))   
    else :
        # if nft send 1 back to pk1 for the next test
        param["destpubkeys"] = [ pubkey1 ]
        print("creating tokenv2transfer tokenid tx spending 1of2 back to pk1...", json.dumps(param))
        txid = call_token_rpc_send_tx(rpc1, "tokenv2transfer", '', json.dumps(param))
        assert(check_txid(txid))   

    if amount > 1 :  # if not nft send half amount back to pk1
        # wait to prevent adding same inputs while tx is not propagated to this node mempool
        time.sleep(3)   
        param["destpubkeys"] = [ pubkey1 ]
        print("creating tokenv2transfer tokenid tx spending 1of2 back to pk1...",json.dumps(param))
        txid = call_token_rpc_send_tx(rpc1, "tokenv2transfer", '', json.dumps(param))
        assert(check_txid(txid))   

    time.sleep(1) # pause to allow tx in mempool to propagate to prevent double spending in mempool               

    # try 2of2
    param = {}
    param["tokenid"] = tokenid
    param["destpubkeys"] = [ pubkey1, pubkey2 ]
    param["M"] = 2
    param["amount"] = amount
    print("creating tokenv2transfer tokenid amount tx to 2of2 ...", json.dumps(param))
    tx2of2 = call_token_rpc_create_tx(rpc1, "tokenv2transfer", '', json.dumps(param))
    txid = rpc1.sendrawtransaction(tx2of2['hex'])
    print("sendrawtransaction result: ", txid)
    assert(check_txid(txid))   
    ccaddr2of2 = rpc1.decoderawtransaction(tx2of2['hex'])['vout'][0]['scriptPubKey']['addresses'][0]

    time.sleep(1) # pause to allow tx in mempool to propagate to prevent double spending in mempool               

    # spend 2of2 to pk1
    param = {}
    param["tokenid"] = tokenid
    param["ccaddressMofN"] = ccaddr2of2
    param["destpubkeys"] = [ pubkey1 ]
    param["M"] = 1
    param["amount"] = amount
    print("creating tokenv2transfer tokenid tx spending 2of2 back to pk1...", json.dumps(param))
    partialtx = call_token_rpc(rpc1, "tokenv2transfer", '', json.dumps(param))
    assert partialtx['hex'], 'partial tx not created'
    assert partialtx['PartiallySigned'], 'partially signed object'

    try :
        print("trying to sendrawtransaction partially 1 signed tx... ")
        result = rpc2.sendrawtransaction(partialtx['hex'])
        print("sending partially1 signed tx returned:", result)
        assert not result, 'sending partially signed 2of2 tx should return error'
    except RpcException as e :
        print ('got a normal exception (note "FULFILLMENT NOT ENCODED" msg on console is normal)', e.message)
        pass # should be error 

    tx2of2back = call_rpc_retry(rpc2, "addccv2signature", '', partialtx['hex'], partialtx['PartiallySigned'])
    assert tx2of2back['hex'], 'sig 2 not added to tx'
    txid = rpc2.sendrawtransaction(tx2of2back['hex'])
    print("sendrawtransaction result: ", txid)
    assert(check_txid(txid))            
    print("tx 2of2 back sent")

    time.sleep(1) # pause to allow tx in mempool to propagate to prevent double spending in mempool               

    # try 2of3
    param = {}
    param["tokenid"] = tokenid
    param["destpubkeys"] = [ pubkey1, pubkey2, pubkey3 ]
    param["M"] = 2
    param["amount"] = amount
    print("creating tokenv2transfer tokenid amount tx to 2of3 tx...", json.dumps(param))
    tx2of3 = call_token_rpc_create_tx(rpc1, "tokenv2transfer", '', json.dumps(param))
    txid = rpc1.sendrawtransaction(tx2of3['hex'])
    print("sendrawtransaction result: ", txid)
    assert(check_txid(txid))   
    ccaddr2of3 = rpc1.decoderawtransaction(tx2of3['hex'])['vout'][0]['scriptPubKey']['addresses'][0]
    
    time.sleep(1) # pause to allow tx in mempool to propagate to prevent double spending in mempool               

    # spend 2of3 to pk1
    param = {}
    param["tokenid"] = tokenid
    param["ccaddressMofN"] = ccaddr2of3
    param["M"] = 1
    param["amount"] = amountback
    param["destpubkeys"] = [ pubkey1 ]
    print("creating tokenv2transfer tokenid tx spending 2of3 back to pk1...", json.dumps(param))
    partialtx = call_token_rpc(rpc1, "tokenv2transfer", '', json.dumps(param))
    assert partialtx['hex'], 'partial tx not created'
    assert partialtx['PartiallySigned'], 'partially signed object'

    try :
        print("trying to sendrawtransaction partially 1 signed tx... ")
        result = rpc2.sendrawtransaction(partialtx['hex'])
        print("sending partially1 signed tx returned:", result)
        assert not result, 'sending partially signed 2of3 tx should return error'
    except RpcException as e :
        print ('got a normal exception (note "FULFILLMENT NOT ENCODED" msg on console is normal)', e.message)
        pass # should be error 


    # add sig 2
    tx2of3back = call_rpc_retry(rpc2, "addccv2signature", '', partialtx['hex'], partialtx['PartiallySigned'])
    assert tx2of3back['hex'], 'sig 2 not added to tx'
    txid = rpc2.sendrawtransaction(tx2of3back['hex'])
    print("sendrawtransaction result: ", txid)
    assert(check_txid(txid))            
    print("tx 2of3 back sent")

    time.sleep(1) # pause to allow tx in mempool to propagate to prevent double spending in mempool               

    # try 3of3
    param = {}
    param["tokenid"] = tokenid
    param["destpubkeys"] = [ pubkey1, pubkey2, pubkey3 ]
    param["M"] = 3
    param["amount"] = amount
    print("creating tokenv2transfer tokenid amount tx to 3of3 ...", json.dumps(param))
    tx3of3 = call_token_rpc_create_tx(rpc1, "tokenv2transfer", '', json.dumps(param))
    txid = rpc1.sendrawtransaction(tx3of3['hex'])
    print("sendrawtransaction result: ", txid)
    assert(check_txid(txid))   
    ccaddr3of3 = rpc1.decoderawtransaction(tx3of3['hex'])['vout'][0]['scriptPubKey']['addresses'][0]

    time.sleep(1) # pause to allow tx in mempool to propagate to prevent double spending in mempool               

    # spend 3of3 to pk1
    param = {}
    param["tokenid"] = tokenid
    param["ccaddressMofN"] = ccaddr3of3
    param["destpubkeys"] = [ pubkey1 ]
    param["M"] = 1
    param["amount"] = amount
    print("creating tokenv2transfer tokenid tx spending 3of3 back to pk1...", json.dumps(param))
    partialtx1 = call_token_rpc(rpc1, "tokenv2transfer", '', json.dumps(param))
    assert partialtx1['hex'], 'partial tx not created'
    assert partialtx1['PartiallySigned'], 'partially signed object'

    try :
        print("trying to sendrawtransaction partially 1 signed tx... ")
        result = rpc2.sendrawtransaction(partialtx1['hex'])
        print("sending partially1 signed tx returned:", result)
        assert not result, 'sending partially signed 3of3 tx should return error'
    except RpcException as e :
        print ('got a normal exception (note "FULFILLMENT NOT ENCODED" msg on console is normal)', e.message)
        pass # should be error 

    # add sig 2
    partialtx2 = call_rpc_retry(rpc2, "addccv2signature", '', partialtx1['hex'], partialtx1['PartiallySigned'])
    assert partialtx2['hex'], 'sig 2 not added to tx'
    assert partialtx2['PartiallySigned'], 'partially signed object'

    try :
        print("trying to sendrawtransaction partially 2 signed tx... ")
        result = rpc2.sendrawtransaction(partialtx2['hex'])
        print("sending partially2 signed tx returned:", result)
        assert not result, 'sending partially signed 3of3 tx should return error'
    except RpcException as e :
        print ('got a normal exception (note "FULFILLMENT NOT ENCODED" msg on console is normal)', e.message)
        pass # should be error 

    # add sig 3
    tx3of3back = call_rpc_retry(rpc3, "addccv2signature", '', partialtx2['hex'], partialtx2['PartiallySigned'])
    assert tx3of3back['hex'], 'sig 3 not added to tx'

    txid = rpc3.sendrawtransaction(tx3of3back['hex'])
    print("sendrawtransaction result: ", txid)
    assert(check_txid(txid))            
    print("tx 3of3 back sent")


def run_assets_orders(rpc1, rpc2, v, tokenid, total, units, unitprice, isnft):

    retries = 24
    delay = 10
   
    askunits = bidunits = units
    # unitprice = 0.0001

    # wait for mempool to empty to get correct balance: 
    retries = 24
    delay = 10
    for i in range(retries):
        print("calling " + "getmempoolinfo...")
        mempoolinfo = rpc1.getmempoolinfo()
        if int(mempoolinfo["size"]) == 0 :
            break
        if i < retries-1:
            print("retrying " + "getmempoolinfo size=" + str(mempoolinfo["size"]) + '...')
            time.sleep(delay)  

    # get initial balance
    result = call_rpc(rpc1, "token"+v+"balance", tokenid)
    assert(check_tx(result))
    initial_balance = int(result["balance"])
    print("initial balance for tokenid " + tokenid + " = " + str(initial_balance))

    # create tokenask 1
    print("creating token"+v+"ask tx #1...")
    askid1 = call_token_rpc_send_tx(rpc1, "token"+v+"ask", '', str(total), tokenid, str(unitprice))

    if not isnft :
        # create tokenask 2
        print("creating token"+v+"ask tx #2...")
        askid2 = call_token_rpc_send_tx(rpc1, "token"+v+"ask", '', str(total), tokenid, str(unitprice))

        # create tokenask 3
        print("creating token"+v+"ask tx #3...")
        askid3 = call_token_rpc_send_tx(rpc1, "token"+v+"ask", '', str(total), tokenid, str(unitprice))

    # fill ask 
    if not isnft :
        print("creating token"+v+"fillask tx #1 in backward order...")
        fillaskid1 = call_token_rpc_send_tx(rpc2, "token"+v+"fillask", '', tokenid, askid3, str(askunits))
        print("creating token"+v+"fillask tx #2...")
        fillaskid2 = call_token_rpc_send_tx(rpc2, "token"+v+"fillask", '', tokenid, askid2, str(askunits))
        print("creating token"+v+"fillask tx #3...")
        fillaskid3 = call_token_rpc_send_tx(rpc2, "token"+v+"fillask", '', tokenid, askid1, str(askunits))
    else :
        print("creating token"+v+"fillask tx #1...")
        fillaskid1 = call_token_rpc_send_tx(rpc2, "token"+v+"fillask", '', tokenid, askid1, str(askunits))


    # cancel ask
    print("creating token"+v+"cancelask tx #1...")
    cancelid = call_token_rpc_send_tx(rpc1, "token"+v+"cancelask", 'ask is empty', tokenid, fillaskid1)
    if not isnft :
        print("creating token"+v+"cancelask tx #2...")
        cancelid = call_token_rpc_send_tx(rpc1, "token"+v+"cancelask", 'ask is empty', tokenid, fillaskid2)
        print("creating token"+v+"cancelask tx #3...")
        cancelid = call_token_rpc_send_tx(rpc1, "token"+v+"cancelask", 'ask is empty', tokenid, fillaskid3)

    # create tokenbid to buy back tokens - same amount
    print("creating token"+v+"bid tx #1...")
    bidid1 = call_token_rpc_send_tx(rpc1, "token"+v+"bid", '', str(askunits), tokenid, str(unitprice))
    if not isnft :
        print("creating token"+v+"bid tx #2...")
        bidid2 = call_token_rpc_send_tx(rpc1, "token"+v+"bid", '', str(askunits), tokenid, str(unitprice))
        print("creating token"+v+"bid tx #3...")
        bidid3 = call_token_rpc_send_tx(rpc1, "token"+v+"bid", '', str(askunits), tokenid, str(unitprice))

    # fill bid 
    print("creating token"+v+"fillbid tx #1...")
    fillbidid1 = call_token_rpc_send_tx(rpc2, "token"+v+"fillbid", '', tokenid, bidid1, str(bidunits))
    if not isnft :
        print("creating token"+v+"fillbid tx #2...")
        fillbidid2 = call_token_rpc_send_tx(rpc2, "token"+v+"fillbid", '', tokenid, bidid2, str(bidunits))
        print("creating token"+v+"fillbid tx #3...")
        fillbidid3 = call_token_rpc_send_tx(rpc2, "token"+v+"fillbid", '', tokenid, bidid3, str(bidunits))

    # cancel bid
    print("creating token"+v+"cancelbid tx #1...")
    cancelid = call_token_rpc_send_tx(rpc1, "token"+v+"cancelbid", 'bid is empty', tokenid, fillbidid1)
    if not isnft :
        print("creating token"+v+"cancelbid tx #2...")
        cancelid = call_token_rpc_send_tx(rpc1, "token"+v+"cancelbid", 'bid is empty', tokenid, fillbidid2)
        print("creating token"+v+"cancelbid tx #3...")
        cancelid = call_token_rpc_send_tx(rpc1, "token"+v+"cancelbid", 'bid is empty', tokenid, fillbidid3)

    # create tokenbid to self to test dust
    print("creating token"+v+"bid tx #4...")
    bidid4 = call_token_rpc_send_tx(rpc1, "token"+v+"bid", '', str(1), tokenid, str(0.0000_0600))  # price more than dust

    # fill bid to self with dust price
    print("creating token"+v+"fillbid tx #4 (to check failure on dust unit price)...")
    # enable this if dust is enabled on normals in the chain:
    # fillbidid4 = call_token_rpc_send_tx(rpc1, "token"+v+"fillbid", '', tokenid, bidid4, str(1), str(0.0000_005)) # fill with dust price (<=500sat)
    # if dust not enabled we should get error:
    fillbidid4 = call_token_rpc(rpc1, "token"+v+"fillbid", '', tokenid, bidid4, str(1), str(0.0000_005)) # fill with dust price (<=500sat)
    assert fillbidid4['hex'], "token"+v+"fillbid" + ' tx not created'

    try :
        print("trying to sendrawtransaction tokenfillbid tx with dust... ")
        result = rpc1.sendrawtransaction(fillbidid4['hex'])
        print("sendrawtransaction tokenfillbid tx with dust returned:", result)
        assert not result, 'sending tokenfillbid tx with dust should return error'
    except RpcException as e :
        print ('got a normal exception for tokenfillbid tx with dust', e.message)
        pass # should be error 
        # balance does not change as this is a tx to self

    # check balance after 
    retries = 24
    delay = 10
    for i in range(retries):
        print("calling " + "token"+v+"balance")
        finresult = call_rpc(rpc1, "token"+v+"balance", tokenid)
        assert(check_tx(result))
        if int(finresult["balance"]) == initial_balance :
            break
        if i < retries-1:
            print("retrying " + "token"+v+"balance" + '...')
            time.sleep(delay)                

    assert(int(finresult["balance"]) == initial_balance)

menuItems = [
    {"run token create/transfers and assets orders": run_tokens_create},
    {"Exit": exit}
]


def main():
    while True:
        os.system('clear')
        print(tuilib.colorize(header, 'pink'))
        print(tuilib.colorize('CLI version 0.2\n', 'green'))
        for item in menuItems:
            print(tuilib.colorize("[" + str(menuItems.index(item)) + "] ", 'blue') + list(item.keys())[0])
        choice = input(">> ")
        try:
            if int(choice) < 0:
                raise ValueError
            # Call the matching function
            if list(menuItems[int(choice)].keys())[0] == "Exit":
                list(menuItems[int(choice)].values())[0]()
            else:
                list(menuItems[int(choice)].values())[0](rpc_connection)
        except (ValueError, IndexError):
            pass


if __name__ == "__main__":
    print("starting assets orders test (remember to set rpc params for two nodes for your chain)")
    time.sleep(2)   
    rpc_connection = ""
    main()

