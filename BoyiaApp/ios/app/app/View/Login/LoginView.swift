//
//  LoginView.swift
//  登录模块主界面
//  使用系统图标构建app，SF Symbols
//  Created by yanbo on 2022/3/24.
//

import SwiftUI

// 下划线样式
public struct UnderlineTextFieldStyle : TextFieldStyle {
    public func _body(configuration: TextField<Self._Label>) -> some View {
        configuration
            .font(.callout)
            .padding(.bottom, 10)
            .background(
                UnderLine()
                    .stroke(Color(hex: 0xCAE1FF), lineWidth: /*@START_MENU_TOKEN@*/1.0/*@END_MENU_TOKEN@*/)
                    .frame(alignment: .bottom)
            )
    }
}

// 下划线
struct UnderLine: Shape {
    func path(in rect: CGRect) -> Path {
        var path = Path()
        
        path.move(to: CGPoint(x: rect.minX, y: rect.maxY))
        path.addLine(to: CGPoint(x: rect.maxX, y: rect.maxY))
        return path
    }
}

// 应用登录界面
struct LoginView : View {
    @State var account: String = ""
    @State var password: String = ""
    
    var body: some View {
//        Image(systemName: "person")
//            .foregroundColor(Color.white)
        NavigationView {
            // 纵向，类似flutter中的column
            VStack {
                // 横向，类似flutter中的row
                HStack {
                    Image(systemName: "person")
                        .foregroundColor(Color.white)
                    //Spacer(minLength: 20)
                    TextField("User:", text: $account, onCommit: {
                        
                    })
                        //.labelStyle(ShadowLabelStyle())
                        .frame(width: 200, height: 60, alignment: Alignment.leading)
                        .textFieldStyle(UnderlineTextFieldStyle())
                        .foregroundColor(Color.white)
                }.frame(width: 300, alignment: Alignment.bottom)
                Divider()
                HStack {
                    Image(systemName: "lock")
                        .foregroundColor(Color.white)
                    TextField("Password:", text: $password, onCommit: {})
                        .frame(width: 200, height: 60, alignment: Alignment.leading)
                        .foregroundColor(Color.white)
                        .textFieldStyle(UnderlineTextFieldStyle())
                    
                }.frame(width: 300)
            }
            .frame(width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height)
            .background(Color(Color.RGBColorSpace.sRGB, red: 0.3, green: 0.3, blue: 0.3, opacity: 1))
            .edgesIgnoringSafeArea(.all)
            
        }
        .navigationBarHidden(true) // 隐藏导航栏
    }
}
