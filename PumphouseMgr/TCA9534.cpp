//
//  TCA9534.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/29/21.
//

#include "TCA9534.hpp"
#include "LogMgr.hpp"


 
#define REGISTER_INPUT_PORT          	 	0x00
#define REGISTER_OUTPUT_PORT          			0X01
#define REGISTER_INVERSION						0x02
#define REGISTER_CONFIGURATION        	 		0X03


TCA9534::TCA9534(){
	_isSetup = false;
}

TCA9534::~TCA9534(){
	stop();
	
}

bool TCA9534::begin(uint8_t deviceAddress,   int * errorOut){
  
	_isSetup = _i2cPort.begin(deviceAddress, errorOut);
	
 // 	LOG_INFO("TCA9534(%02x) begin: %s\n", deviceAddress, _isSetup?"OK":"FAIL");
 
  return _isSetup;
}
 
void TCA9534::stop(){
//	LOG_INFO("TCA9534(%02x) stop\n",  _i2cPort.getDevAddr());

	_isSetup = false;
	_i2cPort.stop();
}
 
bool TCA9534::isOpen(){
	return _isSetup;
	
};


uint8_t	TCA9534::getDevAddr(){
	return _i2cPort.getDevAddr();
};


bool TCA9534::writeConfig(uint8_t val){
	
	if(!isOpen())
			return false;

	uint8_t registerByte[2] = {REGISTER_CONFIGURATION, val};

	ssize_t retval = _i2cPort.writeBytes(registerByte, 2);
//	printf("writeConfig(%02X) = %zd\n",val, retval);

 	if(retval == -1)
		return false;

	return true;
}


bool TCA9534::writeInvert(uint8_t val){
	if(!isOpen())
			return false;
	
	uint8_t registerByte[2] = {REGISTER_INVERSION, val};

	ssize_t retval = _i2cPort.writeBytes(registerByte, 2);
//	printf("writeInvert(%02X) = %zd\n",val, retval);

	if(retval == -1)
		return false;
 
	return true;

}

bool TCA9534::readInput(uint8_t &val){
	
	if(!isOpen())
			return false;
 
	uint8_t registerByte[1] = {0};
	if(_i2cPort.readBytes(REGISTER_INPUT_PORT, registerByte, 1) == -1)
		return false;

	val = registerByte[0];
 
	return true;
}
