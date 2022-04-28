//
//  HomeView.swift
//  app
//
//  Created by yanbo on 2022/3/24.
//

import SwiftUI

struct HomeView : View {
    @ObservedObject var model = BoyiaAppListModel()
    var columns: [GridItem] = //[GridItem(.adaptive(minimum: 50)), GridItem(.adaptive(minimum: 50))]
            Array(repeating: .init(.flexible()), count: 3)
    
    var body: some View {
        NavigationView {
            // 类似flutter中的stack
            ZStack {
//                Button(action: {
//                    model.requestAppList()
//                    BoyiaNavigator.push(view: LoginView())
//                }, label: {
//                    Text("Test")
//                })
                getAppListView()
            }
            .frame(width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height)
            .background(Color(Color.RGBColorSpace.sRGB, red: 0.3, green: 0.3, blue: 0.3, opacity: 1))
            .edgesIgnoringSafeArea(.all)
        }
        .navigationBarHidden(true)
    }
    
    func getAppListView() -> some View {
        return LazyVGrid(columns: columns, alignment: .center, spacing: 10) {
            ForEach(model.appList, id: \.id) { item in
                getAppItemView(item: item)
            }
        }
    }
    
    func getAppItemView(item: BoyiaAppItem) -> some View {
        return VStack(alignment: .leading, spacing: 0) {
            
            AsyncImage(url: URL(string: item.cover)!) { phase in
                if let image = phase.image {
                    image.resizable()
                        .transition(.slide)
                        .frame(width: 100, height: 120, alignment: .top)
                } else if phase.error != nil {
                    Text("path: \(item.cover), error: \(phase.error!.localizedDescription) ")
                } else {
                    ProgressView()
                }
            
            }
            Text(item.name).frame(width: 100, height: 40)
        }
        .frame(width: 100, height: 160)
        .background(Color.blue)
        .onTapGesture {
            BoyiaNavigator.push(controller: BoyiaViewController())
        }
    }
}
