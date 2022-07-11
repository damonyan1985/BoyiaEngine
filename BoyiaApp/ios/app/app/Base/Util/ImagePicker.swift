//
//  ImagePicker.swift
//  app
//
//  Created by yanbo on 2022/6/17.
//

import Foundation
import SwiftUI
import Photos
import PhotosUI

struct ImagePicker : UIViewControllerRepresentable {
    let handleImage: (_ path: String) -> Void
    
    func makeCoordinator() -> Coordinator {
        return Coordinator(self)
    }
    
    func makeUIViewController(context: UIViewControllerRepresentableContext<ImagePicker>) -> PHPickerViewController {//UIImagePickerController {
//        let picker = UIImagePickerController()
//        picker.allowsEditing = false
//        picker.sourceType = UIImagePickerController.SourceType.photoLibrary
//        picker.delegate = context.coordinator
//        return picker
        
        var configuration = PHPickerConfiguration(photoLibrary: PHPhotoLibrary.shared())
             
        configuration.filter = .images
         
        let controller = PHPickerViewController(configuration: configuration)
        controller.delegate = context.coordinator
        return controller
    }
    
    func updateUIViewController(_ uiViewController: UIViewControllerType, context: Context) {
//        if isPresented {
//            PHPhotoLibrary.shared().presentLimitedLibraryPicker(from: uiViewController)
//            DispatchQueue.main.async {
//                isPresented = false
//            }
//        }
    }
    
    class Coordinator : NSObject, UINavigationControllerDelegate, PHPickerViewControllerDelegate {
        //UIImagePickerControllerDelegate {
        let pickerImpl: ImagePicker
        
        init(_ picker: ImagePicker) {
            self.pickerImpl = picker
        }
        
//        func imagePickerController(_ pickerController: UIImagePickerController, didFinishPickingMediaWithInfo info: [UIImagePickerController.InfoKey : Any]) {
////            if let image = info[.originalImage] as? UIImage {
////                self.picker.handleImage(image)
////            }
//            let pickedURL = info[.referenceURL] as! URL
//            let fetchResult: PHFetchResult = PHAsset.fetchAssets(
//                withALAssetURLs: [pickedURL], options: nil)
//        }
    
        func picker(_ picker: PHPickerViewController, didFinishPicking results: [PHPickerResult]) {
            picker.dismiss(animated: true)
            if let itemProvider = results.first?.itemProvider, itemProvider.canLoadObject(ofClass: UIImage.self) {
//                itemProvider.loadObject(ofClass: UIImage.self) { (object, error) in
//                    if let image = object as? UIImage {
//                        self.pickerImpl.handleImage(image)
//                    }
//                }
                itemProvider.loadItem(forTypeIdentifier: UTType.image.identifier, options: nil) { (url, error) in
                    BoyiaLog.d("url = \(url)")
                    guard let uri = url as? URL else {
                        return
                    }
                    
                    self.pickerImpl.handleImage(uri.path)
                }
                
            }
        }
    }
}
