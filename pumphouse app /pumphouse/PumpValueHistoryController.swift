//
//  PumpValueHistoryController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/12/21.
//

import Foundation
import UIKit

final class PumpValueHistoryCell: UITableViewCell {
	
	@IBOutlet var lblTitle	: UILabel!
	@IBOutlet var lblSubTitle	: UILabel!
	@IBOutlet var lblValue	: UILabel!
 
	override func awakeFromNib() {
		super.awakeFromNib()
	}
}


class PumpValueHistoryController: 	UIViewController,										  										UITableViewDelegate,
											UITableViewDataSource   {
	
	// cell reuse id (cells that scroll out of view can be reused)
	let cellReuseIdentifier = "PumpValueHistoryCell"
	
	
	@IBOutlet var lblTitle	: UILabel!
	@IBOutlet var lblSubTitle	: UILabel!
	@IBOutlet var lblCount	: UILabel!
	@IBOutlet var tableView: UITableView!

	var valueKey  = String()
	var schema :RESTSchemaDetails =  RESTSchemaDetails()
	var history: Array< RESTHistoryItem> = []

	var valueFormatter: PumpValueFormatter  {
		let formatter = PumpValueFormatter()
		formatter.units = RESTschemaUnits(rawValue:schema.units) ?? .UNKNOWN
 		return formatter
	}
	
	var timeFormatter: DateFormatter {
		let formatter = DateFormatter()
		formatter.dateStyle = .short
		formatter.timeStyle = .short
		formatter.doesRelativeDateFormatting = true
	
		return formatter
	}

	
	override func viewDidLoad() {
		super.viewDidLoad()
		
		// (optional) include this line if you want to remove the extra empty cell divider lines
		self.tableView.tableFooterView = UIView()
		tableView.delegate = self
		tableView.dataSource = self
	}

	
	override func viewWillAppear(_ animated: Bool) {
		
		super.viewWillAppear(animated)

		lblTitle.text = "History: " +  self.valueKey
		lblSubTitle.text = self.schema.name
		lblCount.isHidden = true
		refreshView()
		
	}

	
	private func refreshView() {
		
		if(AppData.serverInfo.validated){
			
			PumpHouse.shared.fetchHistory(valueKey) { result in
				
				if case .success(let hist as RESTHistory ) = result {
					var items = hist.values
					
					items.sort{
						$0.time > $1.time
					}
					
					self.lblCount.isHidden = false
					self.lblCount.text = String(items.count)
					self.history = items
						
				}
				else {
					self.tableView.reloadData()
					self.history = []
					self.lblCount.isHidden = true
					
				}
				self.tableView.reloadData()

			}
		}
	}
	
	func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		return self.history.count
		
	}
	
	func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {

		if let cell = tableView.dequeueReusableCell(withIdentifier:cellReuseIdentifier) as? PumpValueHistoryCell{
			
			cell.accessoryType = .none
			cell.selectionStyle = .none
			
			let val = self.history[indexPath.row]
 			let timeStr = self.timeFormatter.string(from: Date(timeIntervalSince1970: val.time))
			cell.lblSubTitle.text = String(format: "%@", timeStr )			
			cell.lblTitle.text = valueFormatter.string(for: val.value)
			
			return cell
		}
		
		return UITableViewCell()
	}
	

}

