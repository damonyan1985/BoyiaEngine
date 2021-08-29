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

@interface IOSRenderer : NSObject

-(instancetype)initWithLayer:(CAMetalLayer*)layer;
-(void)render;
-(CAMetalLayer*)layer;
-(void)setBuffer:(const void*)buffer size:(NSUInteger)size;

@end


#endif /* IOSRender_h */
