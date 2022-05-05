//
//  BoyiaAppItem.swift
//  app
//
//  Created by yanbo on 2022/4/27.
//

import Foundation

// Identifiable用于list遍历，Codable用于json解析
struct BoyiaAppItem: Codable, Identifiable {
    var id = UUID()
    let appId: Int
    let name: String
    let cover: String
    let url: String
    let versionCode: Int
    
    // 映射json字段名
    enum CodingKeys: String, CodingKey {
        case appId = "id"
        case name
        case cover
        case url
        case versionCode
    }
}
