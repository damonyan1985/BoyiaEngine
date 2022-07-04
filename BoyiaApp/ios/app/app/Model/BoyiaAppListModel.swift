//
//  BoyiaAppListModel.swift
//  app
//
//  Created by yanbo on 2022/4/26.
//

import Foundation
import UIKit
import core

class BoyiaAppListModel: ObservableObject {
    //private let APP_LIST_URL = "http://47.98.206.177/test.json"
    private let APP_LIST_URL = "https://127.0.0.1:8443/app/v1/appList"
    @Published var appList: [BoyiaAppItem] = []
    
    init() {
        requestAppList()        
    }
    
    // 业务数据请求接口
    func requestAppList() {
        HttpUtil.get(url: APP_LIST_URL, cb: { (data: BoyiaAppListData) in
            DispatchQueue.main.async {
                self.appList.append(contentsOf: data.data)
                BoyiaLog.d("appList first name = \(self.appList[0].appId)")
            }
        })
    }
    
    func searchAppList(key: String, pos: Int, size: Int) {
        var headers = [AnyHashable: Any]()
        headers["Content-Type"] = "application/x-www-form-urlencoded"
        headers["User-Token"] = BoyiaLoginInfo.shared.token
        
        let data = "key=\(key)&pos=\(pos)&size=\(size)"
        
        HttpUtil.post(url: HttpUtil.HttpConstants.SEARCH_APP_URL,
                      data: data,
                      headers: headers,
                      cb: { (data: BoyiaAppListData) in
            DispatchQueue.main.async {
                self.appList.append(contentsOf: data.data)
                BoyiaLog.d("searchAppList first name = \(self.appList[0].appId)")
            }
        })
    }
}
