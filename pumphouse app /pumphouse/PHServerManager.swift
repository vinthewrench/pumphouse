//
//  PHServerManager.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/19/21.
//

import Foundation


class RESTErrorInfo: Codable {
	let code: Int
	let message: String
	let detail: String?
	let cmdDetail: String?
}

class RESTError: Codable {
	let error: RESTErrorInfo
}
 
enum RESTdeviceStatus : Int {
	case unknown = 0
	case disconnected
	case connected
	case error
	case timeout
}


struct RESTStatus: Codable {
	var state: Int
	var inverter: Int
	let inverterLastTime : Int
	var battery: Int
	let batteryLastTime : Int
	var cpuTemp: Double?
	var stateString: String?
	var buildtime: String
	var version: String
	let date: String
	let uptime: Int
	let os_sysname : String
	let os_nodename : String
	let os_release : String
	let os_version : String
	let os_machine : String

	enum CodingKeys: String, CodingKey {
		case state   		= "state"
		case inverter 		= "inverter"
		case battery 		= "battery"
		case inverterLastTime 		= "inverter.lastTime"
		case batteryLastTime 		= "battery.lastTime"
		case cpuTemp  		= "cpuTemp"
		case stateString  	= "stateString"
		case buildtime 		= "buildtime"
		case version 		= "version"
		case date 				= "date"
		case uptime 			= "uptime"
		case os_sysname 		= "os.sysname"
		case os_nodename 	= "os.nodename"
		case os_release 		= "os.release"
		case os_version 		= "os.version"
		case os_machine 		= "os.machine"
	}

}

struct RESTValueDetails: Codable {
	var display: String
	var time:  Double
 	var value: String
	
	enum CodingKeys: String, CodingKey {
		case display = "display"
		case time = "time"
 		case value = "value"
	}
}
 
enum RESTschemaTracking : Int {
	case ignoreValue = 0
	case staticValue = 1
	case trackedValue = 2
	case untrackedValue = 3
}
 
enum RESTschemaUnits : Int {
	case INVALID = 0
	case BOOL				// Bool ON/OFF
	case INT			// Int
	case MAH				// mAh milliAmp hours
	case PERMILLE		// (per thousand) sign ‰
	case PERCENT		// (per hundred) sign ‰
	case DEKAWATTHOUR	 // .01kWh
	case WATTS			// W
	case MILLIVOLTS		// mV
	case MILLIAMPS		// mA
	case SECONDS			// sec
	case MINUTES			// mins
	case DEGREES_C		// degC
	case VOLTS			// V
	case HERTZ			// Hz
	case AMPS				// A
	case BINARY			// Binary 8 bits 000001
	case VE_PRODUCT		// VE.PART
	case STRING			// string
	case IGNORE
	case UNKNOWN
}

struct RESTSchemaDetails: Codable {
	var name: String
	var suffix:  String
	var tracking: Int
	var units: Int

	enum CodingKeys: String, CodingKey {
		case name = "name"
		case suffix = "suffix"
		case tracking = "tracking"
		case units = "units"
	}
	init() {
		name = String()
		suffix = String()
		tracking = RESTschemaTracking.ignoreValue.rawValue
		units = RESTschemaUnits.UNKNOWN.rawValue
	}
}

struct RESTSchemaList: Codable {
	var schema:  Dictionary<String, RESTSchemaDetails>

	enum CodingKeys: String, CodingKey {
		case schema = "schema"
	}
}


struct RESTProperties: Codable {
	var properties:  Dictionary<String, String>
  }

struct RESTValuesList: Codable {
	var values:  Dictionary<String, RESTValueDetails>
	var ETag: 	Int?
	
	var inverter: Int
	let inverterLastTime : Int
	var battery: Int
	let batteryLastTime : Int

	
	enum CodingKeys: String, CodingKey {
		case values = "values"
		case ETag = "ETag"
		case inverter 		= "inverter"
		case battery 		= "battery"
		case inverterLastTime 		= "inverter.lastTime"
		case batteryLastTime 		= "battery.lastTime"
	}
  }

