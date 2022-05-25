//
//  BoyiaSettingMenu.swift
//  app
//
//  Created by yanbo on 2022/5/12.
//

import SwiftUI

struct SettingConstants {
    static let SETTING_WIDTH = PixelRatio.dp(320)
    static let DEFAULT_ICON = "https://img1.baidu.com/it/u=4216761644,15569246&fm=253&fmt=auto&app=120&f=JPEG?w=500&h=500"
}

struct BoyiaSettingMenu : View {
    @EnvironmentObject var loginModel: BoyiaLoginModel
    
    var body: some View {
        VStack(alignment: .center, spacing: 0) {
            VStack {
                marginTop(top: PixelRatio.dp(108))
                AsyncImage(url: URL(string: loginModel.isLogin
                                    ? (BoyiaLoginInfo.shared.user?.avatar ?? "null")
                                    : SettingConstants.DEFAULT_ICON)!) { phase in
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
                
                marginTop(top: PixelRatio.dp(8))
                Text(loginModel.isLogin
                     ? (BoyiaLoginInfo.shared.user?.nickname ?? "null")
                     : "Anonymous")
                
                marginTop(top: PixelRatio.dp(12))
            }
            .frame(width: SettingConstants.SETTING_WIDTH)
            .background(Color(hex: 0xFFFFFF))
            //.padding(.top, PixelRatio.dp(108))
            //.background(Color(hex: 0xFF00FF))
            marginTop(top: PixelRatio.dp(32))
            if !loginModel.isLogin {
                HStack {
                    NavigationLink(destination: LoginView(), label: {
                        Text("Login").foregroundColor(Color(hex: 0x000000))
                    })
                }
                .frame(width: SettingConstants.SETTING_WIDTH, height: PixelRatio.dp(64))
                .background(Color(hex: 0xFFFFFF))
                
                marginTop(top: PixelRatio.dp(1))
            }
            
            if loginModel.isLogin {
                Button(action: {
                    loginModel.logout()
                }) {
                    HStack {
                        Text("Logout").foregroundColor(Color(hex: 0x000000))
                    }
                    .frame(width: SettingConstants.SETTING_WIDTH, height: PixelRatio.dp(64))
                    .background(Color(hex: 0xFFFFFF))
                }
            }
            
            Spacer()
        }
        .frame(
            width: SettingConstants.SETTING_WIDTH,
            height: PixelRatio.screenHeight())
        //.background(Color(hex: 0xFF00FF))
        .background(Color(hex: 0xEEE9E9))
    }
    
    func marginTop(top: Double) -> some View {
        return VStack{}.frame(width: 0, height: top, alignment: .top);
    }
}
