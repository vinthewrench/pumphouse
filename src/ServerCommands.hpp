//
//  ServerCommands.h
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/9/21.
//

#ifndef ServerCommands_h
#define ServerCommands_h


constexpr string_view NOUN_VERSION		 		= "version";
constexpr string_view NOUN_DATE		 			= "date";
constexpr string_view NOUN_STATUS		 		= "status";
constexpr string_view NOUN_SCHEMA	 			= "schema";
constexpr string_view NOUN_VALUES	 			= "values";

constexpr string_view JSON_ARG_DATE			= "date";
constexpr string_view JSON_ARG_VERSION		= "version";
constexpr string_view JSON_ARG_TIMESTAMP		= "timestamp";

constexpr string_view JSON_ARG_HAS_SHUNT		= "hasShunt";
constexpr string_view JSON_ARG_HAS_INVERTER		= "hasInverter";
 
constexpr string_view JSON_ARG_STATE			= "state";
constexpr string_view JSON_ARG_STATESTR		= "stateString";
constexpr string_view JSON_ARG_CPU_TEMP		= "cpuTemp";

constexpr string_view JSON_ARG_SCHEMA			= "schema";
constexpr string_view JSON_ARG_VALUES			= "values";

constexpr string_view JSON_ARG_NAME			= "name";
constexpr string_view JSON_ARG_TRACKING		= "tracking";
constexpr string_view JSON_ARG_UNITS			= "units";



void registerCommandsLineFunctions();
void registerServerNouns();

#endif /* ServerCommands_h */
