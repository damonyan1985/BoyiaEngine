//
//  HttpUtil.swift
//  app
//
//  Created by yanbo on 2022/5/16.
//

import Foundation
import core

typealias ModelDataCallback<T> = (_ data: T) -> Void

class HttpCallbackImpl<T: Decodable>: NSObject, HttpCallback {
    lazy var buffer: Data = Data()
    var callback: ModelDataCallback<T>
    
    init(cb: @escaping ModelDataCallback<T>) {
        callback =  cb
    }
    
    func onDataReceive(_ data: Data!) {
        buffer.append(data);
    }
    
    func onLoadFinished() {
        do {
            BoyiaLog.d("HttpCallbackImpl onLoadFinished json = \(String(data: buffer, encoding: .utf8)!)")
            let value = try JSONDecoder().decode(T.self, from: buffer)
            callback(value)
        } catch {
            BoyiaLog.d("HttpCallbackImpl onLoadFinished error \(error)")
        }
    }
    
    func onLoadError() {
    }
}

class HttpUtil {
    struct HttpConstants {
        static let HTTP_DOMAIN = "https://127.0.0.1:8443/"
        static let API_VERSION = "v1"
        
        static let LOGIN_URL = "\(HTTP_DOMAIN)user/\(API_VERSION)/login"
        static let LOGOUT_URL = "\(HTTP_DOMAIN)user/\(API_VERSION)/logout"
        static let APP_LIST_URL = "\(HTTP_DOMAIN)app/\(API_VERSION)/appList"
    }
    // 业务数据请求接口
    static func get<T: Decodable>(
        url: String,
        headers: [AnyHashable : Any]? = nil,
        cb: @escaping ModelDataCallback<T>) {
        let engine = HttpEngineIOS()
        engine.loadUrl(
            HttpMethod.get,
            url: url,
            headers: headers,
            callback: HttpCallbackImpl<T>(cb: cb))
    }
    
    static func post<T: Decodable>(
        url: String,
        data: String? = nil,
        headers: [AnyHashable : Any]? = nil,
        cb: @escaping ModelDataCallback<T>) {
        let engine = HttpEngineIOS()
        if data != nil {
            engine.setData(data)
        }
        engine.loadUrl(
            HttpMethod.post,
            url: url,
            headers: headers,
            callback: HttpCallbackImpl<T>(cb: cb))
    }
}
