//
//  SetupViewController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/24/21.
//

import Foundation
import UIKit
import Toast
 
//
//class StatusViewController: UIViewController {
//
//	@IBOutlet var lblVersion	: UILabel!
//	@IBOutlet var lblStatus	: UILabel!
//	@IBOutlet var lblUptime	: UILabel!
//
//	var upTimeFormatter: DateComponentsFormatter {
//		let formatter = DateComponentsFormatter()
//		formatter.allowedUnits = [.day, .hour, .minute, .second]
//		formatter.unitsStyle = .short
//		return formatter
//	}
//
//	public func refreshView() {
//
//		if(	AppData.serverInfo.validated){
//
//			PumpHouse.shared.fetchData(.status) { result in
//
//				if case .success(let status as RESTStatus) = result {
//					self.lblStatus.text = status.stateString
//					self.lblVersion.text = status.version
//
//					let upTimeStr = self.upTimeFormatter.string(from: TimeInterval(status.uptime))!
//					self.lblUptime.text = upTimeStr
//
//				}
//				else {
//					self.lblUptime.text = ""
//					self.lblStatus.text = ""
//					self.lblVersion.text = ""
//					}
//			}
//
//		}
//
//	}
//}
//
//protocol SetupViewControllerDelegate {
//	 func didDismissSetupViewController(_ sender:SetupViewController)
//}

class SetupViewController: UIViewController,
									UITextFieldDelegate,
									UIAdaptivePresentationControllerDelegate {

	enum loginState {
		case loading, success, failed, disconnected
	}
 	@IBOutlet var txtServerHost	: UITextField!
	@IBOutlet var txtServerPort	: UITextField!
	@IBOutlet var txtKey			: UITextField!
	@IBOutlet var txtSecret		: UITextField!
	@IBOutlet var btnConnect		: BHButton!
	@IBOutlet var btnSecured		: UIButton!
	
//	var svc : StatusViewController?
 	var timer = Timer()
	var isSecured: Bool = true
	
	private var connectState = loginState.disconnected

	class func create() -> SetupViewController? {
		let storyboard = UIStoryboard(name: "SetupView", bundle: nil)
		let vc = storyboard.instantiateViewController(withIdentifier: "SetupViewController") as? SetupViewController
		
		return vc
	}
 
	
	override func viewDidLoad() {
		super.viewDidLoad()
		self.presentationController?.delegate = self
}
	
	func presentationControllerShouldDismiss(_ presentationController: UIPresentationController) -> Bool {
			 return connectState == loginState.success
		}

	
	func textFieldShouldReturn(_ textField: UITextField) -> Bool {
		 textField.resignFirstResponder()
		 return true
	}

	
	private func refreshView() {
		
		if(connectState == loginState.success){
			
			self.btnConnect.setTitle("Disconnect", for: .normal)
		}
		else {
			
			self.btnConnect.setTitle("Connect", for: .normal)
		}
		
	}
	
		
	override func viewWillDisappear(_ animated: Bool) {
		super.viewWillDisappear(animated)
		stopPollng();
	}
	 
	override func viewWillAppear(_ animated: Bool) {
		
		super.viewWillAppear(animated)
	
		let serverHost: String = AppData.serverInfo.url?.host ?? ""
		let port: String = String(AppData.serverInfo.url?.port ?? 8080)
		let apiKey: String =  AppData.serverInfo.apiKey ?? ""
		let apiSecret: String = AppData.serverInfo.apiSecret ?? ""
 
		txtServerHost.text = serverHost;
		txtServerPort.text = port;
		txtKey.text = apiKey;
		txtSecret.text = apiSecret;
		isSecured = true
		refreshSecureText()
	
		connectState =  AppData.serverInfo.validated ? loginState.success: loginState.disconnected

		if(connectState	 == .success) 	{
			startPolling();
		}
		else {
			stopPollng();
		}
		
		refreshView()
	}
	
	@IBAction func btnSecuredClicked(_ sender: Any) {
		
		isSecured = !isSecured
		 refreshSecureText()
	}

	func refreshSecureText() {
		let  image = isSecured ? UIImage(named: "Eye_Closed_Template")  :UIImage(named: "Eye_Open_Template")
		btnSecured.setImage(image, for: .normal)
		
		txtSecret.isSecureTextEntry = isSecured

	}
	
	
	@IBAction func btnConnectClicked(_ sender: Any) {

		self.stopPollng();

		if(connectState == loginState.success){
			self.connectState = loginState.disconnected
			AppData.serverInfo.validated = false
			refreshView()
			return
		}
		
///		hideKeyboard()
		
		self.view.endEditing(true)

		if let url =  URL(string:"http://\(txtServerHost.text ?? "localhost"):\(txtServerPort.text ?? "8081")") {
			AppData.serverInfo.url = url
		}

		AppData.serverInfo.apiKey  = txtKey.text
		AppData.serverInfo.apiSecret  = txtSecret.text
	
		self.connectState = loginState.loading
		self.btnConnect.isHidden = true
		
		PumpHouse.shared.fetchData(.status) { result in
			
			switch result {
			case .success( _ as RESTStatus):
				
				self.connectState = loginState.success
				AppData.serverInfo.validated = true
				
				self.dismiss(animated: true)

//				self.startPolling();
//				self.refreshView()
				break
				
			case .success(let status as RESTError):
				self.connectState = loginState.failed
				AppData.serverInfo.validated = false
		 
				Toast.text(status.error.message,
											  config: ToastConfiguration(
												autoHide: true,
												displayTime: 1.0
//												attachTo: self.vwError
											  )).show()
				
 
				
				break;
				
			case .success(_):
				assert(true, "unknown condition")
				break;
				
				
			case .failure(let error):
	//			print(error.localizedDescription)
				
				self.connectState = loginState.failed
				AppData.serverInfo.validated = false
		 
				Toast.text("Failed",
							  subtitle:error.localizedDescription,
											  config: ToastConfiguration(
												autoHide: true,
												displayTime: 1.0
//			 									attachTo: self.vwError
											  )).show()

			}
			
			self.btnConnect.isHidden = false

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
