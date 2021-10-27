//
//  Utilities.swift
//  Home Control
//
//  Created by Vincent Moscaritolo on 7/9/21.
//

import Foundation
import SwiftUI


#if canImport(UIKit)
extension View {
	 func hideKeyboard() {
		  UIApplication.shared.sendAction(#selector(UIResponder.resignFirstResponder), to: nil, from: nil, for: nil)
	 }
}
#endif

struct CopyableText: ViewModifier {
	
	var text: String
	func body(content: Content) -> some View {
		content
			.contextMenu {
				Button(action: {
					#if os(iOS)
					UIPasteboard.general.string = text
					#endif
				}) {
					Text("Copy to clipboard")
					Image(systemName: "doc.on.doc")
				}
			}
	}
}
extension View {
	func CopyableTextStyle(_ text: String) -> some View {
		self.modifier(CopyableText(text:text))
	}
}


struct CustomScrollView<ROOTVIEW>: UIViewRepresentable where ROOTVIEW: View {
	 
	 var width : CGFloat, height : CGFloat
	 let handlePullToRefresh: () -> Void
	 let rootView: () -> ROOTVIEW
	 
	 func makeCoordinator() -> Coordinator<ROOTVIEW> {
		  Coordinator(self, rootView: rootView, handlePullToRefresh: handlePullToRefresh)
	 }

	 func makeUIView(context: Context) -> UIScrollView {
		  let control = UIScrollView()
		  control.refreshControl = UIRefreshControl()
		  control.refreshControl?.addTarget(context.coordinator, action:
				#selector(Coordinator.handleRefreshControl),
														for: .valueChanged)

		  let childView = UIHostingController(rootView: rootView() )
		  childView.view.frame = CGRect(x: 0, y: 0, width: width, height: height)
		  
		  control.addSubview(childView.view)
		  return control
	 }

	 func updateUIView(_ uiView: UIScrollView, context: Context) {}

	 class Coordinator<ROOTVIEW>: NSObject where ROOTVIEW: View {
		  var control: CustomScrollView
		  var handlePullToRefresh: () -> Void
		  var rootView: () -> ROOTVIEW

		  init(_ control: CustomScrollView, rootView: @escaping () -> ROOTVIEW, handlePullToRefresh: @escaping () -> Void) {
				self.control = control
				self.handlePullToRefresh = handlePullToRefresh
				self.rootView = rootView
		  }

		  @objc func handleRefreshControl(sender: UIRefreshControl) {

				sender.endRefreshing()
				handlePullToRefresh()
			  
		  }
	 }
}

 
extension StringProtocol {
	 subscript(offset: Int) -> Character {
		  self[index(startIndex, offsetBy: offset)]
	 }
}

extension String {
var boolValue: Bool {
	 return (self as NSString).boolValue
}}
