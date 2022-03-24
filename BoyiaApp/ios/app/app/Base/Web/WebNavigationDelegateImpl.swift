//
//  WebNavigationDelegateImpl.swift
//  app
//
//  Created by yanbo on 2022/3/23.
//

import Foundation
import WebKit

class WebNavigationDelegateImpl : NSObject, WKNavigationDelegate {
    // ---- WKNavigationDelegate begin ----//
    // 页面开始加载时调用
    func webView(_ webView: WKWebView, didStartProvisionalNavigation navigation: WKNavigation!) {
        
    }
    
    // 当内容开始返回时调用
    func webView(_ webView: WKWebView, didCommit navigation: WKNavigation!) {
        
    }
    
    // 页面加载完成之后调用
    func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) {
        
    }
    
    // 页面加载失败时调用
    func webView(_ webView: WKWebView, didFailProvisionalNavigation navigation: WKNavigation!, withError error: Error) {
        
    }
    
    // 接收到服务器跳转请求之后调用
    func webView(_ webView: WKWebView, didReceiveServerRedirectForProvisionalNavigation navigation: WKNavigation!) {
        
    }
    
    // 在收到响应后，决定是否跳转 -> 默认允许
    func webView(_ webView: WKWebView, decidePolicyFor navigationResponse: WKNavigationResponse, decisionHandler: @escaping (WKNavigationResponsePolicy) -> Void) {
        //允许跳转
        decisionHandler(.allow)
        //不允许跳转
    }
    
    // 在发送请求之前，决定是否跳转 -> 默认允许
    func webView(_ webView: WKWebView, decidePolicyFor navigationAction: WKNavigationAction, preferences: WKWebpagePreferences, decisionHandler: @escaping (WKNavigationActionPolicy, WKWebpagePreferences) -> Void) {
        preferences.allowsContentJavaScript = true
        decisionHandler(.allow, preferences)
    }
    // ---- WKNavigationDelegate end ----//
}
