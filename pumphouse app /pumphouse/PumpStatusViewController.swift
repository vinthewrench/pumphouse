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

	@IBOutlet var lblPumpAvail	: UILabel!
	@IBOutlet var lblPumpGals	: UILabel!
	@IBOutlet var cnstTankHeight: NSLayoutConstraint!
	@IBOutlet var vwTankWater: GradientView!
	var originalTankHeight: CGFloat = 0
	
	@IBOutlet var lblInverter	: UILabel!
	@IBOutlet var lblACin	: UILabel!
	@IBOutlet var lblACout	: UILabel!
	@IBOutlet var lblAChz	: UILabel!
	@IBOutlet var lblACcur	: UILabel!

	@IBOutlet var lblBatCur	: UILabel!
	@IBOutlet var lblBatV	: 	UILabel!
	@IBOutlet var lblBatSOC	: UILabel!

	@IBOutlet var lblTemp1	: UILabel!
	@IBOutlet var lblTemp2	: UILabel!
	@IBOutlet var lblCPUTemp	: UILabel!

	
	func tempInFahrenheit(_ temperature: Double) -> Double {
			let fahrenheitTemperature = temperature * 9 / 5 + 32
			return fahrenheitTemperature
	}
	
	override func viewDidLoad() {
			super.viewDidLoad()
		}
	
	override func viewWillAppear(_ animated: Bool) {
		
		super.viewWillAppear(animated)
	
		self.lblPumpAvail.text =  ""
		self.lblPumpGals.text =  ""
		originalTankHeight = self.cnstTankHeight.constant;
		
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

					if(iv.utilityFail){
						self.lblInverter.text = "Inverter";
						self.lblInverter.textColor =  UIColor.systemBlue
					}
					else {
						self.lblInverter.text = "Charger";
						self.lblInverter.textColor =  UIColor.systemTeal

					}
					
					self.lblCPUTemp.text =  String(format: "%.0f°C", iv.cpuTemp )
					self.lblTemp1.text =  String(format: "%.0f°F", self.tempInFahrenheit(iv.temp1 ))
					self.lblTemp2.text =  String(format: "%.0f°F", self.tempInFahrenheit(iv.temp2 ))
				} else {
					
				}
			}
		}
	}
	
	override func viewWillDisappear(_ animated: Bool) {
		super.viewWillDisappear(animated)

	}
	

}
