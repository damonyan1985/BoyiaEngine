package com.boyia.app.shell.util

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import androidx.appcompat.widget.AppCompatEditText

class UnderlineEditText(context: Context): AppCompatEditText(context) {
    private var paint: Paint? = null

    init {
        paint = Paint()
        paint?.style = Paint.Style.STROKE
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        paint?.color = if (isFocused) 0xFF7FFFD4.toInt() else 0xFFCAE1FF.toInt()
        canvas.drawLine(0F, (this.height - 2).toFloat(), (this.width - 2).toFloat(), (this.height - 2).toFloat(), paint!!)
    }
}