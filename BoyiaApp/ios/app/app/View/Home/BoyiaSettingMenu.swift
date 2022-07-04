//
//  BoyiaSettingMenu.swift
//  app
//
//  Created by yanbo on 2022/5/12.
//

import SwiftUI

struct SettingConstants {
    static let SETTING_WIDTH = 320.dp
    static let DEFAULT_ICON = "https://img1.baidu.com/it/u=4216761644,15569246&fm=253&fmt=auto&app=120&f=JPEG?w=500&h=500"
}

struct BoyiaSettingMenu : View {
    @EnvironmentObject var loginModel: BoyiaLoginModel
    @State private var showImagePicker = false
    
    var body: some View {
        VStack(alignment: .center, spacing: 0) {
            VStack {
                marginTop(top: 108.dp)
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
                .frame(width: 108.dp,
                       height: 108.dp,
                        alignment: .top)
                .cornerRadius(54.dp)
                .onTapGesture {
                    BoyiaPermission.authPhoto(closurer: { (status) in
                        showImagePicker = status
                    })
                }
                
                marginTop(top: 8.dp)
                Text(loginModel.isLogin
                     ? (BoyiaLoginInfo.shared.user?.nickname ?? "null")
                     : "Anonymous").foregroundColor(Color.black)
                
                marginTop(top: 12.dp)
            }
            .frame(width: SettingConstants.SETTING_WIDTH)
            .background(Color(hex: 0xFFFFFF))
            //.padding(.top, PixelRatio.dp(108))
            //.background(Color(hex: 0xFF00FF))
            marginTop(top: 32.dp)
            if !loginModel.isLogin {
                HStack {
                    NavigationLink(destination: LoginView(), label: {
                        Text("LOGIN")
                            .bold()
                            .foregroundColor(Color(hex: 0x000000))
                    })
                }
                .frame(width: SettingConstants.SETTING_WIDTH, height: 64.dp)
                .background(Color(hex: 0xFFFFFF))
                
                marginTop(top: 1.dp)
            }
            
            if loginModel.isLogin {
                Button(action: {
                    loginModel.logout()
                }) {
                    HStack {
                        Text("LOGOUT")
                            .bold()
                            .foregroundColor(Color(hex: 0x000000))
                    }
                    .frame(width: SettingConstants.SETTING_WIDTH, height: 64.dp)
                    .background(Color(hex: 0xFFFFFF))
                }
                marginTop(top: 1.dp)
            }
            
//            Button(action: {
//            }) {
//                HStack {
//                    Text("ABOUT")
//                        .bold()
//                        .foregroundColor(Color(hex: 0x000000))
//                }
//                .frame(width: SettingConstants.SETTING_WIDTH, height: 64.dp)
//                .background(Color(hex: 0xFFFFFF))
//            }
            HStack {
                NavigationLink(destination: AboutView(), label: {
                    Text("ABOUT")
                        .bold()
                        .foregroundColor(Color(hex: 0x000000))
                })
            }
            .frame(width: SettingConstants.SETTING_WIDTH, height: 64.dp)
            .background(Color(hex: 0xFFFFFF))
            
            Spacer()
        }
        .frame(
            width: SettingConstants.SETTING_WIDTH,
            height: PixelRatio.screenHeight())
        //.background(Color(hex: 0xFF00FF))
        .background(Color(hex: 0xEEE9E9))
        .sheet(isPresented: $showImagePicker, onDismiss: {}) {
            ImagePicker() { image in
                showImagePicker = false
            }
        }
//        .actionSheet(isPresented: $showImagePicker, content: {
//            HStack() {
//                ImagePicker(sourceType: UIImagePickerController.SourceType.photoLibrary) { image in
//
//                }
//            }
//        })
        
    }
}
