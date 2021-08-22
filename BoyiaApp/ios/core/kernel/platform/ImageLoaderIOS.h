//
//  ImageLoader.h
//  core
//
//  Created by yanbo on 2021/8/21.
//

#ifndef ImageLoader_h
#define ImageLoader_h

#import <Foundation/Foundation.h>
#import "HttpEngineIOS.h"

@interface ImageLoaderIOS : NSObject

+(instancetype)shareInstance;
-(void)loadImage:(NSString*)url clientId:(int)cid;

@end


#endif /* ImageLoader_h */
