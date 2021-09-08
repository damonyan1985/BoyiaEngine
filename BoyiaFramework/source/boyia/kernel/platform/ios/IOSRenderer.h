//
//  IOSRender.h
//  core
//
//  Created by yanbo on 2021/8/28.
//

#ifndef IOSRenderer_h
#define IOSRenderer_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#define STR_TO_OCSTR(str) ([[NSString alloc] initWithUTF8String: GET_STR(str)])

#define METAL_COLOR_BIT(bit) ((float)bit/255)
#define METAL_COLOR(color) {\
    METAL_COLOR_BIT(color.m_red),\
    METAL_COLOR_BIT(color.m_green),\
    METAL_COLOR_BIT(color.m_blue),\
    METAL_COLOR_BIT(color.m_alpha)\
}

typedef NS_ENUM(NSInteger, BatchCommandType) {
    BatchCommandNormal,
    BatchCommandTexture
};

// IOSRenderer除了作为renderer使用，
// 还有一个用途就是作为boyia和oc之间的桥梁
@interface IOSRenderer : NSObject

+(IOSRenderer*)initRenderer:(CAMetalLayer*)layer;
+(void)runOnUiThead:(dispatch_block_t)block;

-(instancetype)initWithLayer:(CAMetalLayer*)layer;
-(void)render;
-(CAMetalLayer*)layer;

-(void)clearBatchCommandBuffer;
-(void)appendBatchCommand:(BatchCommandType)cmdType size:(NSInteger)size key:(NSString*)key;
-(void)setBuffer:(const void*)buffer size:(NSUInteger)size;
-(void)setTextureData:(NSString*)key data:(Byte*)data width:(NSUInteger)width height:(NSUInteger)height;
-(id<MTLTexture>)getTexture:(NSString*)key;

-(void)handleTouchEvent:(int)type x:(int)x y:(int)y;

@end


#endif /* IOSRender_h */
