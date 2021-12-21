//
//  UIButton+Extensions.swift
//  pumphouse
//
//  Created by Vincent Moscaritolo on 11/22/21.
//

import UIKit

final class ExtendedHitButton: UIButton {
	 
	 override func point( inside point: CGPoint, with event: UIEvent? ) -> Bool {
		  let relativeFrame = self.bounds
		  let hitTestEdgeInsets = UIEdgeInsets(top: -44, left: -44, bottom: -44, right: -44) // Apple recommended hit target
		  let hitFrame = relativeFrame.inset(by: hitTestEdgeInsets)
		  return hitFrame.contains( point );
	 }
}
