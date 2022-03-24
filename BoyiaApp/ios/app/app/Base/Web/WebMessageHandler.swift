//
//  WebMessageHandler.swift
//  app
//
//  Created by yanbo on 2022/3/23.
//

import Foundation
import WebKit

// 接受window.webkit.messageHandlers.getMessage.postMessage(...)发送过来的消息
class WebMessageHandler : NSObject, WKScriptMessageHandler {
    // ---- 实现WKScriptMessageHandler begin ----//
    func userContentController(_ userContentController: WKUserContentController, didReceive message: WKScriptMessage) {
        // 处理js传递过来的消息
    }
    // ---- 实现WKScriptMessageHandler end ----//
}
