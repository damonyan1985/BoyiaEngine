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
    }

    public static class ApiRequestCode {
        /**
         * 相册请求code
         */
        public static final int PHOTO_CODE = 1;
    }
}
