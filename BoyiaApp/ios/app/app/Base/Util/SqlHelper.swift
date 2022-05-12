//
//  SqlManager.swift
//  app
//
//  Created by yanbo on 2022/5/10.
//

import Foundation
import SQLite3

class SqlHelper: NSObject {
    struct SqlConstants {
        static let DB_NAME = "boyia_app.db"
    }
    
    static let shared = SqlHelper()
    
    internal var db: OpaquePointer?
    
    private var queue: DispatchQueue!
    
    private override init() {
        super.init()
        queue = DispatchQueue(label: SqlConstants.DB_NAME, attributes: [])
    }
    
    func open(filePath: String = SqlConstants.DB_NAME) -> Bool {
        let docDir: String! = NSSearchPathForDirectoriesInDomains(
            FileManager.SearchPathDirectory.documentDirectory,
            FileManager.SearchPathDomainMask.userDomainMask, true).first
        
        let path =  docDir + "/" + filePath
        let error = sqlite3_open(path, &db)
        if error != SQLITE_OK {
            sqlite3_close(db)
            return false
        }
        
        return true
    }
    
    func execute(sql: String) -> Bool {
        return (sqlite3_exec(db, sql, nil, nil, nil) == SQLITE_OK)
    }
    
    func query<T: BoyiaData>(sql: String) -> [T]? {
        var stmt : OpaquePointer? = nil
        if sqlite3_prepare_v2(db, sql, -1, &stmt, nil) != SQLITE_OK {
            BoyiaLog.d("sqlite3_prepare_v2 error");
            return nil
        }
        
        //let mirror = Mirror(reflecting: obj)
        
//        for prop in mirror.children {
//            
//        }
        var list = [T]()
        // 如果还有其他的行
        while sqlite3_step(stmt) == SQLITE_ROW {
            let columnCount = sqlite3_column_count(stmt)
            for i in 0..<columnCount {
                let obj = T()
                list.append(obj)
                let mirror = Mirror(reflecting: obj)
                // 取出列的名称
                setFieldValue(m: mirror, index: i, stmt: stmt)
            }
        }
        // 重置语句
        sqlite3_reset(stmt)
        // 释放资源
        sqlite3_finalize(stmt)
        
        return list
    }
    
    func setFieldValue(m: Mirror, index: Int32, stmt: OpaquePointer?) {
        // 本地数据库中获取的名称需要转为小驼峰
        let name = String(cString: sqlite3_column_name(stmt, index)!, encoding: .utf8)?.camelName
        print(name!)
        
        let type = sqlite3_column_type(stmt, index)
        
        for prop in m.children {
            if prop.label != name {
                continue
            }
            
            if type == SQLITE_INTEGER {
                //let value = sqlite3_column_int(stmt, i)
                guard let v = prop.value as? IntObject else {
                    return
                }
                v.value = Int(sqlite3_column_int(stmt, index)) 
                        
            } else if type == SQLITE_FLOAT {
                guard let v = prop.value as? DoubleObject else {
                    return
                }
                
                v.value = sqlite3_column_double(stmt, index)
            } else if type == SQLITE_TEXT {
                guard let v = prop.value as? StringObject else {
                    return
                }
                
                let value = String(cString: sqlite3_column_text(stmt, index)!)
                v.value = value
            }
            
            return
        }
    }
        
    
    func close() {
        if db != nil {
            sqlite3_close(db)
            db = nil
        }
    }
}
