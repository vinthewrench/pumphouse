//
//  DateExtensions.swift
//  Home Control
//
//  Created by Vincent Moscaritolo on 7/1/21.
//

import Foundation


//func convertDateFormatter(date: String) -> String {
//	 let dateFormatter = DateFormatter()
//	 dateFormatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss"//this your string date format
//	dateFormatter.timeZone = NSTimeZone(name: "UTC") as TimeZone?
//	 dateFormatter.locale = Locale(identifier: "your_loc_id")
//	 let convertedDate = dateFormatter.date(from: date)
//
//	 guard dateFormatter.date(from: date) != nil else {
//		  assert(false, "no date from string")
//		  return ""
//	 }
//
//	 dateFormatter.dateFormat = "yyyy MMM HH:mm EEEE"///this is what you want to convert format
//	dateFormatter.timeZone = NSTimeZone(name: "UTC") as TimeZone?
//	 let timeStamp = dateFormatter.string(from: convertedDate!)
//
//	 return timeStamp
//}

let RFC1123dateFormatter: DateFormatter = {
	  let dateFormatter = DateFormatter()
	  dateFormatter.dateFormat = "EEE MMM d HH:mm:ss yyyy"
//	"%a, %d %b %Y %T GMT";
	  return dateFormatter
  }()
