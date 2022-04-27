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
//        let view = UIView(frame: CGRect(x: 0, y: 0, width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height))
//        self.view.addSubview(view)
        
        let gradient = CAGradientLayer();
        gradient.frame = view.bounds
        // 渐变使用CGColor
        gradient.colors = [UIColor(hex: 0xFFC125).cgColor, UIColor(hex: 0xFFD700).cgColor]
        gradient.startPoint = CGPoint(x: 0, y: 0)
        gradient.endPoint = CGPoint(x: 1, y: 1)
        //self.view.backgroundColor = UIColor(red: 0x37/255, green: 0xba/255, blue: 0x46/255, alpha: 1)
        //self.view.backgroundColor = UIColor.green
        self.view.layer.addSublayer(gradient)
        //view.backgroundColor = UIColor(hex: 0xFFC125)
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
