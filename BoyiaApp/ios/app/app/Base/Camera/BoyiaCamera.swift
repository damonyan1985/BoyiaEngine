//
//  BoyiaCamera.swift
//  app
//
//  Created by yanbo on 2022/3/23.
//

import Foundation
import AVFoundation
import UIKit

class BoyiaCamera : NSObject {
    var captureDevice: AVCaptureDevice?;
    
    override init() {
        super.init();
        self.initCamera();
    }
    
    // 默认使用前置摄像头
    func initCamera(position: AVCaptureDevice.Position = .front) {
        let devices = AVCaptureDevice.DiscoverySession(deviceTypes: [AVCaptureDevice.DeviceType.builtInWideAngleCamera], mediaType: AVMediaType.video, position: AVCaptureDevice.Position.front).devices;
        
        
        guard let device = devices.filter({$0.position == position}).first else {
            return;
        }
        
        guard let input = try? AVCaptureDeviceInput(device: device) else {
            BoyiaLog.d("fetch capture input fail");
            return;
        }
        
        self.captureDevice = device;
    }
    
    // 判断相机是否可用
    func isCameraAvail() -> Bool {
        return UIImagePickerController.isSourceTypeAvailable(UIImagePickerController.SourceType.camera);
    }
    
    // 判断闪光灯是否可用
    func isFlashAvail() -> Bool {
        return UIImagePickerController.isFlashAvailable(for: UIImagePickerController.CameraDevice.front);
    }
}
