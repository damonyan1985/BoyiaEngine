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
        let completed = model.progress == 1.0
        Button(action: {
            model.download(ccb: {
                
            })
        }) {
            buttonContent(completed: completed)
        }
        .frame(width: 108.dp, height: 54.dp, alignment: Alignment.center)
        .cornerRadius(27.dp)
        .overlay {
            RoundedRectangle(cornerRadius: 27.dp, style: .continuous)
                .stroke(completed ? Color.gray : Color.green, lineWidth: 2.dp)
        }
    }
    
    func buttonContent(completed: Bool) -> some View {
        let width = model.progress * 108.dp
        //let completed = model.progress == 1.0
        return ZStack {
            if (!completed) {
                Path { path in
                    path.move(to: CGPoint(x: 0, y: 0))
                    path.addLine(to: CGPoint(x: width, y: 0))
                    path.addLine(to: CGPoint(x: width, y: 54.dp))
                    path.addLine(to: CGPoint(x: 0, y: 54.dp))
                    path.closeSubpath()
                    //path.addLine(to: CGPoint(x: 0, y: 0))
                }
                .fill(Color(argb: 0x3300FF00))
                .frame(width: 108.dp, height: 54.dp)
            }
            
            Text(completed ? "Open" : "Install")
                .font(.system(size: 24.dp))
                .foregroundColor(completed ? Color.gray : Color.green)
        }.frame(width: 108.dp, height: 54.dp, alignment: Alignment.center)
    }
}
