//
//  TCA9534.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/29/21.
//


#ifndef TCA9534_hpp
#define TCA9534_hpp

#include <stdio.h>
#include "I2C.hpp"


// cloned from https://github.com/sparkfun/SparkFun_Qwiic_GPIO_Library/blob/master/src/SparkFun_TCA9534.h

class TCA9534
{
 
public:
	TCA9534();
	~TCA9534();
 
	// Address of I2C GPIO (0x20 - 0x27)

	bool begin(uint8_t deviceAddress = 0x27,  int *error = NULL);
	
	void stop();
	bool isOpen();
 
	uint8_t	getDevAddr();

	bool writeConfig(uint8_t val);
	bool writeInvert(uint8_t val);

	bool readInput(uint8_t &val);

	
//	bool pinMode(uint8_t gpioNumber, bool mode);
//	bool pinMode(bool *gpioPinMode);
//
//	bool invertPin(uint8_t gpioNumber, bool inversionMode);
//	bool invertPin(bool *inversionMode);
//
//	bool digitalWrite(uint8_t gpioNumber, bool value);
//	bool digitalWrite(bool *gpioStatus);
//
//	bool digitalRead(uint8_t gpioNumber);
//	uint8_t digitalReadPort(bool *gpioStatus);
//
//	bool readBit(uint8_t regAddr, uint8_t bitAddr);
//	bool writeBit(uint8_t regAddr, uint8_t bitAddr, bool bitToWrite);
//
//	uint8_t readRegister(uint8_t addr);
//	bool writeRegister(uint8_t addr, uint8_t val);

//	bool readBits(uint8_t &bits);

private:
	
	I2C 		_i2cPort;
	bool		_isSetup;

	uint8_t _gpioOutStatus = 0xFF;
	uint8_t _gpioInStatus;
	uint8_t _gpioPinMode = 0xFF;
	uint8_t _inversionStatus = 0;

};

#endif /* TCA9534_hpp */

 
