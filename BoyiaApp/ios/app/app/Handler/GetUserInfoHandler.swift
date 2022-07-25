//
//  GetUserInfoHandler.swift
//  app
//
//  Created by yanbo on 2022/7/25.
//

import Foundation

class GetUserInfoHandler: NSObject, BoyiaApiHandler {
    func handle(_ json: [AnyHashable : Any]!, callback cb: BoyiaApiHandlerCB!) {
        let data = BoyiaUserData(
            data: BoyiaLoginInfo.shared.user!,
            userToken: BoyiaLoginInfo.shared.token!, retCode: 0, retMsg: "")
        
        let jsonEncoder = JSONEncoder()
        guard let jsonData = try? jsonEncoder.encode(data) else {
            return
        }
        
        cb.callback(String(data: jsonData, encoding: .utf8))
    }
}
