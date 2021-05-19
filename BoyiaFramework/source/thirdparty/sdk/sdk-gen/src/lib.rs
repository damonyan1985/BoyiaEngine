use proc_macro::TokenStream;
use syn::{parse_macro_input, ItemFn, AttributeArgs, NestedMeta, Meta, FnArg};
use quote::{quote};

mod route;

macro_rules! method_macro {
    (
        $($variant:ident, $method:ident,)+
    ) => {
        // 通配符来展开宏
        $(
            #[proc_macro_attribute]
            pub fn $method(args: TokenStream, input: TokenStream) -> TokenStream {
                route::with_method(Some(route::MethodType::$variant), args, input)
            }
        )+
    };
}

method_macro! {
    Get,       get,
    Post,      post,
    Put,       put,
    Delete,    delete,
    Head,      head,
    Connect,   connect,
    Options,   options,
    Trace,     trace,
    Patch,     patch,
}

// 本地服务端请求路由宏开发

// args表示宏的参数
// input为宏修饰的函数或者属性的token流
// #[proc_macro_attribute]
// pub fn get(args: TokenStream, func: TokenStream) -> TokenStream {
//     let func = parse_macro_input!(func as ItemFn); // 传入的是一个函数，所以需要使用ItemFn
//     let func_vis = &func.vis; // 函数的访问控制权限，例如pub
//     let func_block = &func.block; // 函数主体部分, 如{}

//     let func_decl = &func.sig; // 函数申明
//     let func_name = &func_decl.ident; // 函数名
//     let func_generics = &func_decl.generics; // 函数泛型
//     let func_inputs = &func_decl.inputs; // 函数输入参数
//     let func_output = &func_decl.output; // 函数返回

//     println!("ident is {}", &func.sig.ident.to_string());

//     // 提取参数，参数可能是多个
//     let params: Vec<_> = func_inputs.iter().map(|i| {
//         match i {
//             // 提取形参的pattern
//             // https://docs.rs/syn/1.0.1/syn/struct.PatType.html
//             FnArg::Typed(ref val) => &val.pat,
//             _ => unreachable!("it's not gonna happen."),
//         }
//     }).collect();
    
//     // 解析args
//     let attrArgs = parse_macro_input!(args as AttributeArgs);
//     let expanded = quote! { // 重新构建函数体
//         #func_vis fn #func_name #func_generics(#func_inputs) #func_output {
//         }
//     };
//     expanded.into()
// }
