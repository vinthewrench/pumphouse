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


struct RESTVersion: Codable {
	var timestamp: String
	var version: String
}


class RESTDateInfo: Codable {
	let date: String
	let uptime: Int
}


struct RESTStatus: Codable {
	var state: Int
	var stateString: String?
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

struct RESTProperties: Codable {
	var properties:  Dictionary<String, String>
  }

struct RESTValuesList: Codable {
	var values:  Dictionary<String, RESTValueDetails>
	var ETag: 	Int?
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
			
// 	 	print(request)
			
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
					else if let obj = try? decoder.decode(RESTDateInfo.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTStatus.self, from: data){
						completion(response, obj, nil)
					}					else if let obj = try? decoder.decode(RESTVersion.self, from: data){
						completion(response, obj, nil)
					}
					else if let obj = try? decoder.decode(RESTProperties.self, from: data){
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

