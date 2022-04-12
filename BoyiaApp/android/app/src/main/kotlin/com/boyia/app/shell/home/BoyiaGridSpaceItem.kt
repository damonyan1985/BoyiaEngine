package com.boyia.app.shell.home

import android.graphics.Rect
import android.view.View
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.StaggeredGridLayoutManager

class BoyiaGridSpaceItem(private val space: Int): RecyclerView.ItemDecoration() {
    override fun getItemOffsets(outRect: Rect , view: View,
                                parent: RecyclerView , state: RecyclerView.State) {
        if (parent.layoutManager is StaggeredGridLayoutManager) {
            outRect.top = space
            outRect.right = space
            if (parent.getChildLayoutPosition(view) % 3 == 0) {
                outRect.left = space
            }
        }
    }
}