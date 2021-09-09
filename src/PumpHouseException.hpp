//
//  PumpHouseException.h
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/7/21.
//

#ifndef PumpHouseException_h
#define PumpHouseException_h

#include <stdexcept>

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
 

#endif /* PumpHouseException_h */
