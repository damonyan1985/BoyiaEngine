use proc_macro::TokenStream;

// 本地服务端请求路由宏开发

// args表示宏的参数
// input为宏修饰的函数或者属性的token流
#[proc_macro_attribute]
pub fn get(_args: TokenStream, input: TokenStream) -> TokenStream {
    input
}
