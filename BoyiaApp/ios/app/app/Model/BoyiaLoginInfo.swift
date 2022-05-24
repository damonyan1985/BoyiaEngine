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
        static let Name = "name"
        static let NickName = "nickname"
        static let Icon = "icon"
        static let Token = "token"
    }
    
    static let shared = BoyiaLoginInfo()
    
    var _name: String?
    var name: String? {
        set {
            _name = newValue;
            if newValue == nil {
                UserDefaults.standard.removeObject(forKey: BoyiaLoginInfo.LoginKeys.Name)
            } else {
                UserDefaults.standard.set(newValue, forKey: BoyiaLoginInfo.LoginKeys.Name)
            }
        }
        
        get {
            if _name == nil {
                _name = UserDefaults.standard.string(forKey: BoyiaLoginInfo.LoginKeys.Name)
            }
            
            return _name
        }
    }
    
    var _nickname: String?
    var nickname: String? {
        set {
            _nickname = newValue;
            
            if newValue == nil {
                UserDefaults.standard.removeObject(forKey: BoyiaLoginInfo.LoginKeys.NickName)
            } else {
                UserDefaults.standard.set(newValue, forKey: BoyiaLoginInfo.LoginKeys.NickName)
            }
        }
        
        get {
            if _nickname == nil {
                _nickname = UserDefaults.standard.string(forKey: BoyiaLoginInfo.LoginKeys.NickName)
            }
            
            return _nickname
        }
    }
    
    var _token: String?
    var token: String? {
        set {
            _token = newValue;
            
            if newValue == nil {
                UserDefaults.standard.removeObject(forKey: BoyiaLoginInfo.LoginKeys.Token)
            } else {
                UserDefaults.standard.set(newValue, forKey: BoyiaLoginInfo.LoginKeys.Token)
            }
        }
        
        get {
            if _token == nil {
                _token = UserDefaults.standard.string(forKey: BoyiaLoginInfo.LoginKeys.Token)
            }
            
            return _token
        }
    }
    
    var _icon: String?
    var icon: String? {
        set {
            _icon = newValue;
            
            if newValue == nil {
                UserDefaults.standard.removeObject(forKey: BoyiaLoginInfo.LoginKeys.Icon)
            } else {
                UserDefaults.standard.set(newValue, forKey: BoyiaLoginInfo.LoginKeys.Icon)
            }
        }
        
        get {
            if _icon == nil {
                _icon = UserDefaults.standard.string(forKey: BoyiaLoginInfo.LoginKeys.Icon)
            }
            
            return _icon
        }
    }
    
    static func setLoginInfo(info: BoyiaUserInfo, token: String) {
        BoyiaLoginInfo.shared.name = info.name
        BoyiaLoginInfo.shared.nickname = info.nickname
        BoyiaLoginInfo.shared.icon = info.avatar
        BoyiaLoginInfo.shared.token = token
    }
    
    static func logout() {
        BoyiaLoginInfo.shared.name = nil
        BoyiaLoginInfo.shared.nickname = nil
        BoyiaLoginInfo.shared.icon = nil
        BoyiaLoginInfo.shared.token = nil
    }
}
