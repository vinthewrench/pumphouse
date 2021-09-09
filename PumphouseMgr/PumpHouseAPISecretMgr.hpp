//
//  PumpHouseAPISecretMgr.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/9/21.
//

#ifndef PumpHouseAPISecretMgr_hpp
#define PumpHouseAPISecretMgr_hpp

#include <stdio.h>

#include "ServerCmdQueue.hpp"
#include "PumpHouseDB.hpp"


class PumpHouseAPISecretMgr : public APISecretMgr {

public:
	PumpHouseAPISecretMgr(PumpHouseDB* db);
	
	virtual bool apiSecretCreate(string APIkey, string APISecret);
	virtual bool apiSecretDelete(string APIkey);
	virtual bool apiSecretGetSecret(string APIkey, string &APISecret);
	virtual bool apiSecretMustAuthenticate();
	
private:
	PumpHouseDB* 	 		_db;

};

#endif /* PumpHouseAPISecretMgr_hpp */
