//
//  MainViewController.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 9/24/21.
//

import Foundation
import UIKit

class MainViewController: UITabBarController, UITabBarControllerDelegate {
	 override func viewDidLoad() {
		  super.viewDidLoad()
		  delegate = self
		
		
		self.selectedIndex = AppData.serverInfo.tabSelection

	 }

 
	 override func viewWillAppear(_ animated: Bool) {
		  super.viewWillAppear(animated)
		
			
//		  let item1 = Item1ViewController()
//		  let icon1 = UITabBarItem(title: "Title", image: UIImage(named: "someImage.png"), selectedImage: UIImage(named: "otherImage.png"))
//		  item1.tabBarItem = icon1
//		  let controllers = [item1]  //array of the root view controllers displayed by the tab bar interface
//		  self.viewControllers = controllers
	 }

	 //Delegate methods
//	 func tabBarController(_ tabBarController: UITabBarController, shouldSelect viewController: UIViewController) -> Bool {
//		  print("Should select viewController: \(viewController.title ?? "") ?")
//		  return true;
//	 }
//
	func tabBarController(_ tabBarController: UITabBarController, didSelect viewController: UIViewController) {
		
		AppData.serverInfo.tabSelection  = self.selectedIndex
	}
}
