//
//  SearchItem.swift
//  app
//
//  Created by yanbo on 2022/7/11.
//

import SwiftUI

struct SearchItemView : View {
    private var item: BoyiaAppItem
    
    public init(data: BoyiaAppItem) {
        item = data;
    }
    var body: some View {
        HStack {
            AsyncImage(url: URL(string: HttpUtil.getRemoteUrl(url: item.cover)!)!) { phase in
                if let image = phase.image {
                    image.resizable()
                        .transition(.slide)
                } else if phase.error != nil {
                    ProgressView()
                } else {
                    ProgressView()
                }
            }
        }
    }
}
