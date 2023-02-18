package com.boyia.app.loader.mue;

/**
 * 数据从T类型转换成R类型
 * @param <T>
 * @param <R>
 */
public interface MueFunction<T, R> {
    R apply(T t);
}
