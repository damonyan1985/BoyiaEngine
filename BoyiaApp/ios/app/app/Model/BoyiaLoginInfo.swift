//
//  BoyiaLoginInfo.swift
//  app
//
//  Created by yanbo on 2022/5/24.
//

import Foundation
import UIKit

class BoyiaLoginInfo {
    struct LoginKeys {
        static let User = "user"
        static let Token = "token"
    }
        
    static let shared = BoyiaLoginInfo()
    
    
    var _user: BoyiaUserInfo?
    var user: BoyiaUserInfo? {
        set {
            _user = newValue;
            if newValue == nil {
                UserDefaults.standard.removeObject(forKey: LoginKeys.User)
            } else {
                UserDefaults.standard.setItem(obj: newValue, key: LoginKeys.User)
            }
        }

        get {
            if _user == nil {
                _user = UserDefaults.standard.getItem(key: LoginKeys.User)
            }

            return _user
        }
    }
    
    var _token: String?
    var token: String? {
        set {
            _token = newValue;

            if newValue == nil {
                UserDefaults.standard.removeObject(forKey: LoginKeys.Token)
            } else {
                UserDefaults.standard.set(newValue, forKey: LoginKeys.Token)
            }
        }

        get {
            if _token == nil {
                _token = UserDefaults.standard.string(forKey: LoginKeys.Token)
            }

            return _token
        }
    }
    
    static func flush() {
        let info = BoyiaLoginInfo.shared.user
        BoyiaLoginInfo.shared.user = info
    }
    
    static func setLoginInfo(info: BoyiaUserInfo, token: String) {
        BoyiaLoginInfo.shared.token = token
        BoyiaLoginInfo.shared.user = info
    }
    
    static func logout() {
        BoyiaLoginInfo.shared.user = nil
        BoyiaLoginInfo.shared.token = nil
    }
}
