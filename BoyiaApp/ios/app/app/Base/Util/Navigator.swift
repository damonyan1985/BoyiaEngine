//
//  Navigator.swift
//  app
//
//  Created by yanbo on 2022/3/24.
//

import UIKit
import SwiftUI

// 封装导航
class BoyiaNavigator {
    // push一个swiftui view
    static func push<CustomView>(view: CustomView) -> Void where CustomView : View  {
        //findNavigator()?.pushViewController(UIHostingController(rootView: view), animated: true)
        push(controller: UIHostingController(rootView: view))
    }
    
    static func push(controller: UIViewController) {
        findNavigator()?.pushViewController(controller, animated: true)
    }
    
    // static func修饰的方法不能被子类重写
    // class func修饰的方法能被子类重写
    static func pop() {
        findNavigator()?.popViewController(animated: true)
    }
    
    // 找到导航VC
    static func findNavigator() -> UINavigationController? {
//        guard let window = UIApplication.shared.windows.first else {
//            return nil
//        }
        guard let window = getKeyWindow() else {
            return nil
        }
        
        let rootController = window.rootViewController;
        let viewController = findCurrentVC(rootController)
        
        guard let nav = viewController?.navigationController else {
            return nil
        }
        
        return nav;
    }
    
    // 找到当前使用的VC
    static func findCurrentVC(_ vc: UIViewController?) -> UIViewController? {
        var currentShowingVC: UIViewController?
        if (vc?.presentedViewController != nil) { //注要优先判断vc是否有弹出其他视图，如有则当前显示的视图肯定是在那上面
            // 当前视图是被presented出来的
            let nextRootVC = vc?.presentedViewController
            currentShowingVC = findCurrentVC(nextRootVC)
            
        } else if (vc is UITabBarController) {
            // 根视图为UITabBarController
            let nextRootVC = (vc as? UITabBarController)?.selectedViewController
            currentShowingVC = findCurrentVC(nextRootVC)
            
        } else if (vc is UINavigationController){
            // 根视图为UINavigationController
            let nextRootVC = (vc as? UINavigationController)?.visibleViewController
            currentShowingVC = findCurrentVC(nextRootVC)
            
        } else {
            // 根视图为非导航类
            currentShowingVC = vc
        }
        
        return currentShowingVC
    }
        
    static func getKeyWindow() -> UIWindow? {
//        // Get connected scenes
//        return UIApplication.shared.connectedScenes
//            // Keep only active scenes, onscreen and visible to the user
//            .filter { $0.activationState == .foregroundActive }
//            // Keep only the first `UIWindowScene`
//            .first(where: { $0 is UIWindowScene })
//            // Get its associated windows
//            .flatMap({ $0 as? UIWindowScene })?.windows
//            // Finally, keep only the key window
//            .first(where: \.isKeyWindow)
        guard let scene = getKeyWindowScene() else {
            return nil
        }
        
        // Finally, keep only the key window
        return scene.windows.first(where: \.isKeyWindow)
    }
    
    static func getKeyWindowScene() -> UIWindowScene? {
        return UIApplication.shared.connectedScenes
            // Keep only active scenes, onscreen and visible to the user
            .filter { $0.activationState == .foregroundActive }
            // Keep only the first `UIWindowScene`
            .first(where: { $0 is UIWindowScene })
            // Get its associated windows
            .flatMap({ $0 as? UIWindowScene })
    }
    
    // 获取状态栏高度
    static func getStatusbarHeight() -> Double {
        guard let statusBarManager = getKeyWindowScene()?.statusBarManager else {
            return 0
        }
        
        return Double(Int(statusBarManager.statusBarFrame.size.height))
    }
}
