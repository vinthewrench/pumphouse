//
//  AppPrefs.swift
//  Home Control
//
//  Created by Vincent Moscaritolo on 6/27/21.
//

import Foundation


@propertyWrapper
struct Storage<T: Codable> {
	private let key: String
	private let defaultValue: T
	
	init(key: String, defaultValue: T) {
		self.key = key
		self.defaultValue = defaultValue
	}
	
	var wrappedValue: T {
		get {
			// Read value from UserDefaults
			guard let data = UserDefaults.standard.object(forKey: key) as? Data else {
				// Return defaultValue when no data in UserDefaults
				return defaultValue
			}
			
			// Convert data to the desire data type
			let value = try? JSONDecoder().decode(T.self, from: data)
			return value ?? defaultValue
		}
		set {
			// Convert newValue to data
			let data = try? JSONEncoder().encode(newValue)
			
 	//		print ( String(decoding: (data)!, as: UTF8.self))
			// Set value to UserDefaults
			UserDefaults.standard.set(data, forKey: key)
			
		}
	}
}
 

struct ServerPrefs: Codable {
	var 	apiKey: 			String?
	var 	apiSecret: 		String?
	var 	url:				URL?
	var 	validated: 		Bool // we did succeed in a login
	var 	tabSelection:	Int
}

struct AppData {
	
	//	 @Storage(key: "enable_auto_login_key", defaultValue: false)
	//	 static var enableAutoLogin: Bool
	
	// Declare a ServerPrefs object
	
	@Storage(key: "pumphouse",
				defaultValue: ServerPrefs(apiKey: "",
												  apiSecret: "",
												  url: URL(string: "http://pi7:8081")!,
												  validated: false,
												  tabSelection: 0
												  ))
	
	static var serverInfo: ServerPrefs
}

