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
            ZStack(alignment: .topLeading) {
//                Button(action: {
//                    model.requestAppList()
//                    BoyiaNavigator.push(view: LoginView())
//                }, label: {
//                    Text("Test")
//                })
                
                
                
            
                HStack(alignment: .top) {
                    VStack(alignment: .leading) {
                        ScrollView {
                            getAppListView()
                        }
                    }
                }
                .offset(x: 0, y: PixelRatio.dp(value: 200))
                
                HStack {
                    HStack {}
                        .frame(width: PixelRatio.dp(value: 560))
                    
                    Image(systemName: "gearshape")
                        .resizable()
                        .frame(width: PixelRatio.dp(value: 50), height: PixelRatio.dp(value: 50))
                        .foregroundColor(Color.black)
                        .offset(x: 0, y: PixelRatio.dp(value: 26))
                        
                        //.alignmentGuide(.right, computeValue: { $0[.trailing] })
                        //.alignmentGuide(.trailing, computeValue: { d in d[.trailing] - 5})
                }
                .frame(width: UIScreen.main.bounds.width, height: PixelRatio.dp(value: 160))
                .background(Color(hex: 0xEDEDED))
            }
            .frame(width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height)
            .background(Color(Color.RGBColorSpace.sRGB, red: 0.3, green: 0.3, blue: 0.3, opacity: 1))
            .edgesIgnoringSafeArea(.all)
        }
        .navigationBarHidden(true)
    }
    
    func getAppListView() -> some View {
        return LazyVGrid(columns: columns, alignment: .center, spacing: PixelRatio.dp(value: 10)) {
            ForEach(model.appList, id: \.id) { item in
                getAppItemView(item: item)
            }
        }
    }
    
    func getAppItemView(item: BoyiaAppItem) -> some View {
        let width = PixelRatio.dp(value: 224)
        return VStack(alignment: .leading, spacing: 0) {
            
            AsyncImage(url: URL(string: item.cover)!) { phase in
                if let image = phase.image {
                    image.resizable()
                        .transition(.slide)
                        .frame(width: width,
                               height: PixelRatio.dp(value: 224),
                               alignment: .top)
                } else if phase.error != nil {
                    Text("path: \(item.cover), error: \(phase.error!.localizedDescription) ")
                } else {
                    ProgressView()
                }
            
            }
            Text(item.name).frame(width: width, height: PixelRatio.dp(value: 36))
        }
        .frame(width: width, height: PixelRatio.dp(value: 260))
        .background(Color(hex: 0xEDEDED))
        .onTapGesture {
            BoyiaNavigator.push(controller: BoyiaViewController())
        }
    }
}
