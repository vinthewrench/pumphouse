//
//  PumpHistoryViewController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/5/21.
//

import Foundation
import UIKit
import Charts

public class EpochAxisValueFormatter: NSObject, AxisValueFormatter {
 
	let format: DateFormatter = DateFormatter()
 
	override init() {
		super.init()
		format.timeZone = .current
		format.dateFormat = "MM/dd"
	}
	 
	 public func stringForValue(_ value: Double, axis: AxisBase?) -> String {

		let d1 = Date.init(timeIntervalSince1970: value)
		return String(format.string(from: d1))
		}
	 }

class PumpHistoryViewController: UIViewController, ChartViewDelegate {
	
	class func create() -> PumpHistoryViewController? {
		let storyboard = UIStoryboard(name: "PumpHistory", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "PumpHistoryViewController") as? PumpHistoryViewController
		
		return vc
	}
	
	@IBOutlet weak var vwChart	: LineChartView!
	
	override func viewDidLoad() {
		super.viewDidLoad()
		
 	}
	
	override func viewWillAppear(_ animated: Bool) {
		
		super.viewWillAppear(animated)
		
		let params = PumpHouse.shared.cachedValues
		let gals = params.tankGals
	
		vwChart.rightAxis.enabled = false
		vwChart.dragEnabled = true
		vwChart.setScaleEnabled(true)
		vwChart.pinchZoomEnabled = true
		vwChart.backgroundColor = .white
		vwChart.delegate = self
		vwChart.gridBackgroundColor = .systemBlue
		
 	//	vwChart.animate(xAxisDuration: 2.5)
		let ll2 = ChartLimitLine(limit: gals * 0.9, label: "Float Limit")
		ll2.lineWidth = 2
 		ll2.lineDashLengths = [5,5]
		ll2.labelPosition = .rightBottom
		ll2.valueFont = .systemFont(ofSize: 10)
		ll2.valueTextColor = .darkGray
		ll2.lineColor = .lightGray

		let leftAxis = vwChart.leftAxis
		leftAxis.removeAllLimitLines()
 		leftAxis.addLimitLine(ll2)
 		leftAxis.axisMaximum = gals
 	// 	leftAxis.axisMinimum = 0
		leftAxis.gridLineDashLengths = [5, 5]
//		leftAxis.drawLimitLinesBehindDataEnabled = true

		leftAxis.axisLineColor = .black
		leftAxis.labelTextColor	= .black
		leftAxis.labelPosition = .insideChart
 
		let xAxis = vwChart.xAxis
		xAxis.labelPosition = .top
 
		refreshView()
	}
	
	func chartValueSelected(_ chartView: ChartViewBase, entry: ChartDataEntry, highlight: Highlight) {
			print(entry)
	}
	
	private func refreshView() {
		
		if(AppData.serverInfo.validated){
			
			PumpHouse.shared.fetchHistory("TANK_RAW") { result in
				
				if case .success(let hist as RESTHistory ) = result {
					var items = hist.values
					
					items.sort{
						$0.time < $1.time
					}
					
					let params = PumpHouse.shared.cachedValues
					let empty =  params.tankEmpty
					let full =  params.tankFull
					let gals =  params.tankGals
		 
					var minTime:Double = .greatestFiniteMagnitude
					var maxTime:Double = 0

					var lineChartEntry = [ChartDataEntry]()

					for i in 0..<items.count {
						if var tank_raw  = Double(items[i].value){
							
							if(tank_raw < empty){ tank_raw = empty}
							if (tank_raw > full) {tank_raw = full}
							let tankP  = ((tank_raw - empty) / (full - empty)) * gals
	
	 					let t = items[i].time
							
						if(t < minTime){
								minTime = t
							}
							if(t > maxTime){
								maxTime = t
							}

							let value = ChartDataEntry(x:t, y:tankP )
							lineChartEntry.append(value)
						}
				
					}
						let line1 = LineChartDataSet(entries: lineChartEntry,label: "")
						line1.mode = .cubicBezier
						line1.lineWidth = 1
						line1.colors = [NSUIColor.systemBlue]
						line1.drawCirclesEnabled = false

					
					let xAxis = self.vwChart.xAxis
 					xAxis.axisMinimum = minTime
 					xAxis.axisMaximum = maxTime
					xAxis.labelCount = 5
					xAxis.labelTextColor = .black
 					xAxis.valueFormatter = EpochAxisValueFormatter()

					let data = LineChartData()
					data.append(line1)
					self.vwChart.data = data

	

//						self.vwChart.chartDescription.text = "Water Level"
						
					 
				}
			}
		}
	}
	
}
