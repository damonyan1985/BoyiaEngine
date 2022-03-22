//
//  Uploader.swift
//  app
//
//  Created by yanbo on 2022/3/22.
//

import Foundation
import core

class Uploader {
    static func upload(_ url: String, _ path: String) {
        let reqUrl = URL(string: url);
        var request: URLRequest = URLRequest(url: reqUrl!);
        
        let boundary = "Boundary-\(UUID().uuidString)";
        request.httpMethod = "POST";
        request.setValue("multipart/form-data; boundary=\(boundary)",forHTTPHeaderField: "Content-Type");

        let fileUrl = URL(fileURLWithPath: path);
        let data = try! Data(contentsOf: fileUrl);
        
        var body = Data();
        body.append(String(format:"Content-Disposition:form-data;name=\"%@\";\r\n").data(using: .utf8)!)
        //body.append("Content-Type:image/jpeg\r\n\r\n".data(using: .utf8)!)
        body.append(data)
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
