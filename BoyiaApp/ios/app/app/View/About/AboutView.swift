//
//  AboutView.swift
//  app
//
//  Created by yanbo on 2022/6/6.
//

import Foundation
import SwiftUI

struct AboutView : View {
    var body: some View {
        ZStack {
            VStack(alignment: .center, spacing: 0) {
                VStack {
                    Image(uiImage: UIImage(named: appIconName())!)
                        .resizable()
                        .frame(width: 108.dp, height: 108.dp, alignment: .top)
                    
                    Spacer()
                }
                .padding(Edge.Set.top, 60.dp)
                .frame(width: PixelRatio.screenWidth(),
                            height: 360.dp)
                
                buildButton(text: "Version update", action: {
                    HttpUtil.checkVersion()
                })
                
                marginTop(top: 1.dp)
                buildButton(text: "Feature introduction", action: {
                    HttpUtil.upload(path: "/Users/yanbo/Library/Developer/Xcode/DerivedData/Boyia-fcdfmyhktgurjubvszoklymjqfym/Build/Products/Debug-iphonesimulator/core.framework/metal.bundle/boyia.json")
                })
                
                marginTop(top: 1.dp)
                buildButton(text: "Register", action: {})
                
                marginTop(top: 1.dp)
                buildButton(text: "Feedback", action: {})

                Spacer()
            }.background(Color(argb: 0x08000000))
        }.background(Color(hex: 0xFFFFFF))
    }
    
    func buildButton(text: String, action: @escaping () -> Void) -> some View {
        Button(action: action) {
            HStack {
                Text(text)
                    .bold()
                    .foregroundColor(Color(hex: 0x000000))
            }
            .frame(width: PixelRatio.screenWidth(), height: 84.dp)
            .background(Color(hex: 0xFFFFFF))
        }
    }
    
    // 获取应用图标
    func appIconName() -> String {
        let iconInfo = Bundle.main.infoDictionary!["CFBundleIcons"] as! Dictionary<String, Any>
        let appiconInfo = iconInfo["CFBundlePrimaryIcon"] as! Dictionary<String, Any>
        let icons = appiconInfo["CFBundleIconFiles"] as! Array<String>
        BoyiaLog.d("icons = \(icons.last!)")
        return icons.last!
    }
}
