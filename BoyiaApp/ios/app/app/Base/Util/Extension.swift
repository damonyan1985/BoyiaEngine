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

