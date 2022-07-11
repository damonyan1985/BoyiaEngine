//
//  ViewController.m
//  app
//
//  Created by yanbo on 2021/8/20.
//

#import "ViewController.h"
#import "HttpEngineIOS.h"
//#import "IOSRenderer.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    HttpEngineIOS* engine = [HttpEngineIOS alloc];
    
    [engine loadUrl:kHttpGet url:@"https://www.baidu.com" headers:nil callback:nil];
    // Do any additional setup after loading the view.
    //[[ImageLoaderIOS shareInstance] loadImage:@"https://www.baidu.com" clientId:0];
}


@end
