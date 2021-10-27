//
//  PumpHouse.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/24/21.
//

import Foundation

struct keyGroupEntry {
	var title = String()
	var keys = [String]()
}

class PumpValueFormatter: Formatter {
	
	var units = RESTschemaUnits.UNKNOWN
	
	func unitSuffixForUnits(forUnits unit: RESTschemaUnits) -> String {
		
		var suffix = String()
		
		switch unit {
		case .MILLIVOLTS,
			  .VOLTS:
			suffix = "V"
			break
			
		case .MILLIAMPS, .AMPS:
			suffix = "A"
			break
			
		case .MAH:
			suffix = "Ahrs"
			break
			
		case .DEKAWATTHOUR:
			suffix = "kWh"
			break
			
		case .DEGREES_C:
			suffix = "ºC"
			break
			
		case .PERMILLE,
			  .PERCENT:
			suffix = "%"
			break
			
		case .WATTS:
			suffix = "W"
			break
			
		case .SECONDS:
			suffix = "Seconds"
			break
			
		case .MINUTES:
			suffix = "Minutes"
			break
			
		case .HERTZ:
			suffix = "Hz"
			break
			
		default:
			break
		}
		
		if !suffix.isEmpty {
			suffix = " " + suffix
		}
		
		return suffix
	}
	
	
	func normalizedDoubleForValue(in string: String, forUnits unit: RESTschemaUnits) -> Double {
		var  retVal: Double = 0
		
		if let val = Double(string) {
			
			switch unit {
			case .MILLIVOLTS,
				  .MILLIAMPS,
				  .MAH:
				retVal = val / 1000;
				break
				
			case .DEKAWATTHOUR:
				retVal = val / 100;
				break;
				
			case .PERMILLE:
				retVal = val / 10;
				break;
				
			case .PERCENT,
				  .DEGREES_C,
				  .WATTS,
				  .VOLTS,
				  .AMPS,
				  .SECONDS,
				  .MINUTES,
				  .HERTZ:
				retVal = val;
				
			default:
				break
			}
		}
		return retVal
	}
	
	func normalizedIntForValue(in string: String, forUnits unit: RESTschemaUnits) -> Int {
		var  retVal: Int = 0
			
		switch unit {
 
		case .MINUTES,
			  .SECONDS,
			  .BOOL,
			  .INT:
			if let val = Int(string) {
				retVal = val
			}
			break
			
		case .VE_PRODUCT:
			
			let range = NSRange(location: 0, length: string.utf16.count)
			let regex = try! NSRegularExpression(pattern: "^0?[xX][0-9a-fA-F]{4}$")
			if(regex.firstMatch(in: string, options: [], range: range) != nil){
				if let intVal = Int(string, radix: 16) {
					retVal = intVal
				}
			}
			break;
			
		default:
			break
		}
		
		return retVal
	}
	
	func displayStringForValue(in string: String, forUnits unit: RESTschemaUnits) -> String {
		
		var retVal = string
 		let suffix =  self.unitSuffixForUnits(forUnits: units)
		
		switch unit {
		case .MILLIVOLTS,
			  .MILLIAMPS,
			  .MAH,
			  .VOLTS,
			  .AMPS,
			  .HERTZ,
			  .WATTS,
			  .DEKAWATTHOUR,
			  .PERMILLE,
			  .PERCENT:
		
			let  val = normalizedDoubleForValue(in: string, forUnits: units)
			retVal = String(format: "%3.2f%@", val, suffix )
 			break

		case .BOOL:
			let  val = normalizedIntForValue(in: string, forUnits: units)
			if(val == 0){
				retVal = "No"
			}  else if(val == 1){
				retVal = "Yes"
			}
			break
	
		case .DEGREES_C:
			let  val = normalizedDoubleForValue(in: string, forUnits: units)
			let  tempF =  val * 9.0 / 5.0 + 32.0
			retVal = String(format: "%3.2f%@", tempF, "°F")
			break
			
		case .SECONDS,
			  .MINUTES,
			  .INT:
				let  val = normalizedIntForValue(in: string, forUnits: units)
			if(val == -1){
				retVal = "Infinite";
			}
			else
			{
				retVal = String(format: "%d %@", val, suffix)
			}
			break
			
		case .VE_PRODUCT:
				let  val = normalizedIntForValue(in: string, forUnits: units)
			if(val == 0xA389){
				retVal = "SmartShunt 500A/50mV";
			}
	
			break
			
		default:
			break
		}
		
		
		return retVal
	}

	
	func replaceValueWithString(in string: String, forUnits unit: RESTschemaUnits) -> String {
		var rawString = string
		
		rawString = displayStringForValue(in: string, forUnits: 	unit)
			
	//		String(format: "%@%@", rawString, self.unitSuffixForUnits(forUnits: units))
		
		return rawString
	}
	
	
	override func string(for obj: Any?) -> String? {
		
		if let string = obj as? String {
			return replaceValueWithString(in: string , forUnits: units)
		}
		return nil
	}
}


