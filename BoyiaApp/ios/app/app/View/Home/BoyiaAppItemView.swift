//
//  BoyiaAppItemView.swift
//  app
//
//  Created by yanbo on 2022/5/12.
//

import SwiftUI

struct BoyiaAppItemView : View {
    private var item: BoyiaAppItem
    @State private var showMask = true
    
    public init(data: BoyiaAppItem) {
        item = data;
    }
    
    var body: some View {
        let width = 224.dp
        let height = 260.dp
        ZStack {
            VStack(alignment: .leading, spacing: 0) {
                AsyncImage(url: URL(string: item.cover)!) { phase in
                    if let image = phase.image {
                        image.resizable()
                            .transition(.slide)
                            .frame(width: width,
                                   height: width,
                                   alignment: .top)
                    } else if phase.error != nil {
                        Text("path: \(item.cover), error: \(phase.error!.localizedDescription) ")
                    } else {
                        ProgressView()
                    }
                
                }
                Text(item.name)
                    .frame(width: width, height: 36.dp)
                    .foregroundColor(Color.black)
            }
            .frame(width: width, height: height)
            .background(Color(hex: 0xEDEDED))
            .onTapGesture {
    //            BoyiaPermission.authMicrophone(closurer: { (s) in
    //                BoyiaLog.d("authPhoto status = \(s)")
    //            })
                BoyiaNavigator.push(controller: BoyiaViewController())
            }
            
            if showMask {
                BoyiaDownloadMask(width: width, height: height, ccb: {
                    DispatchQueue.main.async {
                        showMask = false
                    }
                })
            }
        }
        .cornerRadius(12.dp)
    }
}
