//
//  Animation.swift
//  app
//
//  Created by yanbo on 2022/5/23.
//

import Foundation
import SwiftUI

struct BoyiaTranslate : AnimatableModifier {
    var initialOffset: Double
    var offset: Double
    var onCompletion: (() -> Void)?

    init(offset: Double, onCompletion: (() -> Void)? = nil) {
        self.initialOffset = offset
        self.offset = offset
        self.onCompletion = onCompletion
    }

    var animatableData: CGFloat {
        get { offset }
        set {
            offset = newValue
            checkIfFinished()
        }
    }

    func checkIfFinished() -> () {
        if let onCompletion = onCompletion, offset == initialOffset {
            DispatchQueue.main.async {
                onCompletion()
            }
        }
    }

    func body(content: Content) -> some View {
        content.offset(x: offset)
    }
}
