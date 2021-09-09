//
//  PumpHouseAPISecretMgr.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/9/21.
//

#include "PumpHouseAPISecretMgr.hpp"
#include "LogMgr.hpp"

PumpHouseAPISecretMgr::PumpHouseAPISecretMgr(PumpHouseDB* db){
	_db = db;
}

bool PumpHouseAPISecretMgr::apiSecretCreate(string APIkey, string APISecret){
	return _db->apiSecretCreate(APIkey,APISecret );
}

bool PumpHouseAPISecretMgr::apiSecretDelete(string APIkey){
	return _db->apiSecretDelete(APIkey);
}

bool PumpHouseAPISecretMgr::apiSecretGetSecret(string APIkey, string &APISecret){
	return _db->apiSecretGetSecret(APIkey, APISecret);
}

bool PumpHouseAPISecretMgr::apiSecretMustAuthenticate(){
	return _db->apiSecretMustAuthenticate();
}
