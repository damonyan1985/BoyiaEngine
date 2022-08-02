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
    BatchCommandNormal = 0,
    BatchCommandTexture,
    BatchCommendExternalTexture,
    BatchCommandRound,
};

@interface IOSTexture : NSObject
-(instancetype)initTexture:(id<MTLTexture>)tex;
-(instancetype)initTexture:(id<MTLTexture>)tex andPixel:(CVPixelBufferRef)ref;
@end

// IOSRenderer除了作为renderer使用，
// 还有一个用途就是作为boyia和oc之间的桥梁
@interface IOSRenderer : NSObject

+(IOSRenderer*)renderer;

+(IOSRenderer*)initRenderer:(CAMetalLayer*)layer;
+(void)runOnUiThread:(dispatch_block_t)block;
+(UIWindow*)getKeyWindow;
-(void)showKeyboard:(NSString*)text cursor:(int)cursor;

-(instancetype)initWithLayer:(CAMetalLayer*)layer;
-(void)render;
-(CAMetalLayer*)layer;
-(float)getRenderStatusBarHight;
//-(UIFont*)getIconFont:(CGFloat)size;
-(void)initFonts;

-(void)clearBatchCommandBuffer;
-(BOOL)appendBatchCommand:(BatchCommandType)cmdType size:(NSInteger)size key:(NSString*)key;
-(void)setVerticeBuffer:(const void*)buffer size:(NSUInteger)size;
//-(void)setUniformBuffer:(const void*)buffer size:(NSUInteger)size;


-(void)setTextureData:(NSString*)key data:(Byte*)data width:(NSUInteger)width height:(NSUInteger)height;

-(void)setTexture:(NSString*)key texture:(id<MTLTexture>)texture;
-(void)setTexture:(NSString*)key texture:(id<MTLTexture>)texture pixel:(CVPixelBufferRef)ref;
-(id<MTLTexture>)getTexture:(NSString*)key;

-(void)handleTouchEvent:(int)type point:(CGPoint)point;
-(void)onFling:(CGPoint)speed pointStart:(CGPoint)start pointEnd:(CGPoint)end;
-(void)setInputText:(NSString*)text cursor:(NSUInteger)cursor;

-(CVMetalTextureCacheRef)cvTextureCache;
-(void)cacheCode;
@end


#endif /* IOSRender_h */
