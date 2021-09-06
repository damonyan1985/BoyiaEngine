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

@interface IOSRenderer : NSObject

-(instancetype)initWithLayer:(CAMetalLayer*)layer;
-(void)render;
-(CAMetalLayer*)layer;

-(void)setBuffer:(const void*)buffer size:(NSUInteger)size;
-(void)setTextureData:(NSString*)key data:(Byte*)data width:(NSUInteger)width height:(NSUInteger)height;
-(id<MTLTexture>)getTexture:(NSString*)key;

@end


#endif /* IOSRender_h */