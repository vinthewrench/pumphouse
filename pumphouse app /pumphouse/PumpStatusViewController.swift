//
//  PumpStatusViewController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/24/21.
//

 
import Foundation
import UIKit
import Toast
 
class GradientView: UIView {

	 // MARK: - Lifecycle
//	 override init(frame: CGRect) {
//		  super.init(frame: frame)
//	 }
//
//	 required init?(coder: NSCoder) {
//		super.init(coder:coder)
//		  // fatalError("init(coder:) has not been implemented")
//	 }

	 override func layoutSubviews() {
		  super.layoutSubviews()
		  configureGradientLayer()
	 }

	 func configureGradientLayer(){
		  backgroundColor = .clear
		  let gradient = CAGradientLayer()
		
		let endcolor =  UIColor(red: 15/255, green: 94/255, blue: 156/266, alpha: 0.8)
		let startcolor =  UIColor(red: 116/255, green: 204/255, blue: 244/255, alpha: 1)
		  gradient.colors = [startcolor.cgColor, endcolor.cgColor]
		  gradient.locations = [0, 1]
		  gradient.frame = bounds
		  layer.addSublayer(gradient)
	 }
}
class PumpStatusViewController: UIViewController {

	@IBOutlet var vwPump	: UIView!
	@IBOutlet var lblPumpAvail	: UILabel!
	@IBOutlet var lblPumpGals	: UILabel!
	@IBOutlet var cnstTankHeight: NSLayoutConstraint!
	@IBOutlet var vwTankWater: GradientView!
	var originalTankHeight: CGFloat = 0
	
	@IBOutlet var vwPower		: UIView!
	@IBOutlet var lblInverter	: UILabel!
	@IBOutlet var lblACin	: UILabel!
	@IBOutlet var lblACout	: UILabel!
	@IBOutlet var lblAChz	: UILabel!
	@IBOutlet var lblACcur	: UILabel!
	@IBOutlet var imgAC		: UIImageView!

	@IBOutlet var imgACin		: UIImageView!

	@IBOutlet var imgBatIn		: UIImageView!
	@IBOutlet var imgBatOut		: UIImageView!
	@IBOutlet var imgBattery		: UIImageView!

	@IBOutlet var lblBatCur	: UILabel!
	@IBOutlet var lblBatV	: 	UILabel!
	@IBOutlet var lblBatSOC	: UILabel!
	@IBOutlet var lblBatTime	: UILabel!
	@IBOutlet var lblBatConsumed	: UILabel!

	@IBOutlet var lblTemp1	: UILabel!
	@IBOutlet var lblTemp2	: UILabel!
	@IBOutlet var lblCPUTemp	: UILabel!

	var timer = Timer()
	
	var ttgTimeFormatter: DateComponentsFormatter {
		let formatter = DateComponentsFormatter()
		formatter.allowedUnits = [.day, .hour, .minute, .second]
		formatter.unitsStyle = .short
		return formatter
	}

	func tempInFahrenheit(_ temperature: Double) -> Double {
			let fahrenheitTemperature = temperature * 9 / 5 + 32
			return fahrenheitTemperature
	}
	
	override func viewDidLoad() {
			super.viewDidLoad()
		
		self.vwPump!.layer.borderWidth = 1
		self.vwPump!.layer.borderColor = UIColor.darkGray.cgColor

		self.vwPower!.layer.borderWidth = 1
		self.vwPower!.layer.borderColor = UIColor.darkGray.cgColor
		}
 

	private func refreshView() {
		
		if(AppData.serverInfo.validated){
		
			 		PumpHouse.shared.fetchTankValues() { result in
				
				if case .success(let pv as PumpValues ) = result {
					
					self.lblPumpAvail.text =  String(format: "%.0f%%", pv.tankPercent )
					self.lblPumpGals.text =  String(format: "%.f Gals", pv.tankGals )
					
					var newHeight = self.originalTankHeight * (CGFloat(pv.tankPercent) / 100.00)
					if newHeight > self.originalTankHeight {
						newHeight = self.originalTankHeight
					}
					self.cnstTankHeight.constant = newHeight
	
				}
				else {
				}
		}
 
			PumpHouse.shared.fetchInverterValues() { result in
				if case .success(let iv as InverterValues ) = result {

					self.lblBatSOC.text =  String(format: "%.f%%", iv.batSOC )

					self.lblBatV.text =  String(format: "%.1f V", iv.batVolts )
					self.lblBatCur.text =  String(format: "%.0f A", iv.batAmps )
					 
					self.lblACin.text =  String(format: "%.0f V", iv.acIn )
					self.lblACout.text =  String(format: "%.0f V", iv.acOut )
					self.lblACcur.text =  String(format: "%.0f A", iv.acAmps )
					self.lblAChz.text =  String(format: "%.0f Hz", iv.acHz )

					self.lblBatConsumed.text =  String(format: "%.1f Ah", iv.batteryConsumed )

					if(iv.batteryTime	 == -1){
						self.lblBatTime.text = String("Infinite")
					}
					else
					{
						let timeStr = self.ttgTimeFormatter.string(from: TimeInterval(iv.batteryTime * 60))!
						self.lblBatTime.text = String(format: "%@", timeStr )
					}
					
					if(iv.utilityFail){
						self.lblInverter.text = "Inverter";
						self.imgBatOut.isHidden = false
						self.imgBatIn.isHidden = true
						self.imgBattery.tintColor = UIColor.red
						self.imgACin.tintColor =  UIColor(white: 0.80, alpha: 	0.8)
						self.imgAC.tintColor =  UIColor(white: 0.80, alpha: 	0.8)
						self.lblACin.isHidden = true
						self.lblBatTime.isHidden = false
						self.lblBatConsumed.isHidden = false
					}
					else {
						self.lblInverter.text = "Charger";
						self.imgBatOut.isHidden = true
						self.imgBatIn.isHidden = false
						self.imgBattery.tintColor = UIColor.black
	
						self.imgACin.tintColor =  UIColor(white: 0.0, alpha: 	1.0)
						self.imgAC.tintColor =  UIColor(white: 0.0, alpha: 	1.0)
						self.lblACin.isHidden = false
						self.lblBatTime.isHidden = true
						self.lblBatConsumed.isHidden = true
		}
					
					self.lblCPUTemp.text =  String(format: "%.0f°C", iv.cpuTemp )
					self.lblTemp1.text =  String(format: "%.0f°F", self.tempInFahrenheit(iv.temp1 ))
					self.lblTemp2.text =  String(format: "%.0f°F", self.tempInFahrenheit(iv.temp2 ))
				} else {
					
				}
			}
		}
		else {
			
		}

	}
	override func viewWillAppear(_ animated: Bool) {
		
		super.viewWillAppear(animated)
	
		self.lblPumpAvail.text =  ""
		self.lblPumpGals.text =  ""
		originalTankHeight = self.cnstTankHeight.constant;
		

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

	
}
