//
//  LoginHandler.swift
//  app
//
//  Created by yanbo on 2022/7/26.
//

import Foundation

class LoginHandler: NSObject, BoyiaApiHandler {
    func handle(_ json: [AnyHashable : Any]!, callback cb: BoyiaApiHandlerCB!) {
        // 使用独立的viewcontroller承载view，覆盖在当前viewcontroller之上
        BoyiaNavigator.push(view: LoginView())
    }
}
