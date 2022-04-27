//
//  BoyiaAppListModel.swift
//  app
//
//  Created by yanbo on 2022/4/26.
//

import Foundation
import core

class BoyiaAppListModel: BoyiaAppModel {
    private let APP_LIST_URL = "http://47.98.206.177/test.json"
    @Published var appList: [BoyiaAppItem] = []
    
    // 业务数据请求接口
    func requestAppList() {
        requestImpl(url: APP_LIST_URL, cb: { (data: BoyiaAppListData) in
            DispatchQueue.main.async {
                self.appList.append(contentsOf: data.apps)
                BoyiaLog.d("appList first name = \(self.appList[0].name)")
            }
        })
    }
}
