//
//  BoyiaLoginModel.swift
//  app
//
//  Created by yanbo on 2022/5/24.
//

import Foundation

class BoyiaLoginModel : ObservableObject {
    @Published var isLogin = false
    
    init() {
        BoyiaLog.d("token=\(String(describing: BoyiaLoginInfo.shared.token))")
        isLogin = BoyiaLoginInfo.shared.token != nil
    }
    
    func login(name: String, password: String, onComplete: ((Bool) -> Void)? = nil) {
        if name.isEmpty || password.isEmpty {
            BoyiaLog.d("name and password is empty")
            return
        }
        BoyiaLog.d("name=\(name)&pwd=\(password)")
        //var headers = NSMutableDictionary()
        //headers.setObject("application/x-www-form-urlencoded", forKey: NSString("Content-Type"))
        var headers = [AnyHashable: Any]()
        headers["Content-Type"] = "application/x-www-form-urlencoded"
        headers["User-Token"] = "none"
        
        let data = "name=\(name)&pwd=\(password)"
        
        HttpUtil.post(url: HttpUtil.HttpConstants.LOGIN_URL, data: data, headers: headers, cb: { (data: BoyiaUserData) in
            DispatchQueue.main.async {
                BoyiaLog.d("BoyiaLoginModel login = \(data.retCode)")
                if (data.retCode == 200) {
                    BoyiaLoginInfo.setLoginInfo(info: data.data, token: data.userToken)
                    self.isLogin = true
                    if onComplete != nil {
                        onComplete!(true)
                    }
                } else {
                    if onComplete != nil {
                        onComplete!(false)
                    }
                }
            }
        })
    }
    
    func logout(onComplete: ((Bool) -> Void)? = nil) {
        var headers = [AnyHashable: Any]()
        headers["Content-Type"] = "application/x-www-form-urlencoded"
        headers["User-Token"] = BoyiaLoginInfo.shared.token
        HttpUtil.post(url: HttpUtil.HttpConstants.LOGOUT_URL, headers: headers, cb: { (data: BoyiaBaseData) in
            DispatchQueue.main.async {
                BoyiaLog.d("BoyiaLoginModel logout = \(data.retCode)")
                // 就算失败本地也不再保存变量
                BoyiaLoginInfo.logout()
                self.isLogin = false
            }
        })
    }
}
