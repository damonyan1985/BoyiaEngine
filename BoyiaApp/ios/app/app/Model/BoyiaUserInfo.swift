//
//  BoyiaUserInfo.swift
//  app
//
//  Created by yanbo on 2022/5/24.
//

import Foundation

struct BoyiaUserInfo : Codable {
    let name: String
    let nickname: String
    let avatar: String
    let email: String
    let phone: String? // 允许phone为空
}
