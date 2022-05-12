//
//  HomeView.swift
//  app
//
//  Created by yanbo on 2022/3/24.
//

import SwiftUI

struct HomeView : View {
    @ObservedObject var model = BoyiaAppListModel()
    
    @State var offset: CGFloat = 0
    
    var columns: [GridItem] = //[GridItem(.adaptive(minimum: 50)), GridItem(.adaptive(minimum: 50))]
            Array(repeating: .init(.flexible()), count: 3)
    
    var body: some View {
        ZStack(alignment: .leading) {
            
            buildContent()
//                .overlay(
//                    Group {
//                        if showSidebar {
//                            Color.white
//                                .opacity(showSidebar ? 0.01 : 0)
//                                .onTapGesture {
//                                    self.showSidebar = false
//                                }
//                        } else {
//                            Color.clear
//                            .opacity(showSidebar ? 0 : 0)
//                            .onTapGesture {
//                                self.showSidebar = false
//                            }
//                        }
//                    }
//                )
                .offset(x: offset, y: 0)
                .animation(.default, value: offset)
                
            BoyiaSettingMenu()
                .offset(x: offset - PixelRatio.dp(200), y: 0)
                .animation(.default, value: offset)
        }
    }
    
    func getAppListView() -> some View {
        return LazyVGrid(columns: columns, alignment: .center, spacing: PixelRatio.dp(10)) {
            ForEach(model.appList, id: \.id) { item in
                BoyiaAppItemView(data: item)
            }
        }
    }
    
    func buildContent() -> some View {
        // 类似flutter中的stack
        return ZStack(alignment: .topLeading) {
                // middle
                HStack(alignment: .top) {
                    VStack(alignment: .leading) {
                        ScrollView {
                            getAppListView()
                        }
                    }
                }
                .offset(x: 0, y: PixelRatio.dp(200))
                .background(Color(hex: 0xFF00FF))
                
                // header
                HStack {
                    HStack {}
                        .frame(width: PixelRatio.dp(560))
                    
                    Image(systemName: "gearshape")
                        .resizable()
                        .frame(width: PixelRatio.dp(50), height: PixelRatio.dp(50))
                        .foregroundColor(Color.black)
                        .offset(x: 0, y: PixelRatio.dp(26))
                        .onTapGesture {
                            offset = PixelRatio.dp(200)
                        }
                        
                        //.alignmentGuide(.right, computeValue: { $0[.trailing] })
                        //.alignmentGuide(.trailing, computeValue: { d in d[.trailing] - 5})
                }
                .frame(width: UIScreen.main.bounds.width, height: PixelRatio.dp(160))
                .background(Color(hex: 0xEDEDED))
            }
            .frame(width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height)
            .background(Color(Color.RGBColorSpace.sRGB, red: 0.3, green: 0.3, blue: 0.3, opacity: 1))
            .edgesIgnoringSafeArea(.all)
            .navigationBarHidden(true)
    }
}
