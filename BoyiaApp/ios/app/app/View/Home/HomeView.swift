//
//  HomeView.swift
//  app
//
//  Created by yanbo on 2022/3/24.
//

import SwiftUI

struct HomeView : View {
    @ObservedObject var model = BoyiaAppListModel()
    
    var body: some View {
        NavigationView {
            // 类似flutter中的stack
            ZStack {
                Button(action: {
                    model.requestAppList()
                    BoyiaNavigator.push(view: LoginView())
                }, label: {
                    Text("Test")
                })
            }
            .frame(width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height)
            .background(Color(Color.RGBColorSpace.sRGB, red: 0.3, green: 0.3, blue: 0.3, opacity: 1))
            .edgesIgnoringSafeArea(.all)
        }
        .navigationBarHidden(true)
    }
}
