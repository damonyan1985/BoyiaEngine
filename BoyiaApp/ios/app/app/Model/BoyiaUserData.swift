//
//  BoyiaUserData.swift
//  app
//
//  Created by yanbo on 2022/5/24.
//

import Foundation

struct BoyiaUserData : Codable {
    let data: BoyiaUserInfo
    let userToken: String
    let retCode: Int
    let retMsg: String
}
