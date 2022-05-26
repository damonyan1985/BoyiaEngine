//
//  BoyiaUserInfo.swift
//  app
//
//  Created by yanbo on 2022/5/24.
//

import Foundation

struct BoyiaUserInfo : Codable {
    let uid: Int
    let name: String
    let nickname: String
    let avatar: String
    let email: String
    let phone: String? // 允许phone为空
    
    enum CodingKeys: String, CodingKey {
        case uid = "id"
        case name
        case nickname
        case avatar
        case email
        case phone
    }
}
