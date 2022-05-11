//
//  Extension.swift
//  app
//
//  Created by yanbo on 2022/3/24.
//

import SwiftUI

// 处理所有基础扩展

// 扩展颜色
extension Color {
    init(hex: Int, alpha: Double = 1) {
        let components = (
            R: Double((hex >> 16) & 0xff) / 255,
            G: Double((hex >> 08) & 0xff) / 255,
            B: Double((hex >> 00) & 0xff) / 255
        )
        self.init(
            .sRGB,
            red: components.R,
            green: components.G,
            blue: components.B,
            opacity: alpha
        )
    }
}

extension UIColor {
    convenience init(hex: Int, alpha: CGFloat = 1) {
        let red = (CGFloat((hex & 0xFF0000) >> 16)) / 255
        let green = (CGFloat((hex & 0xFF00) >> 8)) / 255
        let blue = (CGFloat(hex & 0xFF)) / 255
        self.init(red: red, green: green, blue: blue, alpha: alpha)
    }
}

extension HorizontalAlignment {
  struct RightAlignment: AlignmentID {
    static func defaultValue(in context: ViewDimensions) -> CGFloat {
      return context[.trailing]
    }
  }
  
  static let right = HorizontalAlignment(RightAlignment.self)
}

extension HorizontalAlignment {
    private enum HAlignment: AlignmentID {
        static func defaultValue(in dimensions: ViewDimensions) -> CGFloat {
            return dimensions[HorizontalAlignment.leading]
        }
    }
    static let myHAlignment = HorizontalAlignment(HAlignment.self)
}

extension String {
    // 下划线转驼峰
    public var camelName:String {
        var result = ""
        var flag = false
        self.forEach { c in
            let s = String(c)
            if s == "_" {
                flag = true
                return
            }
            
            if flag {
                result += s.uppercased()
                flag = false
            } else {
                result += s
            }
        }
        return result
    }
    
    // 驼峰转下划线，主要是用于本地数据库
    public var underName: String {
        var result = ""
        self.forEach { c in
            let num = c.unicodeScalars.map { $0.value }.last!
            let s = String(c)
            if num > 64 && num < 91 {
                result += "_"
                result += s.lowercased()
            } else {
                result += s
            }
        }
        return result
    }
}

