//
//  HomeView.swift
//  app
//
//  Created by yanbo on 2022/3/24.
//

import SwiftUI

struct HomeView : View {
    // @StateObject相比@ObservedObject的好处是，view销毁后StateObject也会销毁
    // 而ObservedObject则不一定会销毁
    @StateObject var model = BoyiaAppListModel()
    
    // @State只针对值类型，不能针对对象类型
    @State var offset: CGFloat = 0
    @State var showSetting = false
    // 将loginModel共享给其他子view
    let loginModel = BoyiaLoginModel()
    
    var columns: [GridItem] = //[GridItem(.adaptive(minimum: 50)), GridItem(.adaptive(minimum: 50))]
            Array(repeating: .init(.flexible()), count: 3)
        
    var body: some View {
        NavigationView {
            ZStack(alignment: .leading) {
                buildContent()
                    .overlay(
                        Color.black
                            .offset(x: 0, y: -PixelRatio.dp(10))
                            .opacity(offset > 0 ? 0.6 : 0)
                            .onTapGesture {
                                offset = 0
                            }
                    )
                    .offset(x: offset, y: 0)
                    .animation(.default, value: offset)
                    
    //            BoyiaSettingMenu()
    //                .offset(x: offset - SettingConstants.SETTING_WIDTH, y: 0)
    //                .animation(.default, value: offset)
                if (showSetting) {
                    BoyiaSettingMenu()
                        .modifier(BoyiaTranslate(offset: offset - SettingConstants.SETTING_WIDTH, onCompletion: {
                            if (offset == 0) {
                                BoyiaLog.d("islogin=\(loginModel.isLogin)")
                                showSetting = false
                            }
                        }))
                        .animation(.spring(), value: offset)
                        .onAppear {
                            DispatchQueue.main.async {
                                offset = SettingConstants.SETTING_WIDTH
                            }
                        }
                }
            }
        }
        .navigationBarHidden(true)
        .environmentObject(loginModel)
    }
    
    func getAppListView() -> some View {
        // spacing元素之间的距离
        return LazyVGrid(columns: columns, alignment: .center, spacing: 10.dp) {
            // foreach \.id不是appitem id, 而是迭代的id
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
                .offset(x: 0, y: 200.dp)
                //.background(Color(hex: 0xFF00FF))
                
                // header
                HStack {
                    NavigationLink(destination: SearchView(), label: {
                        // seach
                        HStack(alignment: VerticalAlignment.center) {
                            Image(systemName: "magnifyingglass")
                                .resizable()
                                .frame(width: 50.dp, height: 50.dp)
                                .foregroundColor(Color.black)
                            Spacer()
                        }
                        .frame(width: 560.dp, height: 70.dp)
                        .padding(Edge.Set.leading, 20.dp)
                        .background(Color.white)
                        .cornerRadius(35.dp) // radius必须在background之后，否则不起效果
                        .offset(x: 0, y: 26.dp)
                    })
                    
                    Image(systemName: "gearshape")
                        .resizable()
                        .frame(width: 50.dp, height: 50.dp)
                        .foregroundColor(Color.black)
                        .offset(x: 0, y: 26.dp)
                        .onTapGesture {
                            showSetting = true
                            
                            //offset = SettingConstants.SETTING_WIDTH
                        }
                        
                        //.alignmentGuide(.right, computeValue: { $0[.trailing] })
                        //.alignmentGuide(.trailing, computeValue: { d in d[.trailing] - 5})
                }
                .frame(width: UIScreen.main.bounds.width, height: 180.dp)
                .background(Color(hex: 0xEDEDED))
            }
            .frame(width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height)
            .background(Color(Color.RGBColorSpace.sRGB, red: 0.3, green: 0.3, blue: 0.3, opacity: 1))
            .edgesIgnoringSafeArea(.all)
            .navigationBarHidden(true)
    }
}
