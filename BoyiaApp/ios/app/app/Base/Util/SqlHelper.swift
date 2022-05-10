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
    
    func open() -> Bool {
        let docDir: String! = NSSearchPathForDirectoriesInDomains(
            FileManager.SearchPathDirectory.documentDirectory,
            FileManager.SearchPathDomainMask.userDomainMask, true).first
        
        let path =  docDir + "/" + SqlConstants.DB_NAME
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
                let columnName = sqlite3_column_name(stmt, i)
                let columnNameStr = String(cString: columnName!, encoding: .utf8)
                print(columnNameStr!)
                // 取出列的值
                // 不同的数据类型，需要执行不同函数来获取
                // 获取每列的数据类型
//                let columnType = sqlite3_column_type(stmt, i)
//                // 根据不同的数据类型，使用不同的函数，获取结果
//                if columnType == SQLITE_INTEGER {
//                    let value = sqlite3_column_int(stmt, i)
//
//                }
//
//                if columnType == SQLITE_FLOAT {
//                    let value = sqlite3_column_double(stmt, i)
//                    print(value)
//                }
//
//                if columnType == SQLITE_TEXT {
//                    let value = sqlite3_column_text(stmt, i)
//                    //let valueInt8 = UnsafePointer<CChar>(value)
//                    let valueStr = String(cString: value!)
//                    print(valueStr)
//                }
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
        let name = String(cString: sqlite3_column_name(stmt, index)!, encoding: .utf8)
        print(name!)
        
        let type = sqlite3_column_type(stmt, index)
        
        for prop in m.children {
//            if let value = prop.value as? String {
//
//            }
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