enum PumpHouseError: Error {
	case connectFailed
	case invalidState
	case invalidURL
	case restError
	case internalError
	
	case unknown
}

enum PumpHouseRequest: Error {
	case status
	case values
	case props
	case schema
	case unknown
}
 
enum DeviceState: Int {
	case unknown = 0
	case disconnected
	case connected
	case error
	case timeout
}

struct PumpHouseValues {
	
	// Tank Values
	var rawTank: Double
	var tankGals: Double
	var tankEmpty: Double
	var tankFull: Double
	var tankPercent: Double
	
	// inverter / charger values
	var acIn: Double
	var acOut: Double
	var acHz: Double
	var invLoad: Double
	var batVolts: Double
	var batAmps: Double
	var batSOC: Double
	var utilityFail : Bool
	var batteryLow : Bool
 
	var inverterState: DeviceState
	var inverterLastTime : Int
	var batteryState: DeviceState
	var batteryLastTime : Int

	var batteryTime: Double
	var batteryConsumed: Double

	var temp1: Double
	var temp2: Double
	var cpuTemp: Double

	init(){
		self.inverterState = .unknown
		self.batteryState = .unknown
		self.inverterLastTime = 0
		self.batteryLastTime = 0
		
		self.rawTank = 0
		self.tankGals = 0
		self.tankEmpty = 0
		self.tankFull = 0
		self.tankPercent = 0
		
		self.acIn = 0
		self.acOut = 0
		self.acHz = 0
		self.invLoad = 0
		self.batAmps = 0
		self.batSOC = 0
		self.batVolts = 0
		self.temp1 = 0
		self.temp2 = 0
		self.cpuTemp = 0
		self.batteryTime = 0
		self.batteryConsumed = 0
		self.utilityFail = false
		self.batteryLow = false
	}
}

public class PumpHouse {
	
	var isValid: Bool = false;
	
	static let shared: PumpHouse = {
		let instance = PumpHouse()
		// Setup code
		return instance
	}()
	
	
	struct cachedValues_t {
		var tankGals: Double
		var tankEmpty: Double
		var tankFull: Double
		var isValid: Bool
		
		init() {
			self.tankGals = 0
			self.tankEmpty = 0
			self.tankFull = 0
			self.isValid = false
		}
	}
	
	var cachedValues :cachedValues_t 
	
	private init(){
		cachedValues = .init()
	}
	
	
	func fetchValues(completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		// get the list of cached properties first
		if(!cachedValues.isValid){
			
			fetchData(.props) { result in
				
				if case .success(let props as RESTProperties) = result {
					var cnt = 0
					if let s1 = props.properties["prop-tank-empty"] {
						self.cachedValues.tankEmpty = Double(s1) ?? 0.0
						cnt+=1
					}
					if let s1 = props.properties["prop-tank-full"] {
						self.cachedValues.tankFull = Double(s1) ?? 0.0
						cnt+=1
					}
					if let s1 = props.properties["prop-tank-gals"] {
						self.cachedValues.tankGals = Double(s1) ?? 0.0
						cnt+=1
					}
					
					self.cachedValues.isValid = cnt == 3
					
					if(self.cachedValues.isValid) {
						self.fetchValues(completionHandler: completionHandler)
					}
					else {
						completionHandler(.failure(PumpHouseError.internalError))
					}
					
				}
				else {
					completionHandler(.failure(PumpHouseError.connectFailed))
					
				}
			}
		}
		else {
			fetchData(.values) { result in
				
				if case .success(let v as RESTValuesList) = result {
					
					if let s1 = v.values["TANK_RAW"]?.value,
						var tank_raw = Double(s1) {
						
						var phv :PumpHouseValues = .init()
						
						phv.inverterState = DeviceState(rawValue:v.inverter) ?? .unknown
						phv.batteryLastTime = v.batteryLastTime
						
						phv.batteryState = DeviceState(rawValue:v.battery) ?? .unknown
 
						let empty =  self.cachedValues.tankEmpty
						let full =  self.cachedValues.tankFull
						let gals =  self.cachedValues.tankGals
						
						if(tank_raw < empty){ tank_raw = empty}
						if (tank_raw > full) {tank_raw = full}
						let tankP  = ((tank_raw - empty) / (full - empty))
						
						phv.tankEmpty = empty
						phv.tankFull = full
						phv.tankPercent =  tankP * 100.00
						phv.rawTank = tank_raw
						phv.tankGals = gals * tankP
						
						if let s1 = v.values["I_IPFV"]?.value,
							let val = Double(s1) {
							phv.acIn = val
						}
						if let s1 = v.values["I_OPV"]?.value,
							let val = Double(s1) {
							phv.acOut = val
						}
						if let s1 = v.values["I_FREQ"]?.value,
							let val = Double(s1) {
							phv.acHz = val
						}
						if let s1 = v.values["I_OPC"]?.value,
							let val = Double(s1) {
							phv.invLoad = val
						}
						
						if let s1 = v.values["SOC"]?.value,
							let val = Double(s1) {
							phv.batSOC = val
						}
						
						if let s1 = v.values["V"]?.value,
							let val = Double(s1) {
							phv.batVolts = val
						}
						if let s1 = v.values["I"]?.value,
							let val = Double(s1) {
							phv.batAmps = val
						}
						if let s1 = v.values["I_STATUS"]?.value,
							s1.count == 8 {
							phv.utilityFail =  s1[0]=="1"
							phv.batteryLow = s1[1] == "1"
						}
	
						if let s1 = v.values["CPU_TEMP"]?.value,
							let val = Double(s1) {
							phv.cpuTemp = val
							
						}
						if let s1 = v.values["TEMP_0x48"]?.value,
							let val = Double(s1) {
							phv.temp1 = val
						}
						
						if let s1 = v.values["TEMP_0x49"]?.value,
							let val = Double(s1) {
							phv.temp2 = val
						}
						
						if let s1 = v.values["TTG"]?.value,
							let val = Double(s1) {
							phv.batteryTime = val
						}
						
						if let s1 = v.values["CE"]?.value,
							let val = Double(s1) {
							phv.batteryConsumed = val
						}
						completionHandler(.success(phv))
					}
				}
				else {
					completionHandler(.failure(PumpHouseError.connectFailed))
				}
				
			}
		}
	}
	
