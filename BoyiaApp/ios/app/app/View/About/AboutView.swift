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
        VStack {
            VStack {
                Image(uiImage: UIImage(named: appIconName())!)
                    .resizable()
                    .frame(width: 108.dp, height: 108.dp, alignment: .top)
                
                Spacer()
            }
            .padding(Edge.Set.top, 60.dp)
            .frame(width: PixelRatio.screenWidth(),
                        height: 360.dp)
            
            Button(action: {
                HttpUtil.checkVersion()
            }) {
                HStack {
                    Text("Version update")
                        .bold()
                        .foregroundColor(Color(hex: 0x000000))
                }
                .frame(width: PixelRatio.screenWidth(), height: 84.dp)
                .background(Color(hex: 0xFFFFFF))
            }
            Spacer()
        }.background(Color(argb: 0x08000000))
        
        
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
