//
//  I2C.cpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/10/21.
//

#include "I2C.hpp"
#include <errno.h>
#include <sys/ioctl.h>                                                  // Serial Port IO Controls
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "LogMgr.hpp"

#ifndef I2C_SLAVE
#define I2C_SLAVE	0x0703	/* Use this slave address */
#endif /* I2C_SLAVE */

#ifndef I2C_BUS_DEV_FILE_PATH
#define I2C_BUS_DEV_FILE_PATH "/dev/i2c-1"
#endif /* I2C_SLAVE */




I2C::I2C(){
	_isSetup = false;
	_fd = -1;
	_devAddr = 00;
}


I2C::~I2C(){
	stop();
	
}
 
bool I2C::begin(uint8_t	devAddr){
	int error = 0;

	return begin(devAddr, &error);
}


bool I2C::begin(uint8_t	devAddr,   int * errorOut){
	static const char *ic2_device = "/dev/i2c-1";
 
	_isSetup = false;
	
 	_fd = open( ic2_device, O_RDWR);
	
	if(_fd == -1){
		if(errorOut) *errorOut = errno;
		return false;
	}
	
	if (isAvailable())
	{
		LOG_INFO("Failed to acquire I2C bus access and/or talk to I2C slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		_fd = -1;
		if(errorOut) *errorOut = errno;
		return false;
	}
 
	_isSetup = true;
	_devAddr = devAddr;
	
	return _isSetup;
}


void I2C::stop(){
	
	if(_isSetup){
		close(_fd);
		_devAddr = 00;
	}
	
	_isSetup = false;
}

bool I2C::isAvailable(){

	if(!_isSetup)
		return false;
	
	if (::ioctl(_fd, I2C_SLAVE, _devAddr) < 0)
	{
	//	LOG_INFO("Failed to acquire bus access and/or talk to I2C slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		_fd = -1;
		return false;
	}
	
	return true;
}


ssize_t I2C::writeBytes(uint8_t regAddr, const uint8_t* buf, size_t nbyte){
	
	ssize_t count = 0;

	if(!_isSetup)
		return -1;

	if (::ioctl(_fd, I2C_SLAVE, _devAddr) < 0)
	{
		LOG_ERROR("Failed to select I2C  device(%02X): %s\n", _devAddr,strerror(errno));
		return -1;
	}
 
	if (::write(_fd, &regAddr, 1) != 1) {
		LOG_ERROR( "Failed to write reg(%02x) on device(%02X) : %s\n", regAddr, _devAddr,strerror(errno));
		return(-1);
	}
	
	count =  ::write(_fd, buf, nbyte);
	if (count < 0) {
			LOG_ERROR( "Failed to write device(%02x): %s\n", regAddr, strerror(errno));
		return(-1);
	} else if (count != nbyte) {
		LOG_ERROR( "Short write from device(%02x): expected %d, got %d \n", regAddr, nbyte, count);
		return(-1);
	}
	
	return count;
}


ssize_t I2C::writeByte(uint8_t regAddr, const uint8_t data){
	return writeBytes(regAddr, &data, data);
}

ssize_t I2C::writeByte(const uint8_t data){
	
	ssize_t count = 0;

	if(!_isSetup)
		return -1;

	if (::ioctl(_fd, I2C_SLAVE, _devAddr) < 0)
	{
		LOG_ERROR("Failed to select I2C  device(%02X): %s\n", _devAddr,strerror(errno));
		return -1;
	}
 
	count = ::write(_fd, &data, 1);
	if (count != 1) {
			LOG_ERROR( "Failed to write byte to device(%02x): %s\n", _devAddr, strerror(errno));
		return(-1);
	}
	
	return count;
}


ssize_t I2C::readBytes(void *buf, size_t nbyte){
	
	ssize_t count = 0;

	if(!_isSetup)
		return -1;

	if (::ioctl(_fd, I2C_SLAVE, _devAddr) < 0)
	{
		LOG_ERROR("Failed to select I2C  device(%02X): %s\n", _devAddr,strerror(errno));
		return -1;
	}
 
	count =  ::read(_fd, buf, nbyte);
	if (count < 0) {
			LOG_ERROR( "Failed to read device(%02x): %s\n", _devAddr, strerror(errno));
		return(-1);
	} else if (count != nbyte) {
		LOG_ERROR( "Short read from device(%02x): expected %d, got %d \n", _devAddr, nbyte, count);
		return(-1);
	}
	
	return count;
}

ssize_t I2C::readByte(void *buf){
	return readBytes(buf, 1);
}

ssize_t I2C::readByte(uint8_t regAddr, void *buf) {
	 return readBytes(regAddr,  buf, 1);
}


ssize_t I2C::readBytes(uint8_t regAddr, void *buf, size_t nbyte){
	
	ssize_t count = 0;

	if(!_isSetup)
		return -1;

	if (::ioctl(_fd, I2C_SLAVE, _devAddr) < 0)
	{
		LOG_ERROR("Failed to select I2C  device(%02X): %s\n", _devAddr,strerror(errno));
		return -1;
	}
 
	if (::write(_fd, &regAddr, 1) != 1) {
		LOG_ERROR( "Failed to write reg(%02x) on device(%02X) : %s\n", regAddr, _devAddr,strerror(errno));
		return(-1);
	}
	
	count =  ::read(_fd, buf, nbyte);
	if (count < 0) {
			LOG_ERROR( "Failed to read device(%02x): %s\n", regAddr, strerror(errno));
		return(-1);
	} else if (count != nbyte) {
		LOG_ERROR( "Short read from device(%02x): expected %d, got %d \n", regAddr, nbyte, count);
		return(-1);
	}
	
	return count;
}