	func fetchData(_ requestType: PumpHouseRequest,
						completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		var urlPath :String = ""
		
		switch(requestType){
		case .status:
			urlPath = "state"
			
		case .values:
			urlPath = "values"
			
		case .props:
			urlPath = "props"
			
		case .schema:
			urlPath = "schema"
			
		default:
			break;
		}
		
		PHServerManager.shared.RESTCall(urlPath: urlPath,
												  headers:nil,
												  queries: nil) { (response, json, error)  in
			
			if (json == nil) {
				completionHandler(.failure(PumpHouseError.connectFailed))
			}
			else 	if let values = json as? RESTValuesList {
				completionHandler(.success(values))
			}
			else 	if let props = json as? RESTProperties {
				completionHandler(.success(props))
			}
			else 	if let status = json as? RESTStatus {
				completionHandler(.success(status))
			}
			else 	if let schema = json as? RESTSchemaList {
				completionHandler(.success(schema))
			}
		else if let restErr = json as? RESTError {
				completionHandler(.success(restErr))
			}
			else if let error = error{
				completionHandler(.failure(error))
			}
		}
	}
	
	func fetchHistory(_ key: String,
							completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		let urlPath :String = "history/" + key
		
		let headers = ["limit" : "500"]
		
		PHServerManager.shared.RESTCall(urlPath: urlPath,
												  headers:headers,
												  queries: nil) { (response, json, error)  in
						
			if let restHist = json as? RESTHistory {
				completionHandler(.success(restHist))
			}
			else if let restErr = json as? RESTError {
				completionHandler(.success(restErr))
			}
			else if let error = error{
				completionHandler(.failure(error))
			}
		}
	}
	
	
	func fetchEvents( completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		let urlPath :String = "events"
		
		PHServerManager.shared.RESTCall(urlPath: urlPath,
												  headers:nil,
												  queries: nil) { (response, json, error)  in
						
			if let restEvents = json as? RESTEvents {
				completionHandler(.success(restEvents))
			}
			else if let restErr = json as? RESTError {
				completionHandler(.success(restErr))
			}
			else if let error = error{
				completionHandler(.failure(error))
			}
			else {
				let error = PumpHouseError.internalError
				completionHandler(.failure(error))
			}
		}
	}

	
	
	func groupValueKeys(_ keys: [String]) -> [keyGroupEntry]{
		
		var result:[keyGroupEntry] = []
		
		result.append( keyGroupEntry(title: "Inverter" ))
		result.append( keyGroupEntry(title: "SmartShunt" ))
		result.append( keyGroupEntry(title: "Pump" ))
		result.append( keyGroupEntry(title: "Temperature" ))
		
		let pumpKeys = ["TANK", "TANK_RAW"]
		
		let tempKeys = ["TEMP_0x48","TEMP_0x49","CPU_TEMP"]
	
		let shuntKeys = [ "AR", "Alarm", "CE", "DM", "FW", "H1", "H10", "H11", "H12", "H15", "H16", "H17", "H18", "H2", "H3", "H4", "H5", "H6", "H7", "H8", "H9", "I", "MON", "P", "PID", "SOC", "TTG", "V", "VM"]
		
		let inverterKeys = [ "I_BT",  "I_BV",  "I_FREQ",  "I_IPFV",  "I_IPV",  "I_OPC",  "I_OPV",  "I_STATUS"]
		
		let keys = Array(keys).sorted(by: <)
		
		for key in keys {
			
			if(inverterKeys.contains(key)){
				result[0].keys.append(key)
			}else if(shuntKeys.contains(key)){
				result[1].keys.append(key)
			}else if(pumpKeys.contains(key)){
				result[2].keys.append(key)
			}else if(tempKeys.contains(key)){
				result[3].keys.append(key)
			}
		}
		
		return result
	}
}

