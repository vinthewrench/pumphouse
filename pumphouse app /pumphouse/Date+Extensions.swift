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

extension Date {

	 func years(sinceDate: Date) -> Int? {
		  return Calendar.current.dateComponents([.year], from: sinceDate, to: self).year
	 }

	 func months(sinceDate: Date) -> Int? {
		  return Calendar.current.dateComponents([.month], from: sinceDate, to: self).month
	 }

	 func days(sinceDate: Date) -> Int? {
		  return Calendar.current.dateComponents([.day], from: sinceDate, to: self).day
	 }

	 func hours(sinceDate: Date) -> Int? {
		  return Calendar.current.dateComponents([.hour], from: sinceDate, to: self).hour
	 }

	 func minutes(sinceDate: Date) -> Int? {
		  return Calendar.current.dateComponents([.minute], from: sinceDate, to: self).minute
	 }

	 func seconds(sinceDate: Date) -> Int? {
		  return Calendar.current.dateComponents([.second], from: sinceDate, to: self).second
	 }

}

extension Calendar {
	 func numberOfDaysBetween(_ from: Date, and to: Date) -> Int {
		  let fromDate = startOfDay(for: from) // <1>
		  let toDate = startOfDay(for: to) // <2>
		  let numberOfDays = dateComponents([.day], from: fromDate, to: toDate) // <3>
		  
		  return numberOfDays.day!
	 }
}
