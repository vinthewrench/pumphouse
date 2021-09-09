//
//  ServerCmdValidators.hpp
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/9/21.
//
 #ifndef ServerCmdValidators_hpp
#define ServerCmdValidators_hpp

#include <stdio.h>
#include <string>
#include "json.hpp"
#include <regex>
#include <any>
#include "Utils.hpp"
#include <iostream>

using namespace std;
using namespace nlohmann;

class ServerCmdArgValidator {

public:
	ServerCmdArgValidator() {};
	virtual ~ServerCmdArgValidator() {};

	virtual bool validateArg( string_view arg)	{ return false; };
	
	virtual bool createJSONentry( string_view arg, json &j) { return false; };
	
	virtual bool containsKey(string_view key, const json &j) {
		return (j.contains(key));
	}
	
	virtual bool getStringFromJSON(string_view key, const json &j, string &result) {
		string k = string(key);
		
		if( j.contains(k) && j.at(k).is_string()){
			result = j.at(k);
			return true;
		}
		return false;
	}
	virtual bool getStringFromMap(string_view key, const map<string, string> &m, string &result) {
		string k = string(key);
		
		if( m.count(k)){
			result = m.at(k);
			return true;
		}
		return false;
	}

 
	virtual bool getByteFromJSON(string_view key, const json &j, uint8_t &result) {
		if( j.contains(key)) {
			string k = string(key);
		
			if( j.at(k).is_string()){
				string str = j.at(k);
				
				char* p;
				long val = strtol(str.c_str(), &p, 0);
				if(*p == 0){
					result = (uint8_t)val;
					return  true;;
				}
			}
			else if( j.at(k).is_number()){
				result = (uint8_t) j.at(k);
				return true;
			}
		}
		return  false;
	}

	virtual bool getHexByteFromJSON(string_view key, const json &j, uint8_t &result) {
		if( j.contains(key)) {
			string k = string(key);
			
			if( j.at(k).is_string()){
				string str = j.at(k);
				
				uint8_t val = 0;
		
				if( regex_match(string(str), std::regex("^[A-Fa-f0-9]{2}$"))
					&& ( std::sscanf(str.c_str(), "%hhx", &val) == 1)){
					result = val;
					return  true;
				}
				else if( regex_match(string(str), std::regex("^0?[xX][0-9a-fA-F]{2}$"))
						  && ( std::sscanf(str.c_str(), "%hhx", &val) == 1)){
					result = val;
					return  true;
				}
			}
		}
		return  false;
	}

	
	virtual bool getIntFromJSON(string_view key, const json &j, int &result) {
		if( j.contains(key)) {
			string k = string(key);
		
			if( j.at(k).is_string()){
				string str = j.at(k);
				
				char* p;
				long val = strtol(str.c_str(), &p, 0);
				if(*p == 0){
					result = (int)val;
					return  true;;
				}
			}
			else if( j.at(k).is_number()){
				result = j.at(k);
				return true;
			}
		}
		return  false;
	}
 
	virtual bool getLongIntFromJSON(string_view key, const json &j, long &result) {
		if( j.contains(key)) {
			string k = string(key);
		
			if( j.at(k).is_string()){
				string str = j.at(k);
				
				char* p;
				long val = strtol(str.c_str(), &p, 0);
				if(*p == 0){
					result = val;
					return  true;;
				}
			}
			else if( j.at(k).is_number()){
				result = j.at(k);
				return true;
			}
		}
		return  false;
	}
	
	
	  virtual bool getDoubleFromJSON(string_view key, const json &j, double &result) {
		  if( j.contains(key)) {
			  string k = string(key);
		  
			  if( j.at(k).is_string()){
				  string str = j.at(k);
				  
				  char* p;
				  double val = strtod(str.c_str(), &p);
				  if(*p == 0){
					  result = val;
					  return  true;;
				  }
			  }
			  else if( j.at(k).is_number()){
				  auto num = j.at(k);
				  double dd = (double) num;
				  result = dd;
				  return true;
			  }
		  }
		  return  false;
	  }
 
	virtual bool getBoolFromJSON(string_view key, const json &j, bool &result) {
		if( j.contains(key)) {
			string k = string(key);
		
//			if( j.at(k).is_number()){
//				string str = j.at(k);
//
//				char* p;
//				long val = strtol(str.c_str(), &p, 0);
//				if(*p == 0){
//					result = (int)val;
//					return  true;;
//				}
//			}
//			else
				if( j.at(k).is_boolean()){
				result = j.at(k);
				return true;
			}
		}
		return  false;
	}
};
 

class StringArgValidator : public ServerCmdArgValidator {
public:
 
	virtual bool getvalueFromJSON(string_view key, const json &j, any &result){
		string str;
		if(getStringFromJSON(key, j, str)){
			result = str;
			return true;
		}
		return false;
	}
};


#endif /* ServerCmdValidators_hpp */
