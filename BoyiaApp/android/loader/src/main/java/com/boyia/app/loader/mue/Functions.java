package com.boyia.app.loader.mue;

/*
 * Functions
 * Author yanbo.boyia
 * All Copyright reserved
 */
public class Functions {
    public static final Action EMPTY_ACTION = () -> {};
    public static <T> ParamAction<T> emptyParamAction() {
        return t -> {};
    }
}
