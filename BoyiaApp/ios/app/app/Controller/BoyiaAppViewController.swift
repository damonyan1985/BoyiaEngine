//
//  BoyiaAppViewController.swift
//  app
//
//  Created by yanbo on 2022/7/2.
//

import Foundation
import UIKit
import SwiftUI

class BoyiaAppViewController<Content> : UIHostingController<Content> where Content : View {
    override func viewDidLoad() {
        super.viewDidLoad()
        self.setNeedsStatusBarAppearanceUpdate()
    }
    
    // 设置状态栏字体为黑色
    override var preferredStatusBarStyle: UIStatusBarStyle {
        return .darkContent
    }
}


