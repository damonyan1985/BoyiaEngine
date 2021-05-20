extern crate proc_macro;

use std::collections::HashSet;
use std::convert::TryFrom;

use proc_macro::TokenStream;
use proc_macro2::{Span, TokenStream as TokenStream2};
use quote::{format_ident, quote, ToTokens, TokenStreamExt};
use syn::{parse_macro_input, AttributeArgs, Ident, NestedMeta};

macro_rules! method_type {
  (
      $($variant:ident, $upper:ident,)+
  ) => {
      #[derive(Debug, PartialEq, Eq, Hash)]
      pub enum MethodType {
          $(
              $variant,
          )+
      }

      impl MethodType {
          fn as_str(&self) -> &'static str {
              match self {
                  $(Self::$variant => stringify!($variant),)+
              }
          }

          fn parse(method: &str) -> Result<Self, String> {
              match method {
                  $(stringify!($upper) => Ok(Self::$variant),)+
                  _ => Err(format!("Unexpected HTTP method: `{}`", method)),
              }
          }
      }
  };
}

method_type! {
  Get,       GET,
  Post,      POST,
  Put,       PUT,
  Delete,    DELETE,
  Head,      HEAD,
  Connect,   CONNECT,
  Options,   OPTIONS,
  Trace,     TRACE,
  Patch,     PATCH,
}

pub struct Route {
  name: syn::Ident,
  args: AttributeArgs,
  ast: syn::ItemFn,
}

// Self表示类型, &self表示对象引用
impl Route {
  pub fn new(
    args: AttributeArgs,
    input: TokenStream,
    method: Option<MethodType>,) -> syn::Result<Self> {
      //println!("cargo:warning=Start Route.1");
      println!("Start Route.");
      if args.is_empty() {
        return Err(syn::Error::new(
          Span::call_site(),
          format!(
              r#"invalid service definition, expected #[{}("<some path>")]"#,
              method
                  .map(|it| it.as_str())
                  .unwrap_or("route")
                  .to_ascii_lowercase()
          ),
        ));
      }

      // 解析目标函数，转换成AST
      let ast: syn::ItemFn = syn::parse(input)?;
      // 标识符名，即函数名
      let name = ast.sig.ident.clone();
      println!("Start Route name={}", name);
      Ok(Self {
        name,
        args,
        ast,
      })
  }
}

impl ToTokens for Route {
  fn to_tokens(&self, output: &mut TokenStream2) {
    let Self {
      name,
      args,
      ast,
    } = self;
    println!("Start ToTokens");
    let resource_name = name.to_string();
    println!("Start ToTokens {}", resource_name);
    let stream = quote! {
      #[allow(non_camel_case_types, missing_docs)]
      pub struct #name;

      impl IService for #name {
        // fn register(&self, sm: &ServiceManager) {
        //   sm.register(resource_name, self);
        // }
        fn register(&self) {
        }
      }
    };

    println!("Start ToTokens stream {}", stream);
    output.extend(stream);
  }
}


// HTTP服务路由开发
pub(crate) fn with_method(
  method: Option<MethodType>,
  args: TokenStream,
  input: TokenStream,
) -> TokenStream {
  let args = parse_macro_input!(args as syn::AttributeArgs);
  match Route::new(args, input, method) {
      Ok(route) => route.into_token_stream().into(),
      Err(err) => err.to_compile_error().into(),
  }
}