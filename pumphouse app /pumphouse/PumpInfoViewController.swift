//
//  PumpInfoViewController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/22/21.
//

import Foundation
import UIKit

class PumpInfoViewController: UIViewController  {
	
	@IBOutlet var lblVersion		: UILabel!
	@IBOutlet var lblBuildDate	: UILabel!

	@IBOutlet var lblNodeName	: UILabel!
	@IBOutlet var lblOSName	: UILabel!
	@IBOutlet var lblOSVersion	: UILabel!
	@IBOutlet var lblHW			: UILabel!

	@IBOutlet var lblLocalTime	: UILabel!
	@IBOutlet var lblStatus	: UILabel!
	@IBOutlet var lblUptime	: UILabel!
	
	var timer = Timer()

	var upTimeFormatter: DateComponentsFormatter {
		let formatter = DateComponentsFormatter()
		formatter.allowedUnits = [.day, .hour, .minute, .second]
		formatter.unitsStyle = .short
		return formatter
	}
	 
 	class func create() -> PumpInfoViewController? {
		let storyboard = UIStoryboard(name: "InfoView", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "PumpInfoViewController") as? PumpInfoViewController
		
		return vc
	}

	override func viewDidLoad() {
		super.viewDidLoad()
	}
	
	override func viewWillAppear(_ animated: Bool) {
		super.viewWillAppear(animated)
		
		if(AppData.serverInfo.validated){
			startPolling();
		}
		else {
			stopPollng();
		}
		
		
		refreshView()
	}
	
	override func viewWillDisappear(_ animated: Bool) {
		super.viewWillDisappear(animated)
		stopPollng();
		
	}

	private func refreshView() {
		
		if(AppData.serverInfo.validated){
			PumpHouse.shared.fetchData(.status) { result in
				
				if case .success(let status as RESTStatus) = result {
					self.lblVersion.text = status.version
					self.lblBuildDate.text = status.buildtime
					
					self.lblNodeName.text = status.os_nodename
					self.lblOSName.text = status.os_sysname
					self.lblOSVersion.text = status.os_release
					self.lblHW.text = status.os_machine

					self.lblLocalTime.text = status.date
					
					let upTimeStr = self.upTimeFormatter.string(from: TimeInterval(status.uptime))!
					self.lblUptime.text = upTimeStr
					self.lblStatus.text = status.stateString
		
					
				}
				else {
					self.lblVersion.text 	= ""
					self.lblBuildDate.text = ""
					self.lblNodeName.text 	= ""
					self.lblOSName.text	 = ""
					self.lblOSVersion.text = ""
					self.lblHW.text 		= ""
					self.lblLocalTime.text = ""
					self.lblStatus.text 	= ""
					self.lblUptime.text 	= ""
				}
			}
			
		}
	}
	
	public func startPolling() {
		timer =  Timer.scheduledTimer(withTimeInterval: 1.0,
												repeats: true,
												block: { timer in
													self.refreshView()
												})
	}
	
	public func stopPollng(){
		timer.invalidate()
	}

	
}
	 
