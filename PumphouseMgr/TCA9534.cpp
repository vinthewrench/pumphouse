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
	
  	LOG_INFO("TCA9534(%02x) begin: %s\n", deviceAddress, _isSetup?"OK":"FAIL");
 
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


bool TCA9534::pinMode(uint8_t gpioNumber, bool mode)
{
	_gpioPinMode &= ~(1 << gpioNumber);
	_gpioPinMode |= mode << gpioNumber;
	return writeRegister(REGISTER_CONFIGURATION, _gpioPinMode);
}

bool TCA9534::pinMode(bool *gpioPinMode)
{
	_gpioPinMode = 0;
	for (uint8_t bitPosition = 0; bitPosition < 8; bitPosition++)
	{
		_gpioPinMode |= gpioPinMode[bitPosition] << bitPosition;
	}
	return writeRegister(REGISTER_CONFIGURATION, _gpioPinMode);
}

bool TCA9534::invertPin(uint8_t gpioNumber, bool inversionMode)
{
	_inversionStatus &= ~(1 << gpioNumber);
	_inversionStatus |= inversionMode << gpioNumber;
	return writeRegister(REGISTER_INVERSION, _inversionStatus);
}

bool TCA9534::invertPin(bool *inversionMode)
{
	_inversionStatus = 0;
	for (uint8_t bitPosition = 0; bitPosition < 8; bitPosition++)
	{
		_inversionStatus |= inversionMode[bitPosition] << bitPosition;
	}
	return writeRegister(REGISTER_INVERSION, _inversionStatus);
}


bool TCA9534::digitalWrite(uint8_t gpioNumber, bool value)
{
	_gpioOutStatus &= ~(1 << gpioNumber);
	_gpioOutStatus |= value << gpioNumber;
	return writeRegister(REGISTER_OUTPUT_PORT, _gpioOutStatus);
}

bool TCA9534::digitalWrite(bool *gpioOutStatus)
{
	_gpioOutStatus = 0;
	for (uint8_t bitPosition = 0; bitPosition < 8; bitPosition++)
	{
		_gpioOutStatus |= gpioOutStatus[bitPosition] << bitPosition;
	}
	return writeRegister(REGISTER_OUTPUT_PORT, _gpioOutStatus);
}

bool TCA9534::digitalRead(uint8_t gpioNumber)
{
	_gpioInStatus = readRegister(REGISTER_INPUT_PORT);
	return _gpioInStatus & (1 << gpioNumber);
}

uint8_t TCA9534::digitalReadPort(bool *gpioInStatus)
{
	
	_gpioInStatus = readRegister(REGISTER_INPUT_PORT);
	for (uint8_t bitPosition = 0; bitPosition < 8; bitPosition++)
	{
		gpioInStatus[bitPosition] = _gpioInStatus & (1 << bitPosition);
	}
	return _gpioInStatus;
}

bool TCA9534::readBit(uint8_t regAddr, uint8_t bitAddr)
{
	return ((readRegister(regAddr) & (1 << bitAddr)) >> bitAddr);
}

bool TCA9534::writeBit(uint8_t regAddr, uint8_t bitAddr, bool bitToWrite)
{
	_gpioOutStatus &= ~(1 << bitAddr);
	_gpioOutStatus |= bitToWrite << bitAddr;
	return writeRegister(regAddr, _gpioOutStatus);
}

uint8_t TCA9534::readRegister(uint8_t addr)
{
	
	if(!isOpen())
		return false;
 
	uint8_t registerByte[1] = {0};
	if(_i2cPort.readBytes(addr, registerByte, 1) == -1)
		return false;
	return registerByte[0];
}

bool TCA9534::writeRegister(uint8_t addr, uint8_t val)
{
	if(!isOpen())
		return false;
	
	if(_i2cPort.writeByte( addr, val)  == -1)
		return false;
	
	return(true);
}
