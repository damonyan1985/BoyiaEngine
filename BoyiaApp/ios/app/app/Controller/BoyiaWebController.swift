//
//  WebViewController.swift
//  app
//
//  Created by yanbo on 2022/3/23.
//

import UIKit
import WebKit

// WebView与JS交互
class BoyiaWebController : UIViewController {
    var webView: WKWebView?
    var userContentController: WKUserContentController?
    var navigationDelegate: WebNavigationDelegateImpl?
    
    override func viewDidLoad() {
        super.viewDidLoad();
        // 导航代理
        self.navigationDelegate = WebNavigationDelegateImpl();
        
        let userContentController = WKUserContentController()
        self.userContentController = userContentController
        self.userContentController?.add(WebMessageHandler(), name: "getMessage")
        
        
        let webConfig = WKWebViewConfiguration()
        webConfig.userContentController = userContentController
        
        self.webView = WKWebView(
            frame:CGRect(
                x:0,
                y:0,
                width:UIScreen.main.bounds.size.width,
                height:UIScreen.main.bounds.size.height),
            configuration: webConfig
        )
        self.webView?.navigationDelegate = self.navigationDelegate;
        
        let url = URL(string: "https://www.sina.com.cn");
        
        self.webView?.load(URLRequest(url: url!))
                
        self.view.addSubview(self.webView!);
    }
 }
