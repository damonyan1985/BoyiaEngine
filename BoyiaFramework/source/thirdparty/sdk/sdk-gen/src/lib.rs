use proc_macro::TokenStream;

// args表示宏的参数
// input为宏修饰的函数或者属性的token流
#[proc_macro_attribute]
pub fn get(args: TokenStream, input: TokenStream) -> TokenStream {
    input
}
