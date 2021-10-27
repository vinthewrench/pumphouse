//
//  PumphouseCommon.h
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/10/21.
//

#ifndef PumphouseCommon_h
#define PumphouseCommon_h

#include <stdexcept>
 
extern "C" {
void dumpHex(uint8_t* buffer, int length, int offset = 0);
}

class PumpHouseException: virtual public std::runtime_error {
	 
protected:

	 int error_number;               ///< Error Number
	 
public:

	 /** Constructor (C++ STL string, int, int).
	  *  @param msg The error message
	  *  @param err_num Error number
	  */
	 explicit
	PumpHouseException(const std::string& msg, int err_num = 0):
		  std::runtime_error(msg)
		  {
				error_number = err_num;
		  }

	
	 /** Destructor.
	  *  Virtual to allow for subclassing.
	  */
	 virtual ~PumpHouseException() throw () {}
	 
	 /** Returns error number.
	  *  @return #error_number
	  */
	 virtual int getErrorNumber() const throw() {
		  return error_number;
	 }
};
 

#endif /* PumphouseCommon_h */
