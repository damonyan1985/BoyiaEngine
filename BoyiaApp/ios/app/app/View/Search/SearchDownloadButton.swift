//
//  SearchDownloadButton.swift
//  app
//
//  Created by yanbo on 2022/7/15.
//

import Foundation
import SwiftUI

struct SearchDownloadButton : View {
    @StateObject private var model = BoyiaDownloadModel()
    
    var body: some View {
        Button(action: {
            
        }) {
            ZStack {
                Path { path in
                    path.move(to: CGPoint(x: 0, y: 0))
                    
                }
                Text("Install")
                    .font(.system(size: 24.dp))
                    .foregroundColor(Color.green)
            }.frame(width: 108.dp, height: 54.dp, alignment: Alignment.center)
        }
        .frame(width: 108.dp, height: 54.dp, alignment: Alignment.center)
        .cornerRadius(27.dp)
        .overlay {
            RoundedRectangle(cornerRadius: 27.dp, style: .continuous)
                .stroke(Color.green, lineWidth: 2.dp)
        }
    }
}
