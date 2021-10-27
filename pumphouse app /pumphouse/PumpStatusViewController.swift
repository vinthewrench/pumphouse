//
//  PumpStatusViewController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/24/21.
//

 
import Foundation
import UIKit
import Toast
import EzPopup

class GradientView: UIView {
 
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

class BatteryLevelView: UIView {
 
	 override func layoutSubviews() {
		  super.layoutSubviews()
		  configureGradientLayer()
	 }

	 func configureGradientLayer(){
		  backgroundColor = .clear
		  let gradient = CAGradientLayer()
		
	 
		let endcolor =  UIColor(red: 46/255, green: 182/255, blue: 44/266, alpha: 0.8)
		let startcolor =  UIColor(red: 131/255, green: 225/255, blue: 117/255, alpha: 0.8)
		  gradient.colors = [startcolor.cgColor, endcolor.cgColor]
		  gradient.locations = [0, 1]
		  gradient.frame = bounds
		  layer.addSublayer(gradient)
	 }
}

class PumpStatusViewController: UIViewController {
	
	@IBOutlet var vwOverlay	: UIView!
	
	@IBOutlet var vwPump	: UIView!
	@IBOutlet var lblPumpAvail	: UILabel!
	@IBOutlet var lblPumpGals	: UILabel!
	@IBOutlet var cnstTankHeight: NSLayoutConstraint!
	@IBOutlet var vwTankWater: GradientView!
	var originalTankHeight: CGFloat = 0
	var newTankHeight: CGFloat = 0
	
	@IBOutlet var imgWellPump			: UIImageView!
	@IBOutlet var lblPumpStatus		: UILabel!
	
	@IBOutlet var imgPressurePump		: UIImageView!
	
	@IBOutlet var vwPower		: UIView!
	@IBOutlet var btnInverter	: UIButton!

	@IBOutlet var lblACin	: UILabel!
	@IBOutlet var lblACout	: UILabel!
	@IBOutlet var lblAChz	: UILabel!
	@IBOutlet var lblACcur	: UILabel!
	@IBOutlet var imgAC		: UIImageView!
	
	@IBOutlet var imgInverter: UIImageView!
	@IBOutlet var imgACin		: UIImageView!
	@IBOutlet var imgBatIn		: UIImageView!
	@IBOutlet var imgBatOut		: UIImageView!
	@IBOutlet var imgBattery		: UIImageView!
	
	@IBOutlet var vwBatteryLevel: BatteryLevelView!
	var originalBatteryHeight: CGFloat = 0
	var newBatteryHeight: CGFloat = 0
	@IBOutlet var cnstBatteryHeight: NSLayoutConstraint!
	
	@IBOutlet var lblBatCur	: UILabel!
	@IBOutlet var lblBatV	: 	UILabel!
	@IBOutlet var lblBatSOC	: UILabel!
	@IBOutlet var lblBatTime	: UILabel!
	@IBOutlet var lblBatConsumed	: UILabel!

	@IBOutlet var vwTemp	: UIView!
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
	
