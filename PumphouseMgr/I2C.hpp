//
//  I2C.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/10/21.
//

#ifndef I2C_hpp
#define I2C_hpp

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include <time.h>
#include <termios.h>
#include <stdexcept>
#include <string>

class I2C  {
	
public:
	I2C();
	~I2C();
	
	bool begin(uint8_t	devAddr);
 	bool begin(uint8_t	devAddr,  int *error = NULL);

	void stop();
	ssize_t write(const uint8_t*, size_t);
	ssize_t read( void *buf, size_t nbyte);
	
	uint8_t	getDevAddr() {return _devAddr;};
	
private:
	int 			_fd;
	int 			_devAddr;
	bool 			_isSetup;
 
};

 
#endif /* I2C_hpp */
