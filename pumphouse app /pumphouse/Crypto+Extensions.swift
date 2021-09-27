//
//  Crypto.swift
//  Home Control
//
//  Created by Vincent Moscaritolo on 7/1/21.
//

import Foundation

import CommonCrypto

extension Data{
	
	public  func hexString() -> String {
		var bytes = [UInt8](repeating: 0, count: self.count)
		(self as NSData).getBytes(&bytes, length: self.count)
		
		var hexString = ""
		for byte in bytes {
			hexString += String(format:"%02x", UInt8(byte))
		}
		return hexString
	}
	
	public func sha256String() -> String{
		return  self.digest().hexString()
	}
	
	private func digest() -> Data {
		let digestLength = Int(CC_SHA256_DIGEST_LENGTH)
		var hash = [UInt8](repeating: 0, count: digestLength)
		
		self.withUnsafeBytes {
				_ = CC_SHA256($0.baseAddress, CC_LONG(self.count), &hash)
		  }
		return  Data(bytes: hash, count: digestLength);
	}
	
}

public extension String {
	 func sha256() -> String{
		  if let stringData = self.data(using: String.Encoding.utf8) {
				return stringData.sha256String()
		  }
		  return ""
	 }
}

public extension String {

	func hmac(key: String) -> String {
	  var digest = [UInt8](repeating: 0, count: Int(CC_SHA256_DIGEST_LENGTH))
	  CCHmac(CCHmacAlgorithm(kCCHmacAlgSHA256), key, key.count, self, self.count, &digest)
	 let data = Data.init(bytes: digest, count: Int(CC_SHA256_DIGEST_LENGTH))
		return data.hexString()
	 }
 }