	override func viewWillLayoutSubviews() {
		super.viewWillLayoutSubviews();
		
		self.cnstBatteryHeight.constant = newBatteryHeight
		self.cnstTankHeight.constant = newTankHeight
		
	}
	
	
	override func viewDidLoad() {
		super.viewDidLoad()
		
		self.vwPump!.layer.borderWidth = 1
		self.vwPump!.layer.borderColor = UIColor.darkGray.cgColor
		
		self.vwPower!.layer.borderWidth = 1
		self.vwPower!.layer.borderColor = UIColor.darkGray.cgColor
		
		self.vwTemp!.layer.borderWidth = 1
		self.vwTemp!.layer.borderColor = UIColor.darkGray.cgColor
	}
	
	
	private func refreshView() {
		
		if(AppData.serverInfo.validated){

			// we dont have HW for this yet
			lblPumpStatus.isHidden = true
			imgWellPump.tintColor = UIColor.lightGray
			imgPressurePump.tintColor = UIColor.lightGray
	
			PumpHouse.shared.fetchValues() { result in
				
				if case .success(let phv as PumpHouseValues ) = result {
					
					self.vwOverlay.isHidden = true
					var newHeight:CGFloat;
					
					self.lblPumpAvail.text =  String(format: "%.0f%%", phv.tankPercent )
					self.lblPumpGals.text =  String(format: "%.f Gals", phv.tankGals )
					
					newHeight = self.originalTankHeight * (CGFloat(phv.tankPercent) / 100.00)
					if newHeight > self.originalTankHeight {
						newHeight = self.originalTankHeight
					}
					self.newTankHeight = newHeight
					
					self.lblBatSOC.text =  String(format: "%.f%%", phv.batSOC/10.0 )
					
					newHeight = self.originalBatteryHeight * (CGFloat(phv.batSOC) / 100.00)
					if newHeight > self.originalBatteryHeight {
						newHeight = self.originalBatteryHeight
					}
					self.newBatteryHeight = newHeight;
					
					self.lblBatV.text =  String(format: "%.1f V", phv.batVolts / 1000.00 )
					
					let ba1 = abs( phv.batAmps / 1000.0 )
					if(ba1 < 0.9) {
						self.lblBatCur.text =  String(format: "%.1f A", phv.batAmps / 1000.0 )
					}
					else {
						self.lblBatCur.text = ""
					}
					
					self.lblACin.text =  String(format: "%.0f V", phv.acIn )
					self.lblACout.text =  String(format: "%.0f V", phv.acOut )
					
					if(phv.invLoad == 0){
						self.lblACcur.isHidden = true
					}
					else {
						self.lblACcur.text =  String(format: "%.0f%%", phv.invLoad )
						self.lblACcur.isHidden = false
					}
					
					if(phv.acHz == 60){
						self.lblAChz.isHidden = true
					}
					else {
						self.lblAChz.text =  String(format: "%.1f Hz", phv.acHz )
						self.lblAChz.isHidden = false
					}
					
					self.lblBatConsumed.text =  String(format: "%.1f Ah", phv.batteryConsumed/100.0 )
					
					if(phv.batteryTime	 == -1){
						self.lblBatTime.text = String("Infinite")
					}
					else
					{
						let timeStr = self.ttgTimeFormatter.string(from: TimeInterval(phv.batteryTime * 60))!
						self.lblBatTime.text = String(format: "%@", timeStr )
					}
					
	
					if(phv.inverterState == .connected){
						
						self.imgInverter.tintColor = UIColor.black
						self.btnInverter.setImage(nil, for: .normal);
						self.btnInverter.tintColor = UIColor.black
						self.btnInverter.setTitleColor(UIColor.black, for: .normal);

						self.lblACout.isHidden = false
		
						if(phv.utilityFail){
							self.btnInverter.setTitle("Inverter", for: .normal)
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
							self.btnInverter.setTitle("Charger", for: .normal)
							self.imgBatOut.isHidden = true
							self.imgBatIn.isHidden = false
							self.imgBattery.tintColor = UIColor.black
							
							self.imgACin.tintColor =  UIColor(white: 0.0, alpha: 	1.0)
							self.imgAC.tintColor =  UIColor(white: 0.0, alpha: 	1.0)
							self.lblACin.isHidden = false
							self.lblBatTime.isHidden = true
							self.lblBatConsumed.isHidden = true
						}
					}
					else {
						self.imgInverter.tintColor = UIColor.lightGray

						self.btnInverter.setTitle("Inverter", for: .normal)
						self.btnInverter.setImage(UIImage(systemName: "exclamationmark.triangle"), for: .normal);
						self.btnInverter.tintColor = UIColor.red
						self.btnInverter.setTitleColor(UIColor.red, for: .normal);
						
						self.imgBatOut.isHidden = true
						self.imgBatIn.isHidden = false
						self.imgBattery.tintColor = UIColor.black
						
						self.imgACin.tintColor =  UIColor(white: 0.0, alpha: 	1.0)
						self.imgAC.tintColor =  UIColor(white: 0.0, alpha: 	1.0)
						self.lblACin.isHidden = true
						self.lblACout.isHidden = true
						self.lblBatTime.isHidden = true
						self.lblBatConsumed.isHidden = true
					}
					 
					
					self.lblCPUTemp.text =  String(format: "%.0f°C", phv.cpuTemp )
					self.lblTemp1.text =  String(format: "%.0f°F", self.tempInFahrenheit(phv.temp1 ))
					self.lblTemp2.text =  String(format: "%.0f°F", self.tempInFahrenheit(phv.temp2 ))
				}
				else
				{
					self.vwOverlay.isHidden = false
				}
			}
		}
		else {
			self.vwOverlay.isHidden = false
		}
	}
	
	override func viewWillAppear(_ animated: Bool) {
		
		super.viewWillAppear(animated)
		
		self.lblPumpAvail.text =  ""
		self.lblPumpGals.text =  ""
		originalTankHeight = self.cnstTankHeight.constant;
		originalBatteryHeight = self.cnstBatteryHeight.constant;
		
		if(AppData.serverInfo.validated){
			startPolling();
		}
		else {
			stopPollng();
		}
		
		refreshView()
	}
	
	override func viewDidAppear(_ animated: Bool) {
		super.viewDidAppear(animated)
		
		if(!AppData.serverInfo.validated){
			displaySetupView();
		}
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
	
	@IBAction func SetupUpClicked(_ sender: UIButton) -> Void {
		
		self.displaySetupView()
	}
	
	@IBAction func DetailsClicked(_ sender: UIButton) -> Void {
		
		if let pumpValues = PumpValuesViewController.create() {
			
			self.show(pumpValues, sender: self)
		}
		
	}
	@IBAction func InfoClicked(_ sender: UIButton) -> Void {
		
		if let pumpInfo = PumpInfoViewController.create() {
			
			self.show(pumpInfo, sender: self)
		}
		
	}

	@IBAction func InverterBtnClicked(_ sender: UIButton) -> Void {
		
		if let inverterHistory = InverterHistoryViewController.create() {
			self.show(inverterHistory, sender: self)
		}
	}
	
	@IBAction func wellPumpImageTapped(sender: UITapGestureRecognizer) {
		
		if (sender.state == .ended) {
					
		}
	}

	@IBAction func tankImageTapped(sender: UITapGestureRecognizer) {
		
		if (sender.state == .ended) {
			
			if let pumpHistory = PumpHistoryViewController.create() {
				
				self.show(pumpHistory, sender: self)
			}
			
		}
	}
	
	@IBAction func inverterImageTapped(sender: UITapGestureRecognizer) {
		
		if (sender.state == .ended) {
			
			if let inverterHistory = InverterHistoryViewController.create() {
				self.show(inverterHistory, sender: self)
			}
		}
	}
	
	func didDismissSetupViewController(_ sender:SetupViewController){
		self.refreshView();
	}
	
	func displaySetupView(){
		
		if let setupView = SetupViewController.create() {
			self.show(setupView, sender: self)
		}
	}

}


 
 
