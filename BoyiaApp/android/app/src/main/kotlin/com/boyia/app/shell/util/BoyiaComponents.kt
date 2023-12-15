package com.boyia.app.shell.util

import androidx.compose.foundation.background
import androidx.compose.foundation.interaction.MutableInteractionSource
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.defaultMinSize
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.shape.ZeroCornerSize
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.ExperimentalMaterialApi
import androidx.compose.material.LocalTextStyle
import androidx.compose.material.MaterialTheme
import androidx.compose.material.TextFieldColors
import androidx.compose.material.TextFieldDefaults
import androidx.compose.material.TextFieldDefaults.indicatorLine
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.input.VisualTransformation

@OptIn(ExperimentalMaterialApi::class)
@Composable
fun BoyiaTextField(
        value: String,
        onValueChange: StringCallback,
        modifier: Modifier = Modifier,
        enabled: Boolean = true,
        readOnly: Boolean = false,
        textStyle: TextStyle = LocalTextStyle.current,
        label: @Composable (() -> Unit)? = null,
        isError: Boolean = false,
        visualTransformation: VisualTransformation = VisualTransformation.None,
        singleLine: Boolean = false,
        maxLines: Int = Int.MAX_VALUE,
        interactionSource: MutableInteractionSource = remember { MutableInteractionSource() },
        backgroundColor: Color = Color.Transparent,
        shape: Shape = MaterialTheme.shapes.small.copy(bottomEnd = ZeroCornerSize, bottomStart = ZeroCornerSize),
        colors: TextFieldColors = TextFieldDefaults.textFieldColors(),
        paddingValues: PaddingValues = PaddingValues(dpx(value = 0)),
        cursorBrush: Brush = SolidColor(Color.Black),
        keyboardOptions: KeyboardOptions = KeyboardOptions.Default,
        keyboardActions: KeyboardActions = KeyboardActions(),
        placeholder: @Composable (() -> Unit)? = null,
        leadingIcon: @Composable (() -> Unit)? = null,
        trailingIcon: @Composable (() -> Unit)? = null,
) {
    (BasicTextField (
            value = value,
            modifier = modifier
                    .background(backgroundColor, shape)
                    .indicatorLine(enabled, isError, interactionSource, colors)
                    .defaultMinSize(
                            minWidth = TextFieldDefaults.MinWidth,
                            minHeight = TextFieldDefaults.MinHeight
                    ),
            onValueChange = onValueChange,
            enabled = enabled,
            readOnly = readOnly,
            textStyle = textStyle,
            visualTransformation = visualTransformation,
            keyboardOptions = keyboardOptions,
            keyboardActions = keyboardActions,
            interactionSource = interactionSource,
            singleLine = singleLine,
            maxLines = maxLines,
            cursorBrush = cursorBrush,
            decorationBox = @Composable { innerTextField ->
                TextFieldDefaults.TextFieldDecorationBox(
                        value = value,
                        visualTransformation = visualTransformation,
                        placeholder = {
                            Box(contentAlignment = Alignment.CenterStart,
                                    modifier = Modifier.fillMaxHeight()
                            ) {
                                placeholder?.invoke()
                            }
                        },
                        label = label,
                        innerTextField = {
                            Box(
                                    contentAlignment = Alignment.CenterStart,
                                    modifier = Modifier.fillMaxHeight()
                            ) {
                                innerTextField()
                            }
                        },
                        leadingIcon = leadingIcon,
                        trailingIcon = trailingIcon,
                        singleLine = singleLine,
                        enabled = enabled,
                        isError = isError,
                        interactionSource = interactionSource,
                        colors = colors,
                        contentPadding = paddingValues
                )

            }
    ))
}