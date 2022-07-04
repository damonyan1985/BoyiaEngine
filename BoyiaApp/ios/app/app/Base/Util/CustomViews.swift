//
//  CommonView.swift
//  app
//
//  Created by yanbo on 2022/7/2.
//

import SwiftUI

func marginTop(top: Double) -> some View {
    return VStack{}.frame(width: 0, height: top, alignment: .top);
}

func marginLeft(left: Double) -> some View {
    return HStack{}.frame(width: left, height: 0, alignment: .leading)
}

struct CustomTextField: View {
    var placeholder: Text
    @Binding var text: String
    var editingChanged: (Bool)->() = { _ in }
    var commit: ()->() = { }
    
    var body: some View {
        ZStack(alignment: .leading) {
            if text.isEmpty { placeholder }
            TextField("", text: $text, onEditingChanged: editingChanged, onCommit: commit)
        }
    }
}
