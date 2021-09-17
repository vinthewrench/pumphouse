//
//  MCP3427.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/12/21.
//

#ifndef MCP3427_hpp
#define MCP3427_hpp

#include <stdio.h>
#include "I2C.hpp"


class MCP3427
{
 
public:
	
	typedef enum {
		GAIN_1X = 1,
		GAIN_2X = 2,
		GAIN_4X = 4,
		GAIN_8X = 8
	}ADCGain;

	typedef enum {
		ADC_16_BITS = 16,
		ADC_14_BITS = 14,
		ADC_12_BITS = 12
	}ADCBitDepth;
 
	
	MCP3427();
	~MCP3427();
  
	bool begin(uint8_t deviceAddress = 0x68,  int *error = NULL);
	
	void stop();
	bool isOpen();
	
	bool analogRead(uint16_t &result, uint8_t channel = 0, ADCGain gain=GAIN_1X, ADCBitDepth bitDepth=ADC_16_BITS);
	float convertToVoltage(uint16_t adcVal, ADCGain gain=GAIN_1X, ADCBitDepth bitDepth=ADC_16_BITS);

	uint8_t	getDevAddr();

private:
	
	
	I2C 		_i2cPort;
	bool		_isSetup;

	void delayByBitDepth(ADCBitDepth bitDepth);
	uint8_t computeConfigByte(uint8_t channel, bool continuousConversion, ADCBitDepth bitDepth, ADCGain gain);

};

#endif /* MCP3427_hpp */
