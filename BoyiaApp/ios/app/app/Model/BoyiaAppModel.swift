//
//  BoyiaAppModel.swift
//  app
//
//  Created by yanbo on 2022/4/27.
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
            print("HttpCallbackImpl onLoadFinished json = \(String(data: buffer, encoding: .utf8)!)")
            let value = try JSONDecoder().decode(T.self, from: buffer)
            callback(value)
        } catch {
            BoyiaLog.d("HttpCallbackImpl onLoadFinished error \(error)")
        }
    }
    
    func onLoadError() {
    }
}

class BoyiaAppModel: ObservableObject {
    // 业务数据请求接口
    func requestImpl<T: Decodable>(url: String, cb: @escaping ModelDataCallback<T>) {
        let engine = HttpEngineIOS()
        engine.loadUrl(
            HttpMethod.get,
            url: url,
            headers: nil,
            callback: HttpCallbackImpl<T>(cb: cb))
    }
}
