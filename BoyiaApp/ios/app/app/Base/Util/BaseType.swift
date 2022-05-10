//
//  BaseType.swift
//  app
//
//  Created by yanbo on 2022/5/10.
//

import Foundation
import UIKit

class IntObject {
    var value: Int?
    init(_ value: Int) {
        self.value = value
    }
}

class FloatObject {
    var value: Float?
    init(_ value: Float) {
        self.value = value
    }
}

class DoubleObject {
    var value: Double?
    init(_ value: Double) {
        self.value = value
    }
}

class StringObject {
    var value: String?
    init(_ value: String) {
        self.value = value
    }
}

class BoyiaData {
    var id: IntObject?
    required init() {
        id = IntObject(-1)
    }
}
