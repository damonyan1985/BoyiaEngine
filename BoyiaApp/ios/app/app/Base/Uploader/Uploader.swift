//
//  Uploader.swift
//  app
//
//  Created by yanbo on 2022/3/22.
//

import Foundation
import core

// swift如果想被oc调用必须继承NSObject，否则无法编译到app-swift.h中
// 类的方法如果想被导入到app-swift.h, 必须加@objc
class Uploader : NSObject {
    // 参数名前加_表示调用时省略外部形参名，直接传入实参
    @objc static func upload(_ url: String, _ path: String) {
        let reqUrl = URL(string: url);
        var request: URLRequest = URLRequest(url: reqUrl!);
        
        let boundary = "Boundary-\(UUID().uuidString)";
        request.httpMethod = "POST";
        request.setValue("multipart/form-data; boundary=\(boundary)",forHTTPHeaderField: "Content-Type");

        let fileUrl = URL(fileURLWithPath: path);
        let data = try? Data(contentsOf: fileUrl);
        if (data == nil) {
            return;
        }
        
        var body = Data();
        body.append(String(format:"Content-Disposition:form-data;name=\"%@\";\r\n").data(using: .utf8)!)
        //body.append("Content-Type:image/jpeg\r\n\r\n".data(using: .utf8)!)
        body.append(data!)
        body.append("\r\n".data(using: .utf8)!)
        body.append(String(format:"--%@--\r\n",boundary).data(using: .utf8)!)
        
    
        let session = URLSession.shared;
        let task = session.uploadTask(with: request, from: body) {
            (data: Data?, response: URLResponse?, error: Error?) -> Void in
            if  error !=  nil {
                print (error)
            } else {
                let  str =  String (data: data!, encoding:  String . Encoding .utf8)
                print ( "上传完毕：\(str)" )
            }
        };
        
        task.resume();
    }
    
//    static func test() {
//        Uploader.upload("", "");
//    }
}