struct RESTTimeSpanItem: Codable {
	var time:  		Double
	var durration:  TimeInterval
	var value: 		String
}

struct RESTHistoryItem: Codable {
	var time:  Double
	var value: String
	
	enum CodingKeys: String, CodingKey {
		case time = "time"
		case value = "value"
	}
}

struct RESTHistory: Codable {
	var values:  Array< RESTHistoryItem>
	enum CodingKeys: String, CodingKey {
		case values = "values"
	}
 
	func timeLine() -> Array<RESTTimeSpanItem> {
		
		var timeline:Array<RESTTimeSpanItem> = []
		
		let items = self.values.reversed()
		let now = Date().timeIntervalSince1970
		var lastValue:String = "<no value>"
		var lastTime:Double = 0;
		
		for item in items {
		
			// did we change values
			if (item.value != lastValue){
				
				// if this is the first change - we subtract time from present
				let interval:TimeInterval
					= (lastTime == 0) ? now - item.time  : lastTime - item.time
				
				lastTime = item.time
				lastValue = item.value
				
				timeline.append( RESTTimeSpanItem(time: item.time,
															 durration: interval, value: item.value))
			}
		}
		
		return timeline
	}
	
	
}

enum PumpHouseEvents: Int {
	case unknown 	= 0
	case startup		= 1
	case shutdown	= 2
	
	case bypassMode		= 3
	case inverterMode 	= 4
	case fastCharge 	 	= 5
	case floatCharge 	= 6
	case inverterNotResponding = 7
	case inverterFail = 8
	
	var description : String {
		switch self {
		// Use Internationalization, as appropriate.
		case .unknown: return "Unknown"
		case .startup: return "Start"
		case .shutdown: return "Stop"
		case .bypassMode: return "Bypass Mode"
		case .inverterMode: return "Inverter Mode"
		case .fastCharge: return "Fast Charge"
		case .floatCharge: return "Float Charge"
		case .inverterNotResponding: return "Not Responding"
		case .inverterFail: return "Fail"
		}
	}
}

struct RESTEventsTimeSpanItem: Codable {
	var time:  		Double
	var durration:  TimeInterval
	var event: 		Int
}

struct RESTEventsItem: Codable {
	var time:  Double
	var event: Int
	
	enum CodingKeys: String, CodingKey {
		case time = "time"
		case event = "event"
	}
}

struct RESTEvents: Codable {
	var events:  Array< RESTEventsItem>
	enum CodingKeys: String, CodingKey {
		case events = "events"
	}
	
	func timeLine() -> Array<RESTEventsTimeSpanItem> {
		
		var timeline:Array<RESTEventsTimeSpanItem> = []
		
		let items = self.events.reversed()
		let now = Date().timeIntervalSince1970
		var lastTime:Double = 0;
		
		for item in items {
			
			// if this is the first change - we subtract time from present
			let interval:TimeInterval
				= (lastTime == 0) ? now - item.time  : lastTime - item.time
			
			lastTime = item.time
			
			timeline.append( RESTEventsTimeSpanItem(time: item.time,
																 durration: interval, event: item.event))
		}
		
		return timeline
	}
	
	
	func groupedTimeline() -> [[RESTEventsTimeSpanItem]]  {
		
		var values:[[RESTEventsTimeSpanItem]] = []
	
		var lastOffsset:Int = 1;
		var i:Int  = -1

		let today = Date()
	 	
		let timeline = self.timeLine()
		for item in timeline {
			let date = Date.init(timeIntervalSince1970: item.time)
			let days = Calendar.current.numberOfDaysBetween(today, and: date)
		
			if(days != lastOffsset) {
				lastOffsset = days
				values.append([])
				i+=1
			}
			
			values[i].append(item)
		}
		
		return values
	}
	
}
 

