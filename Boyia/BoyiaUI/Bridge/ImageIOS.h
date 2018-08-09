//
//  ImageIOS.h
//  Boyia
//
//  Created by yanbo on 2018/8/9.
//  Copyright © 2018年 yanbo. All rights reserved.
//



#import <UIKit/UIKit.h>

@interface ImageIOS : NSObject
{
    UIImage* m_image;
}

-（void）setWithData:(NSData*) data second : (int) width third: (int) height;
@end
