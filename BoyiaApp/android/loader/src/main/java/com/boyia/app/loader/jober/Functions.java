package com.boyia.app.loader.jober;

public class Functions {
    public static final Action EMPTY_ACTION = () -> {};
    public static <T> ParamAction<T> emptyParamAction() {
        return t -> {};
    }
}
