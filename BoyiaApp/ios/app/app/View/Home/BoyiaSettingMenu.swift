//
//  BoyiaSettingMenu.swift
//  app
//
//  Created by yanbo on 2022/5/12.
//

import SwiftUI

struct BoyiaSettingMenu : View {
    var body: some View {
        VStack(alignment: .center, spacing: 0) {
            marginTop(top: PixelRatio.dp(108))
            AsyncImage(url: URL(string: "https://img1.baidu.com/it/u=4216761644,15569246&fm=253&fmt=auto&app=120&f=JPEG?w=500&h=500")!) { phase in
                if let image = phase.image {
                    image.resizable()
                        .transition(.slide)
//                        .frame(width: PixelRatio.dp(36),
//                               height: PixelRatio.dp(36),
//                               alignment: .top)
                } else if phase.error != nil {
                    //Text("path: \(item.cover), error: \(phase.error!.localizedDescription) ")
                    ProgressView()
                } else {
                    ProgressView()
                }
            
            }
            .frame(width: PixelRatio.dp(108),
                    height: PixelRatio.dp(108),
                    alignment: .top)
            .cornerRadius(PixelRatio.dp(54))
            //.padding(.top, PixelRatio.dp(108))
            //.background(Color(hex: 0xFF00FF))
            
            
            
            Spacer()
        }
        .frame(
            width: PixelRatio.dp(200),
            height: PixelRatio.screenHeight())
        //.background(Color(hex: 0xFF00FF))
        .background(Color(hex: 0xEDEDED))
    }
    
    func marginTop(top: Double) -> some View {
        return VStack{}.frame(width: 0, height: top, alignment: .top);
    }
}
