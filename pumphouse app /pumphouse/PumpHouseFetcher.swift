//
//  PumpHouseFetcher.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/20/21.
//

import Foundation


public class PumpHouseFetcher: ObservableObject {
	
	@Published var values:  Dictionary<String, RESTValueDetails> = [:]
	
	@Published var lastUpdate = Date()
	@Published var lastEtag:Int = 0
	@Published var isLoaded:Bool = false
	@Published var tankLevel:Double = 0

	var timer = Timer()
	
	static let shared: PumpHouseFetcher = {
		let instance = PumpHouseFetcher()
		// Setup code
		return instance
	}()
	
	private init(){
		reload()
		
	}

	public func startPolling() {
		timer =  Timer.scheduledTimer(withTimeInterval: 5.0,
												repeats: true,
												block: { timer in
												self.getChangedValues()
//													self.getKeypads()
												})
	}
	
	public func stopPollng(){
		timer.invalidate()
	}
	
	public func reload(){
		self.isLoaded = false
		
		guard AppData.serverInfo.validated  else {
			return
		}
		
		let dp = DispatchGroup()
	 
			dp.enter()
			getValues(){
				dp.leave()
			}

 
	 
		dp.notify(queue: .main) {
			self.isLoaded = true
		}
		
	}
		
	public func getValues(completion: @escaping () -> Void = {}) {
		self.lastEtag = 0
		self.getChangedValues( ){
			completion()
		}
	}

	public func getChangedValues( completion: @escaping () -> Void = {}) {
		
		return
		
		guard AppData.serverInfo.validated  else {
			return
		}
		
		PHServerManager.shared.RESTCall(urlPath: "values",
												  headers:  ["If-None-Match" : String( self.lastEtag + 1 )],
												  queries: nil)
		{ (response, json, error)  in
			
			if let obj = json  as? RESTValuesList {
				if let etag = obj.ETag {
					self.lastEtag = etag
				}
 
				self.values = self.values.merging(obj.values) { $1 }
			
				if let tl = self.values["TANK"]?.value,
					let ftl = Double(tl)
				{
					self.values["TANK"]?.value = String(ftl + 10)
				}
				 
					if let tl = self.values["TANK"]?.value,
						let ftl = Double(tl) {
					self.tankLevel = ftl * 0.01
				}
			
			}
			
		}
		completion()

	}
}
