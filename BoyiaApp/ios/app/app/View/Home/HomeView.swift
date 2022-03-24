//
//  HomeView.swift
//  app
//
//  Created by yanbo on 2022/3/24.
//

import SwiftUI

struct HomeView : View {
    var body: some View {
        NavigationView {
            // 类似flutter中的stack
            ZStack {
                Button(action: {
//                    guard let window = UIApplication.shared.windows.first else {
//                        return
//                    }
                    
//                    window.rootViewController = UIHostingController(rootView: LoginView())
//                    window.makeKeyAndVisible()
//                    window.rootViewController?.navigationController?.pushViewController(
//                        UIHostingController(rootView: LoginView()), animated: true)
                    
//                    let rootController = window.rootViewController;
//                    let viewController = self.findCurrentVC(rootController)
//
//                    guard let nav = viewController?.navigationController else {
//                        return
//                    }
//
//                    nav.pushViewController(UIHostingController(rootView: LoginView()), animated: true)
                    
                    BoyiaNavigator.push(view: LoginView())
                    
                }, label: {
                    Text("Test")
                })
            }
            .frame(width: UIScreen.main.bounds.width, height: UIScreen.main.bounds.height)
            .background(Color(Color.RGBColorSpace.sRGB, red: 0.3, green: 0.3, blue: 0.3, opacity: 1))
            .edgesIgnoringSafeArea(.all)
        }
        .navigationBarHidden(true)
    }
    
    func findCurrentVC(_ vc: UIViewController?) -> UIViewController? {
        
        var currentShowingVC: UIViewController?
        if (vc?.presentedViewController != nil) { //注要优先判断vc是否有弹出其他视图，如有则当前显示的视图肯定是在那上面
            // 当前视图是被presented出来的
            let nextRootVC = vc?.presentedViewController
            currentShowingVC = self.findCurrentVC(nextRootVC)
            
        } else if (vc is UITabBarController) {
            // 根视图为UITabBarController
            let nextRootVC = (vc as? UITabBarController)?.selectedViewController
            currentShowingVC = self.findCurrentVC(nextRootVC)
            
        } else if (vc is UINavigationController){
            // 根视图为UINavigationController
            let nextRootVC = (vc as? UINavigationController)?.visibleViewController
            currentShowingVC = self.findCurrentVC(nextRootVC);
            
        } else {
            // 根视图为非导航类
            currentShowingVC = vc;
        }
        
        return currentShowingVC;
    }
}
