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

#ifndef _LINUX_I2C_DEV_H
#define _LINUX_I2C_DEV_H

/* /dev/i2c-X ioctl commands.  The ioctl's parameter is always an
 * unsigned long, except for:
 *	- I2C_FUNCS, takes pointer to an unsigned long
 *	- I2C_RDWR, takes pointer to struct i2c_rdwr_ioctl_data
 *	- I2C_SMBUS, takes pointer to struct i2c_smbus_ioctl_data
 */
#define I2C_RETRIES	0x0701	/* number of times a device address should
					be polled when not acknowledging */
#define I2C_TIMEOUT	0x0702	/* set timeout in units of 10 ms */

/* NOTE: Slave address is 7 or 10 bits, but 10-bit addresses
 * are NOT supported! (due to code brokenness)
 */
#define I2C_SLAVE	0x0703	/* Use this slave address */
#define I2C_SLAVE_FORCE	0x0706	/* Use this slave address, even if it
					is already in use by a driver! */
#define I2C_TENBIT	0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit */

#define I2C_FUNCS	0x0705	/* Get the adapter functionality mask */

#define I2C_RDWR	0x0707	/* Combined R/W transfer (one STOP only) */

#define I2C_PEC		0x0708	/* != 0 to use PEC with SMBus */
#define I2C_SMBUS	0x0720	/* SMBus transfer */


/* This is the structure as used in the I2C_SMBUS ioctl call */
struct i2c_smbus_ioctl_data {
	uint8_t read_write;
	uint8_t command;
	uint8_t size;
	union i2c_smbus_data  *data;
};

/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
	struct i2c_msg  *msgs;	/* pointers to i2c_msgs */
	uint32_t nmsgs;			/* number of i2c_msgs */
};

#define  I2C_RDRW_IOCTL_MAX_MSGS	42

#ifdef __KERNEL__
#define I2C_MAJOR	89		/* Device major number		*/
#endif

#endif /* _LINUX_I2C_DEV_H */

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
	
	if (ioctl(_fd, I2C_SLAVE, devAddr) < 0)
	{
		LOG_INFO("Failed to acquire bus access and/or talk to I2C slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		_fd = -1;
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


ssize_t I2C::write(const uint8_t* buf, size_t nbyte){
	return ::write(_fd, buf, nbyte);
};

ssize_t I2C::read( void *buf, size_t nbyte){
	return ::read(_fd, buf, nbyte);
 }
