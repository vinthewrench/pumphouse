//
//  InverterHistoryViewController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/13/21.
//

import Foundation
import UIKit
 
final class InverterHistoryCell: UITableViewCell {
	
	@IBOutlet var lblTime	: UILabel!
	@IBOutlet var lblDurration	: UILabel!
	@IBOutlet var lblEvent	: 	UILabel!

	@IBOutlet var imgDot	: UIImageView!
	@IBOutlet var lineTop	: UIView!
	@IBOutlet var lineBot	: UIView!
	 
	override func awakeFromNib() {
		super.awakeFromNib()
	}
}

class InverterHistoryViewController: UIViewController,
												 UITableViewDelegate,
												 UITableViewDataSource {
	
	var timeFormatter: DateFormatter {
		let formatter = DateFormatter()
		formatter.dateStyle = .none
		formatter.timeStyle = .short
		
		return formatter
	}

	var dateFormatter: DateFormatter {
		let formatter = DateFormatter()
		formatter.dateStyle = .medium
		formatter.doesRelativeDateFormatting = true
			formatter.timeStyle = .none
		
		return formatter
	}

	var intervalFormatter: DateComponentsFormatter {
		let formatter = DateComponentsFormatter()
		formatter.unitsStyle = .abbreviated
		formatter.zeroFormattingBehavior = .dropAll
		formatter.allowedUnits = [.day, .hour, .minute, .second]
		return formatter
	}
	
	class func create() -> InverterHistoryViewController? {
		let storyboard = UIStoryboard(name: "InverterHistory", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "InverterHistoryViewController") as? InverterHistoryViewController
		
		return vc
	}
	
	
	// cell reuse id (cells that scroll out of view can be reused)
	let cellReuseIdentifier = "InverterHistoryCell"
	
	@IBOutlet var tableView: UITableView!
	var timer = Timer()
	var events_grouped:[[RESTEventsTimeSpanItem]] = []

	override func viewDidLoad() {
		super.viewDidLoad()
		
		tableView.register(
			PHtableHeaderView.nib,
				  forHeaderFooterViewReuseIdentifier:
					PHtableHeaderView.reuseIdentifier)

	 
		self.tableView.tableFooterView = UIView()
		self.tableView.separatorStyle = .none
		tableView.delegate = self
		tableView.dataSource = self
		
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

 
	private func refreshView() {
		
		if(AppData.serverInfo.validated){
			
			let dp = DispatchGroup()

			dp.enter()
			PumpHouse.shared.fetchEvents() { result in
				if case .success(let events as RESTEvents ) = result {
					self.events_grouped = events.groupedTimeline()
				 
				}
				else {
					self.events_grouped = []
				}
 			dp.leave()
			}
						
			dp.notify(queue: .main) {
				self.tableView.reloadData()
				self.view.layoutIfNeeded()
			}
			
		}
	}
	
	func numberOfSections(in tableView: UITableView) -> Int {
		return self.events_grouped.count
	}
	
 	// number of rows in table view
	func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		return self.events_grouped[section].count
	}
	
	func tableView(_ tableView: UITableView, heightForRowAt indexPath: IndexPath) -> CGFloat {
		return 70.0
	}
	
	func tableView(_ tableView: UITableView, viewForHeaderInSection section: Int) -> UIView? {
	
		
		guard let headerView = tableView.dequeueReusableHeaderFooterView(
											 withIdentifier: PHtableHeaderView.reuseIdentifier)
											 as? PHtableHeaderView
				  else {
						return nil
				  }
		
	 	let item = self.events_grouped[section][0]
		let dateStr = self.dateFormatter.string(from: Date(timeIntervalSince1970: item.time))
		
		headerView.title?.text = dateStr
	
		return headerView
	}
 
	func tableView(_ tableView: UITableView,
						heightForHeaderInSection section: Int) -> CGFloat {
		return UITableView.automaticDimension
	}
	
	func tableView(_ tableView: UITableView,
						estimatedHeightForHeaderInSection section: Int) -> CGFloat {
		return 50.0
	}
	
	func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
		
		if let cell = tableView.dequeueReusableCell(withIdentifier:cellReuseIdentifier) as? InverterHistoryCell{
			
			cell.accessoryType = .none
			cell.selectionStyle = .none
 
			let item = self.events_grouped[indexPath.section][indexPath.row]
			
			cell.lineTop.isHidden = indexPath.row == 0
			cell.lineBot.isHidden = indexPath.row  ==  self.events_grouped[indexPath.section].count - 1

			cell.lblEvent.textColor = UIColor.black;
			if let state = PumpHouseEvents(rawValue: item.event) {
				
				cell.lblEvent.text = state.description
	
				switch state {
				case .startup:
					cell.imgDot.image = UIImage(systemName: "power")
					cell.imgDot.tintColor = UIColor.systemPurple
	
				case .bypassMode:
					cell.imgDot.image = UIImage(systemName: "equal.circle")
					cell.imgDot.tintColor =  UIColor.systemBlue
					
				case .inverterMode:
					cell.imgDot.image = UIImage(systemName: "minus.plus.batteryblock")
					cell.imgDot.tintColor =  UIColor.systemYellow
					
				case .floatCharge:
					cell.imgDot.image = UIImage(systemName: "minus.plus.batteryblock.fill")
					cell.imgDot.tintColor =  UIColor.systemGreen
		
				case .fastCharge:
					cell.imgDot.image = UIImage(systemName: "speedometer")
					cell.imgDot.tintColor =  UIColor.systemOrange
		
				case .inverterNotResponding:
					cell.imgDot.image = UIImage(systemName: "triangle")
					cell.imgDot.tintColor =  UIColor.red
					cell.lblEvent.textColor =  UIColor.red
					
				default:
					cell.imgDot.tintColor =  UIColor.systemGray
				}
				
				
			}

			let timeStr = self.timeFormatter.string(from: Date(timeIntervalSince1970: item.time))
			cell.lblTime.text = String(format: "%@", timeStr )
			
			if( item.durration.isInfinite){
				cell.lblDurration.isHidden = true
			}
			else if( item.durration == 0 ){
				cell.lblDurration.isHidden = false
				cell.lblDurration.text  = "-"
			}
			else {
				cell.lblDurration.isHidden = false
				cell.lblDurration.text  = self.intervalFormatter.string(from: item.durration)
			}
			
			
			return cell
		}
		
		return UITableViewCell()
	}
}
