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
    let handleImage: (_ image: UIImage) -> Void
    
    func makeCoordinator() -> Coordinator {
        return Coordinator(self)
    }
    
    func makeUIViewController(context: UIViewControllerRepresentableContext<ImagePicker>) -> UIImagePickerController {
        let picker = UIImagePickerController()
        picker.allowsEditing = false
        picker.sourceType = UIImagePickerController.SourceType.photoLibrary
        picker.delegate = context.coordinator
        return picker
    }
    
    func updateUIViewController(_ uiViewController: UIViewControllerType, context: Context) {
//        if isPresented {
//            PHPhotoLibrary.shared().presentLimitedLibraryPicker(from: uiViewController)
//            DispatchQueue.main.async {
//                isPresented = false
//            }
//        }
    }
    
    class Coordinator : NSObject, UINavigationControllerDelegate, UIImagePickerControllerDelegate {
        let picker: ImagePicker
        
        init(_ picker: ImagePicker) {
            self.picker = picker
        }
        
        func imagePickerController(_ pickerController: UIImagePickerController, didFinishPickingMediaWithInfo info: [UIImagePickerController.InfoKey : Any]) {
            if let image = info[.originalImage] as? UIImage {
                self.picker.handleImage(image)
            }
        }
    }
}
