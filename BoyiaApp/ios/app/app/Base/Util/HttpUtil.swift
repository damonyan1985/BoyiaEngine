//
//  HttpUtil.swift
//  app
//
//  Created by yanbo on 2022/5/16.
//

import Foundation
import core

typealias ModelDataCallback<T> = (_ data: T) -> Void
typealias DownloadProgressCallback = (_ progress: Double) -> Void
typealias DownloadCompletedCallback = () -> Void

class HttpCallbackImpl<T: Decodable>: NSObject, HttpCallback {
    lazy var buffer: Data = Data()
    var callback: ModelDataCallback<T>
    
    init(cb: @escaping ModelDataCallback<T>) {
        callback = cb
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
    
    func onProgress(_ current: Int64, total: Int64) {
    }
}

class HttpDownloadCallback: NSObject, HttpCallback {
    //lazy var buffer: Data = Data()
    var progressCB: DownloadProgressCallback
    var completedCB: DownloadCompletedCallback
    let file: FileHandle?
    var size: Int64 = 0
    var current: Int64 = 0
    
    init(path: String, pcb: @escaping DownloadProgressCallback, ccb: @escaping DownloadCompletedCallback) {
        progressCB = pcb
        completedCB = ccb
        file = FileHandle(forWritingAtPath: path)
        BoyiaLog.d("HttpDownloadCallback path = \(path)")
    }
    
    func onDataReceive(_ data: Data!) {
        //buffer.append(data);
        current += Int64(data.count)
        file?.write(data)
        progressCB((Double)(current)/(Double)(size))
    }
    
    func onLoadFinished() {
        do {
            try file?.close()
        } catch {
            BoyiaLog.d("HttpDownloadCallback close file error: \(error)")
        }
        completedCB()
    }
    
    func onLoadError() {
    }
    
    func onProgress(_ current: Int64, total: Int64) {
        size = total
    }
}

class UploadCallback: NSObject, HttpCallback {
    func onProgress(_ current: Int64, total: Int64) {
        
    }
    
    func onDataReceive(_ data: Data!) {
        
    }
    
    func onLoadFinished() {
        
    }
    
    func onLoadError() {
        
    }
}

class HttpUtil {
    struct HttpConstants {
        static let BUNDLE_INFO = "https://itunes.apple.com/lookup?bundleId=1000"
        static let APP_STORE_URL = "itms-apps://itunes.apple.com/app/1000"
        
        static let HTTP_DOMAIN = "https://127.0.0.1:8443/"
        static let API_VERSION = "v1"
        
        static let LOGIN_URL = "\(HTTP_DOMAIN)user/\(API_VERSION)/login"
        static let LOGOUT_URL = "\(HTTP_DOMAIN)user/\(API_VERSION)/logout"
        static let APP_LIST_URL = "\(HTTP_DOMAIN)app/\(API_VERSION)/appList"
        static let UPLOAD_URL = "\(HTTP_DOMAIN)file/\(API_VERSION)/upload"
    }
    // 业务数据请求接口
    static func get<T: Decodable>(
        url: String,
        headers: [AnyHashable : Any]? = nil,
        cb: @escaping ModelDataCallback<T>) {
            
        //BoyiaLog.d("approot = \(BoyiaBridge.getAppRoot()!)")
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
    
    // 检查版本并跳转appstore
    static func checkVersion() {
        HttpUtil.get(url: HttpConstants.BUNDLE_INFO, cb: { (data: BoyiaBundleInfo) in
            DispatchQueue.main.async {
                BoyiaLog.d("requestBundleInfo store_version = \(data.results[0].version!)")
                
                let version: String = (Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String)!
                BoyiaLog.d("requestBundleInfo app_version = \(version)")
                if (version != data.results[0].version!) {
                    let urlString = HttpConstants.APP_STORE_URL
                    if let url = URL(string: urlString) {
                        if #available(iOS 10, *) {
                            UIApplication.shared.open(url, options: [:],
                                                      completionHandler: {
                                                        (success) in
                            })
                        } else {
                            UIApplication.shared.openURL(url)
                        }
                    }
                }
            }
        })
    }
    
    // 下载功能
    static func download(url: String,
                         headers: [AnyHashable : Any]? = nil,
                         pcb: @escaping DownloadProgressCallback,
                         ccb: @escaping DownloadCompletedCallback
    ) {
        let fileManager = FileManager.default
        
        let downloadDir: String = BoyiaBridge.getAppRoot() + "download/"
        if fileManager.fileExists(atPath: downloadDir) == false {
            do {
                // 创建文件目录
                try fileManager.createDirectory(
                    atPath: downloadDir,
                    withIntermediateDirectories: true,
                    attributes: nil)
            } catch {
                print(error)
                return
            }
        }
        
        let downloadFile = downloadDir + url.md5
        if fileManager.fileExists(atPath: downloadFile) {
            do {
                try fileManager.removeItem(atPath: downloadFile)
            } catch {
                print(error)
                return
            }
            
        }
        
        // 创建文件
        fileManager.createFile(atPath: downloadFile, contents: nil, attributes: nil)
        
        let engine = HttpEngineIOS()
        engine.loadUrl(
            HttpMethod.get,
            url: url,
            headers: headers,
            callback: HttpDownloadCallback(path: downloadFile, pcb: pcb, ccb: ccb))
    }
    
    // 上传功能
    static func upload(path: String) {
        var headers = [AnyHashable: Any]()
        headers["User-Token"] = BoyiaLoginInfo.shared.token
        
        let engine = HttpEngineIOS()
        engine.upload(HttpConstants.UPLOAD_URL, path: path, headers: headers, callback: UploadCallback())
    }
}
