//
//  URLResponse+Extensions.swift
//  Home Control
//
//  Created by Vincent Moscaritolo on 7/1/21.
//

import Foundation

public extension URLResponse{
	
	enum httpStatus: Int {
		case STATUS_OK   = 200
		case STATUS_NO_CONTENT = 204
		case STATUS_NOT_MODIFIED = 304
		case STATUS_BAD_REQUEST = 400
		case STATUS_ACCESS_DENIED = 401
		case STATUS_INVALID_BODY = 4006
		case STATUS_NOT_FOUND = 404
		case STATUS_INVALID_METHOD = 405
		case STATUS_CONFLICT = 409
		case STATUS_INTERNAL_ERROR = 500
		case STATUS_NOT_IMPLEMENTED = 501
		case STATUS_UNAVAILABLE = 503
		}

	func httpStatusCode() -> Int {
		var statusCode: Int = 0
		
		if let httpResponse = self as? HTTPURLResponse {
			statusCode = httpResponse.statusCode
		}
		return statusCode
	}
	
	func eTag() -> String? {
		var etag: String?
		
		if let httpResponse = self as? HTTPURLResponse {
			if let val = httpResponse.allHeaderFields["ETag"] as? String {
				etag = val.replacingOccurrences(of: "\"", with: "")
			}
		}
		return etag;
	}
	
	func lastModified() -> Date? {
		var date: Date?
		
		if let httpResponse = self as? HTTPURLResponse {
			if let val = httpResponse.allHeaderFields["Last-Modified"] as? String {
				date = RFC1123dateFormatter.date(from: val)
			}
		}

		return date
	}
	
	func httpStatusCodeString() -> String {
		var text: String = ""
		
		let code = self.httpStatusCode()
		
		switch (code) {
		case 200: text = "200 - OK"
		case 204: text = "204 - No Content"
		case 304: text = "304 - Not Modified"
		case 401: text = "401 - Access denied"
		case 400: text = "400 - Bad Request"
		case 404: text = "404 - Not found"
		case 4066: text = "400.6 - Invalid Request Body"
		case 409: text = "409 - Conflict"
		case 405: text = "405 - Method Not Allowed"
		case 501: text = "501 - Not Implemented"
		case 503: text = "503 - Service unavailable"
		case 500: text = "500 - Internal server error"
		default:
			text = "Error Code: \(code)"
		}
		return text
	}
	
}
