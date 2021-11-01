//
//  BoyiaView.m
//  core
//
//  Created by yanbo on 2021/8/22.
//

@import MetalKit;
@import GLKit;

#import "BoyiaView.h"


@interface BoyiaView()

@end


@implementation BoyiaView

// 利用CAMetalLayer类来创建CAMetalLayer对象
+(Class)layerClass {
    return [CAMetalLayer class];
}

// 返回CAMetalLayer对象
-(CAMetalLayer*)metalLayer {
    return (CAMetalLayer*)self.layer;
}

@end
