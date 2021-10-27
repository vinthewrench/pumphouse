//
//  PumpValuesController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/7/21.
//

import Foundation
import UIKit

final class PumpValuesCell: UITableViewCell {
	
	@IBOutlet var lblTitle	: UILabel!
	@IBOutlet var lblSubTitle	: UILabel!
	@IBOutlet var lblValue	: UILabel!
	@IBOutlet var imgRight	: UIImageView!
	
	override func awakeFromNib() {
		super.awakeFromNib()
	}
}


class PumpValuesViewController: UIViewController,
										  UITableViewDelegate,
										  UITableViewDataSource {

	class func create() -> PumpValuesViewController? {
		let storyboard = UIStoryboard(name: "PumpValues", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "PumpValuesViewController") as? PumpValuesViewController
		
		return vc
	}
	
	
	// cell reuse id (cells that scroll out of view can be reused)
	let cellReuseIdentifier = "PumpValuesCell"
	
	@IBOutlet var tableView: UITableView!
	
	
	var groupedKeys:Array<keyGroupEntry> = []
	var keys:Array<String> = []
	var values:Dictionary<String, RESTValueDetails> = [:]
	var schema: Dictionary<String, RESTSchemaDetails> = [:]
	
	override func viewDidLoad() {
		super.viewDidLoad()
		
		// (optional) include this line if you want to remove the extra empty cell divider lines
		self.tableView.tableFooterView = UIView()
 		tableView.delegate = self
		tableView.dataSource = self
		
		tableView.register(
			PHtableHeaderView.nib,
				  forHeaderFooterViewReuseIdentifier:
					PHtableHeaderView.reuseIdentifier)

	}
	
	
	override func viewWillAppear(_ animated: Bool) {
		
		super.viewWillAppear(animated)
		
		if(	AppData.serverInfo.validated){
			
			let dp = DispatchGroup()
	
			dp.enter()
			PumpHouse.shared.fetchData(.values) { result in
				if case .success(let v as RESTValuesList) = result {
					
					self.groupedKeys = PumpHouse.shared.groupValueKeys(Array(v.values.keys))
					
					self.keys = Array(v.values.keys).sorted(by: <)
					self.values = v.values
					
				}
				else {
					self.keys = []
				}
				
				dp.leave()
			}
			
			dp.enter()
			PumpHouse.shared.fetchData(.schema) { result in
				if case .success(let scm as RESTSchemaList) = result {
					self.schema = scm.schema
				}
				
				dp.leave()
			}
			
			dp.notify(queue: .main) {
				self.tableView.reloadData()
			}
		}
	}
	
	
	func numberOfSections(in tableView: UITableView) -> Int {
		return self.groupedKeys.count
		
	}

	
	// number of rows in table view
	func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		return self.groupedKeys[section].keys.count
	}
 
	func tableView(_ tableView: UITableView, viewForHeaderInSection section: Int) -> UIView? {
	
		
		guard let headerView = tableView.dequeueReusableHeaderFooterView(
											 withIdentifier: PHtableHeaderView.reuseIdentifier)
											 as? PHtableHeaderView
				  else {
						return nil
				  }
	 
		headerView.title?.text = self.groupedKeys[section].title
	
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
		
		if let cell = tableView.dequeueReusableCell(withIdentifier:cellReuseIdentifier) as? PumpValuesCell{
			
			cell.accessoryType = .none
			cell.selectionStyle = .none
			
			let key =  self.groupedKeys[indexPath.section].keys[indexPath.row]
			
			cell.lblTitle?.text = key
			cell.imgRight.isHidden = true
			
			if	let sch = schema[key],
				  let value = self.values[key]?.value{
				
				let formatter = PumpValueFormatter()
				formatter.units = RESTschemaUnits(rawValue:sch.units) ?? .UNKNOWN
				cell.lblValue?.text  =  formatter.string(for: value)
				
				cell.lblSubTitle?.text = sch.name
				
				if let tracking = RESTschemaTracking(rawValue: sch.tracking){
					if(tracking == .trackedValue){
						cell.imgRight.isHidden = false
					}
				}
			}
			return cell
		}
		
		return UITableViewCell()
	}
	
	
	func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
		
		let key =  self.groupedKeys[indexPath.section].keys[indexPath.row]
		if let  sch = schema[key] {
			
			guard let tracking = RESTschemaTracking(rawValue: sch.tracking) else {
				return
			}
			
			if(tracking == .trackedValue){
				self.performSegue(withIdentifier: "segueToHistory", sender: nil)
			}
		}
	}
	
	override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
		if segue.identifier == "segueToHistory" {
			
			if let vc = segue.destination as? PumpValueHistoryController {
				if let indexPath = tableView.indexPathForSelectedRow {
					
					let key =  self.groupedKeys[indexPath.section].keys[indexPath.row]
					if	let sch = schema[key]{
						vc.valueKey = key
						vc.schema = sch
					}
				}
			}
		}
	}
}
	
 