class PHServerManager: ObservableObject {

	enum ServerError: Error {
		case connectFailed
		case invalidState
		case invalidURL
		case unknown
	}
 
	@Published var lastUpdate = Date()
	
 	static let shared: PHServerManager = {
			let instance = PHServerManager()
			// Setup code
			return instance
		}()
	
	init () {
		
	}
	
	
	func calculateSignature(forRequest: URLRequest, apiSecret: String ) -> String {
		
		if let method: String =  forRequest.httpMethod,
			let urlPath = forRequest.url?.path ,
			let daytimeHeader = forRequest.value(forHTTPHeaderField: "X-auth-date"),
			let apiKey = forRequest.value(forHTTPHeaderField: "X-auth-key")
		{
			var bodyHash:String = ""
			
			if let body = forRequest.httpBody {
				bodyHash = body.sha256String()
			}
			else {
				bodyHash = Data().sha256String()
			}
			
			let stringToSign =   method +  "|" + urlPath +  "|"
				+  bodyHash +  "|" + daytimeHeader + "|" + apiKey
  
			let signatureString = stringToSign.hmac(key: apiSecret)
			
				return signatureString;
		}
	
		return "";

	}
	
	func RESTCall(urlPath: String,
					  httpMethod: String? = "GET",
					  headers: [String : String]? = nil,
					  queries: [URLQueryItem]? = nil,
					  body: Any? = nil,
					  timeout:TimeInterval = 10,
					  completion: @escaping (URLResponse?,  Any?, Error?) -> Void)  {
		
		if let requestUrl: URL = AppData.serverInfo.url ,
			let apiKey = AppData.serverInfo.apiKey,
			let apiSecret = AppData.serverInfo.apiSecret {
			let unixtime = String(Int(Date().timeIntervalSince1970))
			
			let urlComps = NSURLComponents(string: requestUrl.appendingPathComponent(urlPath).absoluteString)!
			if let queries = queries {
				urlComps.queryItems = queries
			}
			var request = URLRequest(url: urlComps.url!)
			
			// Specify HTTP Method to use
			request.httpMethod = httpMethod
			request.setValue(apiKey,forHTTPHeaderField: "X-auth-key")
			request.setValue(String(unixtime),forHTTPHeaderField: "X-auth-date")
		
			if let body = body {
				let jsonData = try? JSONSerialization.data(withJSONObject: body)
				request.httpBody = jsonData
			}
 
			let sig =  calculateSignature(forRequest: request, apiSecret: apiSecret)
			request.setValue(sig,forHTTPHeaderField: "Authorization")
			
			headers?.forEach{
				request.setValue($1, forHTTPHeaderField: $0)
			}
				
			// Send HTTP Request
			request.timeoutInterval = timeout
			
 //	 	print(request)
			
			let session = URLSession(configuration: .ephemeral, delegate: nil, delegateQueue: .main)
			
			let task = session.dataTask(with: request) { (data, response, urlError) in
				
				if urlError != nil {
					completion(nil, nil, urlError	)
					return
				}
			
			
 // 			print ( String(decoding: (data!), as: UTF8.self))

				if let data = data as Data? {
					
					let decoder = JSONDecoder()
					
					if let restErr = try? decoder.decode(RESTError.self, from: data){
						completion(response, restErr.error, nil)
					}
					else if let obj = try? decoder.decode(RESTValuesList.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTHistory.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTStatus.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTSchemaList.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTProperties.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTEvents.self, from: data){
						completion(response, obj, nil)
					}
 					else if let jsonObj = try? JSONSerialization.jsonObject(with: data, options: .allowFragments) as? Dictionary<String, Any> {
						completion(response, jsonObj, nil)
					}
					else {
						completion(response, nil, nil)
					}
				}
			}
			
			task.resume()
		}
		else {
			completion(nil,nil, ServerError.invalidURL)
		}
	}
}

