//
//  SearchView.swift
//  app
//
//  Created by yanbo on 2022/7/2.
//

import SwiftUI

struct SearchView : View {
    @State var text = ""
    @FocusState var focus: Bool
    @StateObject var model = BoyiaAppListModel(autoLoad: false)
    @Environment(\.presentationMode) var presentationMode
    
    var body: some View {
        // GeometryReader可以控制视图在statubar之下
        GeometryReader { geometry in
            VStack(alignment: HorizontalAlignment.leading, spacing: 20.dp) {
                //marginTop(top: BoyiaNavigator.getStatusbarHeight())
                HStack(alignment: VerticalAlignment.center) {
                    marginLeft(left: 3.dp)
                    Button(action: {
                        self.presentationMode.wrappedValue.dismiss()
                    }) {
                        Image(systemName: "arrow.left.circle")
                            .resizable()
                            .foregroundColor(Color(hex: 0x696969))
                            .frame(width: 64.dp, height: 64.dp)
                    }
                    CustomTextField(
                        focus: $focus,
                        placeholder: Text("Please input search key")
                            .foregroundColor(Color(hex: 0x696969)),
                        text: $text)
                        .frame(width: 460.dp, height: 96.dp)
                        .padding(Edge.Set.leading, 20.dp)
                        .foregroundColor(Color.black)
                        .background(Color(hex: 0xDCDCDC))
                        .cornerRadius(48.dp)
                    
                    Button(action: {
                        model.searchAppList(key: text)
                    }) {
                        Text("Search")
                            .foregroundColor(Color.white)
                    }
                    .frame(width: 120.dp, height: 72.dp, alignment: Alignment.center)
                    .background(Color.gray)
                    .cornerRadius(36.dp)
                    .overlay {
                        RoundedRectangle(cornerRadius: 36.dp, style: .continuous)
                            .stroke(Color(hex: 0xD3D3D3), lineWidth: 4.dp)
                    }
                    
                    Spacer()
                }
                
                buildAppList()

                //Spacer()
                
            }
            .frame(width: PixelRatio.screenWidth(), height: PixelRatio.screenHeight())
            .background(Color.white)
            
            Spacer()
        }
        .ignoresSafeArea(.keyboard, edges: .bottom)
        .navigationBarHidden(true)
    }
    
    func buildAppList() -> some View {
        BoyiaLog.d("SearchView::buildAppList")
        return ScrollView {
            LazyVStack(alignment: .leading) {
                ForEach(model.appList, id: \.id) { item in
                    //Text(item.name).foregroundColor(Color(hex: 0xFF00FF))
                    SearchItemView(item: item)
                }
            }
        }
    }
}
