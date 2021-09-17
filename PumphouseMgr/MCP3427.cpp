//
//  MCP3427.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/12/21.
//

#include "MCP3427.hpp"
#include "LogMgr.hpp"

 
MCP3427::MCP3427(){
	_isSetup = false;
}

MCP3427::~MCP3427(){
	stop();
	
}

bool MCP3427::begin(uint8_t deviceAddress,   int * errorOut){
  
	_isSetup = _i2cPort.begin(deviceAddress, errorOut);
//	LOG_INFO("MCP3427(%02x) begin: %s\n", deviceAddress, _isSetup?"OK":"FAIL");
   return _isSetup;
}
 
void MCP3427::stop(){
//	LOG_INFO("TMP102(%02x) stop\n",  _i2cPort.getDevAddr());

	_isSetup = false;
	_i2cPort.stop();
}
 
bool MCP3427::isOpen(){
	return _isSetup;
	
};


uint8_t	MCP3427::getDevAddr(){
	return _i2cPort.getDevAddr();
};


bool MCP3427::analogRead(uint16_t &resultOut, uint8_t channel, ADCGain gain, ADCBitDepth bitDepth){
	
	int16_t adcVal = 0;
	bool conversionDone = false;
	
	if(!isOpen())
		return false;
	
	uint8_t configByte = computeConfigByte(channel, true, bitDepth, gain);
	
	if(_i2cPort.writeByte( configByte)  == -1)
		return false;
	
	// Roughly wait the amount of time we need
	delayByBitDepth(bitDepth);
	
	uint8_t reattempts = 0;
	
	while(!conversionDone && reattempts++ < 8)
	{
		
		uint8_t registerByte[3] = {0,0,0};
		if(_i2cPort.readBytes(registerByte, 3) != 3)
		{
			// Read error, got less than the expected byte
			continue;
		}
		
		adcVal = ((uint16_t)registerByte[0])<<8;
		adcVal |= (uint16_t)((uint16_t)registerByte[1]);
		
		uint8_t confByte = registerByte[2];
		if (confByte & 0x80)
		{
			// /RDY is still high, conversion not done
			usleep(200 * 1000);
			continue;
		}
		else {
			conversionDone = true;
			resultOut = adcVal;
		}
	}
		
	resultOut = adcVal;
	return conversionDone;
}

float MCP3427::convertToVoltage(uint16_t adcVal, ADCGain gain, ADCBitDepth bitDepth)
{
	float divisor = 32767.0;
	float retval;
	
	// Mask off unused bits
	if(ADC_12_BITS == bitDepth)
		divisor = 2047.0;
	else if (ADC_14_BITS == bitDepth)
		divisor = 8191.0;
	
	switch(gain)
	{
		case GAIN_1X:
			retval = (2.048/divisor) * adcVal;
			break;

		case GAIN_2X:
			retval = (1.024/divisor) * adcVal;
			break;

		case GAIN_4X:
			retval = (0.512/divisor) * adcVal;
			break;

		case GAIN_8X:
			retval = (0.256/divisor) * adcVal;
			break;
			
		default:
			retval = 0.0;
			break;
	}

	return(retval);
}

 
void delay(int ms){
	
}

uint8_t MCP3427::computeConfigByte(uint8_t channel, bool continuousConversion, ADCBitDepth bitDepth, ADCGain gain)
{
	uint8_t configByte = 0;
	
	if (channel < 4)
		configByte |= (channel & 0x03) << 5;

	if (continuousConversion)
		configByte |= 0x10;
		
	switch(bitDepth)
	{
		case ADC_12_BITS:
			break; // Do nothing, 00
		case ADC_14_BITS:
			configByte |= 0x04;
			break;
		default:
		case ADC_16_BITS:
			configByte |= 0x08;
			break;
	}

	switch(gain)
	{
		default:
		case GAIN_1X:
			break;

		case GAIN_2X:
			configByte |= 0x01;
			break;

		case GAIN_4X:
			configByte |= 0x02;
			break;

		case GAIN_8X:
			configByte |= 0x03;
			break;
	}
	
	return configByte;
}

void MCP3427::delayByBitDepth(ADCBitDepth bitDepth)
{
	
	switch(bitDepth)
	{
		case ADC_16_BITS:
		default:
			usleep(68 * 1000);
			break;
		case ADC_14_BITS:
			usleep(35 * 1000);
			break;
		case ADC_12_BITS:
			usleep(4 * 1000);
		break;
	}
}

