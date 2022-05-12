//
//  DownloadMask.swift
//  app
//
//  Created by yanbo on 2022/5/12.
//

import SwiftUI

// TODO 下载mask动画
struct BoyiaDownloadMask : View {
    @State private var progress: Double = 0.375;
    
    private var width: Double
    private var height: Double
    
    public init(width: Double, height: Double) {
        self.width = width;
        self.height = height;
    }
    
    var body: some View {
        Path { path in
            let w = width
            let h = height
            
            path.move(to: CGPoint(x: w / 2, y: 0))
            path.addLine(to: CGPoint(x: w / 2, y: h / 2))
            if progress <= 0.125 {
                let x1 = w/2 + (w/2) * (progress / 0.125)
                if (progress < 0.125) {
                    path.addLine(to: CGPoint(x: x1, y: 0))
                }
                path.addLine(to: CGPoint(x: w, y: 0))
                path.addLine(to: CGPoint(x: w, y: h))
                path.addLine(to: CGPoint(x: 0, y: h))
                path.addLine(to: CGPoint(x: 0, y: 0))
                
                path.addLine(to: CGPoint(x: w / 2, y: 0))
            } else if (progress > 0.125 && progress <= 0.375) {
                let y1 = ((progress - 0.125) / 0.25) * h
                if (progress < 0.375) {
                    path.addLine(to: CGPoint(x: w, y: y1))
                }
                path.addLine(to: CGPoint(x: w, y: h))
                path.addLine(to: CGPoint(x: 0, y: h))
                path.addLine(to: CGPoint(x: 0, y: 0))
                
                path.addLine(to: CGPoint(x: w / 2, y: 0))
            } else if (progress > 0.375 && progress <= 0.625) {
                let x1 = w - ((progress - 0.375) / 0.25) * w
                if (progress < 0.625) {
                    //path.addLine(x1.toFloat(), h)
                    path.addLine(to: CGPoint(x: x1, y: h))
                }
                path.addLine(to: CGPoint(x: 0, y: h))
                path.addLine(to: CGPoint(x: 0, y: 0))
                
                path.addLine(to: CGPoint(x: w / 2, y: 0))
            } else if (progress > 0.625 && progress <= 0.875) {
                let y1 = h - ((progress - 0.625) / 0.25) * h
                if (progress < 0.875) {
                    path.addLine(to: CGPoint(x: 0, y: y1))
                }
                path.addLine(to: CGPoint(x: 0, y: 0))
                
                path.addLine(to: CGPoint(x: w / 2, y: 0))
            } else if (progress > 0.875 && progress < 1.0) {
                let x1 = ((progress - 0.875) / 0.25) * w
                path.addLine(to: CGPoint(x: x1, y: 0))
                
                path.addLine(to: CGPoint(x: w / 2, y: 0))
            }
        }
        .fill(Color(argb: 0x88000000))
        .frame(width: width, height: height)
        
        getTransparentMask()
    }
    
    // 避免点击事件向下传递
    func getTransparentMask() -> some View {
        return Path { path in
            path.move(to: CGPoint(x: 0, y: 0))
            path.addLine(to: CGPoint(x: width, y: 0))
            path.addLine(to: CGPoint(x: width, y: height))
            path.addLine(to: CGPoint(x: 0, y: height))
            path.addLine(to: CGPoint(x: 0, y: 0))
        }
        .fill(Color(argb: 0x01000000))
        .frame(width: width, height: height)
    }
}


