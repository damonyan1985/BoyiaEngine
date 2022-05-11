//
//  PixelRatio.swift
//  app
//
//  Created by yanbo on 2022/5/5.
//

import Foundation

struct RatioContants {
    static let SCREEN_DP_WITH = Double(720)
}

class PixelRatio {
    static func screenWidth() -> Double {
        return UIScreen.main.bounds.width
    }
    
    static func screenHeight() -> Double {
        return UIScreen.main.bounds.height
    }
    
    static func radio() -> Double {
        let width = screenWidth()
        let height = screenHeight()
        if (width > height) {
            return height / RatioContants.SCREEN_DP_WITH
        }
        
        return width / RatioContants.SCREEN_DP_WITH
    }
    
    // 实际占用的像素
    static func dp(_ value: Int) -> Double {
        return dp(Double(value))
    }
    
    static func dp(_ value: Double) -> Double {
        return round(value * radio())
    }
}
