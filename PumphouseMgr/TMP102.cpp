//
//  TMP102.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/10/21.
//

#include "TMP102.hpp"
#include "LogMgr.hpp"


#define TEMPERATURE_REGISTER 0x00
#define CONFIG_REGISTER 0x01
#define T_LOW_REGISTER 0x02
#define T_HIGH_REGISTER 0x03


TMP102::TMP102(){
	_isSetup = false;
}

TMP102::~TMP102(){
	stop();
	
}

bool TMP102::begin(uint8_t deviceAddress,   int * errorOut){
  
	_isSetup = _i2cPort.begin(deviceAddress, errorOut);
	
// 	LOG_INFO("TMP102(%02x) begin: %s\n", deviceAddress, _isSetup?"OK":"FAIL");
 
  return _isSetup;
}
 
void TMP102::stop(){
//	LOG_INFO("TMP102(%02x) stop\n",  _i2cPort.getDevAddr());

	_isSetup = false;
	_i2cPort.stop();
}
 
bool TMP102::isOpen(){
	return _isSetup;
	
};


uint8_t	TMP102::getDevAddr(){
	return _i2cPort.getDevAddr();
};

bool TMP102::readTempF(float& tempOut){
	
	float cTemp;
	bool status = readTempC(cTemp);
	
	if(status)
		tempOut = cTemp *9.0/5.0 + 32.0;
	
	return status;
};


bool TMP102::readTempC(float& tempOut){
	
	if(!isOpen())
		return false;
 
	uint8_t registerByte[2] = {0,0};
	if(_i2cPort.readBytes(TEMPERATURE_REGISTER, registerByte, 2) == -1)
		return false;

	int16_t digitalTemp;
	// Bit 0 of second byte will always be 0 in 12-bit readings and 1 in 13-bit
	if(registerByte[1]&0x01)	// 13 bit mode
	{
		// Combine bytes to create a signed int
		digitalTemp = ((registerByte[0]) << 5) | (registerByte[1] >> 3);
		// Temperature data can be + or -, if it should be negative,
		// convert 13 bit to 16 bit and use the 2s compliment.
		if(digitalTemp > 0xFFF)
		{
			digitalTemp |= 0xE000;
		}
	}
	else	// 12 bit mode
	{
		// Combine bytes to create a signed int
		digitalTemp = ((registerByte[0]) << 4) | (registerByte[1] >> 4);
		// Temperature data can be + or -, if it should be negative,
		// convert 12 bit to 16 bit and use the 2s compliment.
		if(digitalTemp > 0x7FF)
		{
			digitalTemp |= 0xF000;
		}
	}
	// Convert digital reading to analog temperature (1-bit is equal to 0.0625 C)
	
	tempOut = digitalTemp*0.0625;
	
	return true;
}

