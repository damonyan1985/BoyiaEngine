package com.boyia.app.core.api;

public class ApiConstants {
    public static final String API_METHOD_NAME = "api_method";
    public static final String API_METHOD_PARAMS = "api_params";

    public static class ApiNames {
        public static final String LOCAL_SHARE_SET = "local_share_set";
        public static final String LOCAL_SHARE_GET = "local_share_get";

        /**
         * notification接口
         */
        public static final String NOTIFICATION_NAME = "notification";
        /**
         * 打开相册
         */
        public static final String PICK_IMAGE = "pick_image";

        /**
         * 获取用户信息
         */
        public static final String USER_INFO = "user_info";

        /**
         * 发送ipc binder
         */
        public static final String SEND_BINDER = "send_binder";

        /**
         * 用户登录界面调用
         */
        public static final String USER_LOGIN = "user_login";
    }

    public static class ApiKeys {
        /**
         * 使用IPC_VALUE获取数据
         */
        public static final String IPC_SHARE_KEY = "ipc_key";
        public static final String IPC_SHARE_VALUE = "ipc_value";

        public static final String NOTIFICATION_ACTION = "notification_action";
        public static final String NOTIFICATION_TITLE = "notification_title";
        public static final String NOTIFICATION_ICON = "notification_icon";

        /**
         * 发送给宿主的binder是属于那个aid的
         */
        public static final String BINDER_AID = "binder_aid";
        /**
         * 跨端异步调用回调id
         */
        public static final String CALLBACK_ID = "callback_id";
    }

    public static class ApiRequestCode {
        /**
         * 相册请求code
         */
        public static final int PHOTO_CODE = 1;
    }
}
