//
//  SplashViewController.swift
//  app
//
//  Created by yanbo on 2021/11/1.
//

import UIKit
import SwiftUI

//class Foo {
//    var x: IntObject?
//    var y: String?
//
//    init() {
//        x = IntObject(0)
//        y = ""
//    }
//}

class SplashViewController: UIViewController {
    override func viewDidLoad() {
        super.viewDidLoad()
        
        //let url = HttpUtil.HttpConstants.LOGIN_URL
        
//        let f = Foo()
//        let m = Mirror(reflecting: f)
//        for p in m.children {
//            let value = p.value
//            if var v = value as? IntObject {
//                //v.append("e")
//                v.value = 5
//                BoyiaLog.d("Foo v = \(f.x?.value)")
//            }
//        }
        
        
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
        //self.navigationController?.navigationBar.barStyle = .default
        //view.backgroundColor = UIColor(hex: 0xFFC125)
        
        // 注册api
        self.registerApiHandler()
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
        self.navigationController?.pushViewController(
            BoyiaAppViewController(rootView: HomeView()), animated: true)
    }
    
    func registerApiHandler() {
        BoyiaBridge.registerApi("user_info", creator: {
            return GetUserInfoHandler()
        })
    }
}
