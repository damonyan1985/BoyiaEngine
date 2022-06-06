//
//  BoyiaAppVersion.swift
//  app
//
//  Created by yanbo on 2022/6/6.
//

import Foundation

struct AppInfo: Codable {
    let version: String?
}

struct BoyiaBundleInfo: Codable {
    let resultCount: Int
    let results: [AppInfo]
}
