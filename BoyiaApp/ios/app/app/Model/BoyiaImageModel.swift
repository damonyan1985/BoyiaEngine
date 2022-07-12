//
//  BoyiaImageModel.swift
//  app
//
//  Created by yanbo on 2022/7/12.
//

import Foundation

class BoyiaImageModel: ObservableObject {
    @Published var image = UIImage()
    private var isLoaded = false
    
    func load(url: String) {
        if (self.isLoaded) {
            return
        }
        
        HttpUtil.loadImage(url: url) { im in
            guard let im = im else {
                return
            }
            
            self.image = im
            self.isLoaded = true
        }
    }
}
