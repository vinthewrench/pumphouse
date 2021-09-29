//
//  PumpHouse.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/24/21.
//

import Foundation

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
	case date
	case version
	case values
	case props
	case tank
	case unknown
}
 
struct PumpValues {
	var rawTank: Double
	var tankGals: Double
	var tankEmpty: Double
	var tankFull: Double
	var tankPercent: Double
	
	init(){
		self.rawTank = 0
		self.tankGals = 0
		self.tankEmpty = 0
		self.tankFull = 0
		self.tankPercent = 0
	}
}

struct InverterValues {
	var acIn: Double
	var acOut: Double
	var acHz: Double
	var acAmps: Double
	var batVolts: Double
	var batAmps: Double
	var batSOC: Double
	var utilityFail : Bool
	var batteryLow : Bool
 
	var batteryTime: Double
	var batteryConsumed: Double

	var temp1: Double
	var temp2: Double
	var cpuTemp: Double
	
	init(){
		self.acIn = 0
		self.acOut = 0
		self.acHz = 0
		self.acAmps = 0
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
	
	func fetchTankValues(completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
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
						self.fetchTankValues(completionHandler: completionHandler)
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
						
						var pv :PumpValues = .init()
						
						let empty =  self.cachedValues.tankEmpty
						let full =  self.cachedValues.tankFull
						let gals =  self.cachedValues.tankGals
						
						if(tank_raw < empty){ tank_raw = empty}
						if (tank_raw > full) {tank_raw = full}
						let tankP  = ((tank_raw - empty) / (full - empty))
						
						pv.tankEmpty = empty
						pv.tankFull = full
						pv.tankPercent =  100.0  //tankP * 100.00
						pv.rawTank = tank_raw
						pv.tankGals = gals * tankP
						completionHandler(.success(pv))
					}
				}
				else {
					completionHandler(.failure(PumpHouseError.connectFailed))
				}
				
			}
			
			
		}
	}
	
	func fetchInverterValues(completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		fetchData(.values) { result in
			
			if case .success(let v as RESTValuesList) = result {
				
				var iv :InverterValues = .init()
				
				if let s1 = v.values["I_IPFV"]?.value,
					let val = Double(s1) {
					iv.acIn = val
				}
				if let s1 = v.values["I_OPV"]?.value,
					let val = Double(s1) {
					iv.acOut = val
				}
				if let s1 = v.values["I_FREQ"]?.value,
					let val = Double(s1) {
					iv.acHz = val
				}
				if let s1 = v.values["I_OPC"]?.value,
					let val = Double(s1) {
					iv.acAmps = val
				}
				
				if let s1 = v.values["SOC"]?.value,
					let val = Double(s1) {
					iv.batSOC = val
				}
				
				if let s1 = v.values["V"]?.value,
					let val = Double(s1) {
					iv.batVolts = val
				}
				if let s1 = v.values["I"]?.value,
					let val = Double(s1) {
					iv.batAmps = val
				}
				if let s1 = v.values["I_STATUS"]?.value,
					s1.count == 8 {
					iv.utilityFail =  s1[0]=="1"
					iv.batteryLow = s1[1] == "1"
				}
				
				if let s1 = v.values["CPU_TEMP"]?.value,
					let val = Double(s1) {
					iv.cpuTemp = val
	
				}
				if let s1 = v.values["TEMP_0x48"]?.value,
					let val = Double(s1) {
					iv.temp1 = val
				}
	
				if let s1 = v.values["TEMP_0x49"]?.value,
					let val = Double(s1) {
					iv.temp2 = val
				}
				
				if let s1 = v.values["TTG"]?.value,
					let val = Double(s1) {
					iv.batteryTime = val
				}
	
				if let s1 = v.values["CE"]?.value,
					let val = Double(s1) {
					iv.batteryConsumed = val
				}
	 
				completionHandler(.success(iv))
				
			}
			else {
				completionHandler(.failure(PumpHouseError.connectFailed))
			}
			
		}
		
		
	}
	
	
	func fetchData(_ requestType: PumpHouseRequest,
						completionHandler: @escaping (Result<Any?, Error>) -> Void)  {
		
		var urlPath :String = ""
		
		switch(requestType){
		case .status:
			urlPath = "state"
			
		case .date:
			urlPath = "date"
			
		case .version:
			urlPath = "version"
			
		case .values:
			urlPath = "values"
			
		case .props:
			urlPath = "props"
			
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
			else 	if let status = json as? RESTVersion {
				completionHandler(.success(status))
			}
			else 	if let status = json as? RESTDateInfo {
				completionHandler(.success(status))
			}
			else if let restErr = json as? RESTError {
				completionHandler(.success(restErr))
			}
			else if let error = error{
				completionHandler(.failure(error))
			}
		}
	}
}


