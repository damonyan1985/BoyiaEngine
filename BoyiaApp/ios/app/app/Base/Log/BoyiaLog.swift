//
//  BoyiaLog.swift
//  app
//
//  Created by yanbo on 2022/3/23.
//

import Foundation

class BoyiaLog {
    static func d(_ tag: String, log: Any...) {
        print("\(tag): \(log)")
    }
    
    static func d(_ log: Any...) {
        print(log)
    }
}
