//
//  Permission.swift
//  app
//
//  Created by yanbo on 2022/5/12.
//

import Foundation
import Photos
import CoreLocation
import UserNotifications

typealias AuthClosurer = ((Bool)->())

class BoyiaPermission {
    private static let locationManager = CLLocationManager()
    // 允许其他类重写
    // 获取相册权限
    class func authPhoto(closurer: @escaping AuthClosurer) {
        let status = PHPhotoLibrary.authorizationStatus(for: .readWrite)
        switch status {
        case .notDetermined:
            PHPhotoLibrary.requestAuthorization(for: .readWrite, handler: { (authStatus) in
                DispatchQueue.main.async {
                    closurer(authStatus == .authorized)
                }
            })
            break
        case .authorized:
            closurer(true)
            break
        case .restricted:
            closurer(false)
            break
        case .denied:
            closurer(false)
            break
        case .limited:
            closurer(true)
            break
        @unknown default:
            closurer(false)
        }
    }
    
    // 获取摄像头权限
    class func authCamera(closurer: @escaping AuthClosurer) {
        let authStatus = AVCaptureDevice.authorizationStatus(for: .video)
        switch authStatus {
        case .notDetermined:
            AVCaptureDevice.requestAccess(for: .video) { (result) in
                if result{
                    DispatchQueue.main.async {
                        closurer(true)
                    }
                }else{
                    DispatchQueue.main.async {
                        closurer(false)
                    }
                }
            }
        case .denied:
            closurer(false)
        case .restricted:
            closurer(false)
        case .authorized:
            closurer(true)
        @unknown default:
            closurer(false)
        }
    }
    
    // 获取麦克风权限
    class func authMicrophone(closurer: @escaping AuthClosurer) {
        let authStatus = AVAudioSession.sharedInstance().recordPermission
        switch authStatus {
        case .undetermined:
            AVAudioSession.sharedInstance().requestRecordPermission { (result) in
                if result{
                    DispatchQueue.main.async {
                        closurer(true)
                    }
                }else{
                    DispatchQueue.main.async {
                        closurer(false)
                    }
                }
            }
        case .denied:
            closurer(false)
        case .granted:
            closurer(true)
        @unknown default:
            closurer(false)
        }
    }
    
    // 获取位置权限
    class func authLocation(closurer: @escaping ((Bool, Bool)->())) {
        if CLLocationManager.locationServicesEnabled() {
            switch locationManager.authorizationStatus {
            case .notDetermined:
                // 由于IOS8中定位的授权机制改变 需要进行手动授权
                locationManager.requestAlwaysAuthorization()
                locationManager.requestWhenInUseAuthorization()
                let status = locationManager.authorizationStatus
                if  status == .authorizedAlways || status == .authorizedWhenInUse {
                    DispatchQueue.main.async {
                        closurer(true && CLLocationManager.locationServicesEnabled(), true)
                    }
                } else {
                    DispatchQueue.main.async {
                        closurer(false, true)
                    }
                }
            case .restricted:
                closurer(false, false)
            case .denied:
                closurer(false, false)
            case .authorizedAlways:
                closurer(true && CLLocationManager.locationServicesEnabled(), false)
            case .authorizedWhenInUse:
                closurer(true && CLLocationManager.locationServicesEnabled(), false)
            @unknown default:
                closurer(false, false)
            }
        }
    }
    
    class func authNotification(closurer: @escaping AuthClosurer) {
        UNUserNotificationCenter.current().getNotificationSettings() { (setttings) in
            switch setttings.authorizationStatus {
            case .notDetermined:
                UNUserNotificationCenter.current().requestAuthorization(options: [.alert, .badge, .carPlay, .sound]) { (result, error) in
                    if result{
                        DispatchQueue.main.async {
                            closurer(true)
                        }
                    }else{
                        DispatchQueue.main.async {
                            closurer(false)
                        }
                    }
                }
            case .denied:
                closurer(false)
            case .authorized:
                closurer(true)
            case .provisional:
                closurer(true)
            case .ephemeral:
                closurer(false)
            @unknown default:
                closurer(false)
            }
        }
    }
        
}
