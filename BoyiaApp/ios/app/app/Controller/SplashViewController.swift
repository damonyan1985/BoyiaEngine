//
//  SplashViewController.swift
//  app
//
//  Created by yanbo on 2021/11/1.
//

import UIKit
import SwiftUI

class SplashViewController: UIViewController {
    override func viewDidLoad() {
        super.viewDidLoad()
        //self.view.backgroundColor = UIColor(red: 0x37/255, green: 0xba/255, blue: 0x46/255, alpha: 1)
        self.view.backgroundColor = UIColor.green
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated);
        // 隐藏导航栏
        self.navigationController?.setNavigationBarHidden(true, animated: false)
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated);
        // 显示BoyiaView
        //self.navigationController?.pushViewController(BoyiaViewController(), animated: true)
        self.navigationController?.pushViewController(UIHostingController(rootView: HomeView()), animated: true)
    }
}
