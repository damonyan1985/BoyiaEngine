//
//  Extension.swift
//  app
//
//  Created by yanbo on 2022/3/24.
//

import SwiftUI
import CryptoKit
import CommonCrypto

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
    
    init(argb: UInt) {
        let components = (
            A: Double((argb >> 24) & 0xff) / 255,
            R: Double((argb >> 16) & 0xff) / 255,
            G: Double((argb >> 08) & 0xff) / 255,
            B: Double((argb >> 00) & 0xff) / 255
        )
        self.init(
            .sRGB,
            red: components.R,
            green: components.G,
            blue: components.B,
            opacity: components.A
        )
    }
}

// UIColor扩展颜色
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

// 扩展string下划线驼峰互转功能
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
    
    // 获取string md5值
    public var md5: String {
        guard let data = self.data(using: .utf8) else { return "" }
        return Insecure.MD5
            .hash(data: data)
            .map{String(format: "%02x", $0)}
            .joined()
    }
    
    // 当前字符串为文件路径
    public var fileMd5: String? {
        let url = URL(fileURLWithPath: self);
        
        let bufferSize = 1024 * 1024
        do {
            //打开文件
            let file = try FileHandle(forReadingFrom: url)
            defer {
                file.closeFile()
            }
            
            //初始化内容
            var context = CC_MD5_CTX()
            CC_MD5_Init(&context)
            
            //读取文件信息
            while case let data = file.readData(ofLength: bufferSize), data.count > 0 {
                data.withUnsafeBytes {
                    _ = CC_MD5_Update(&context, $0, CC_LONG(data.count))
                }
            }
            
            //计算Md5摘要
            var digest = Data(count: Int(CC_MD5_DIGEST_LENGTH))
            digest.withUnsafeMutableBytes {
                _ = CC_MD5_Final($0, &context)
            }
            
            return digest.map { String(format: "%02hhx", $0) }.joined()
            
        } catch {
            BoyiaLog.d("Cannot open file: \(error.localizedDescription)")
            return nil
        }
    }
}

// swiftui添加侧滑
extension UINavigationController: UIGestureRecognizerDelegate {
    override open func viewDidLoad() {
        super.viewDidLoad()
        interactivePopGestureRecognizer?.delegate = self
    }
    
    public func gestureRecognizerShouldBegin(_ gestureRecognizer: UIGestureRecognizer) -> Bool {
        return viewControllers.count > 1
    }
}

// 使用userdefaults存储对象
extension UserDefaults {
    func setItem<T: Encodable>(obj: T, key: String) {
        do {
            let data = try JSONEncoder().encode(obj)
            self.set(data, forKey: key)
        } catch {
            BoyiaLog.d("UserDefaults", error)
        }
    }
    
    func getItem<T: Decodable>(key: String) -> T? {
        guard let data = self.data(forKey: key) else {
            return nil
        }
        
        do {
            return try JSONDecoder().decode(T.self, from: data)
        } catch {
            BoyiaLog.d("UserDefaults", error)
        }
        
        return nil
    }
}

extension Int {
    var dp: Double {
        get {
            return PixelRatio.dp(self)
        }
    }
}

extension Double {
    var dp: Double {
        get {
            return PixelRatio.dp(self)
        }
    }
}
