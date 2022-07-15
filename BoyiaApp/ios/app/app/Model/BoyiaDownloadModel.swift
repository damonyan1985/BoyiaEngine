//
//  BoyiaDownloadModel.swift
//  app
//
//  Created by yanbo on 2022/6/8.
//

import Foundation

class BoyiaDownloadModel: ObservableObject {
    // 只能在UI线程更改
    @Published var progress: Double = 0.0
    
//    init() {
//        download()
//    }
    
    func download(ccb: @escaping DownloadCompletedCallback) {
        if (self.progress > 0) {
            return;
        }
        
        // 给一个很小的值避免重复请求
        self.progress = 0.00001
        
        HttpUtil.download(url: "https://klxxcdn.oss-cn-hangzhou.aliyuncs.com/histudy/hrm/media/bg3.mp4", headers: nil, pcb: { (prog: Double) in
            DispatchQueue.main.async {
                self.progress = prog
            }
        }, ccb: ccb)
    }
}
