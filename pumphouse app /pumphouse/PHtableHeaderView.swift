//
//  PHtableHeaderView.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 10/26/21.
//

import Foundation
import UIKit


final class PHtableHeaderView: UITableViewHeaderFooterView {
	 static let reuseIdentifier: String = String(describing: self)

	 static var nib: UINib {
		  return UINib(nibName: String(describing: self), bundle: nil)
	 }

	 @IBOutlet var title: UILabel? {
		  get { return _title }
		  set { _title = newValue }
	 }
	 private var _title: UILabel?
}
 
