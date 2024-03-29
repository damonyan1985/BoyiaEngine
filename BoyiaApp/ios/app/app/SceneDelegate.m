//
//  SceneDelegate.m
//  app
//
//  Created by yanbo on 2021/8/20.
//

#import "SceneDelegate.h"
#import "BoyiaViewController.h"
// app-Swift.h是由xcode自动生成的
// xcode会将swift对外接口编译成oc头文件
#import "app-Swift.h"

@interface SceneDelegate ()

@end

@implementation SceneDelegate

// IOS AppIcon设置的时候必须是正方形不能带圆角，否则可能会出现黑边，因为系统会自己处理方形图标
// 此处添加根视图控制器
-(void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    // Use this method to optionally configure and attach the UIWindow `window` to the provided UIWindowScene `scene`.
    // If using a storyboard, the `window` property will automatically be initialized and attached to the scene.
    // This delegate does not imply the connecting scene or session are new (see `application:configurationForConnectingSceneSession` instead).
    UIWindowScene* windowScene = (UIWindowScene*)scene;
    self.window = [[UIWindow alloc] initWithWindowScene:windowScene];
    self.window.frame = windowScene.coordinateSpace.bounds;
    // 开屏controller
    self.window.rootViewController = [[UINavigationController alloc] initWithRootViewController:[SplashViewController new]];
    
//    self.window.rootViewController = [[UINavigationController alloc] initWithRootViewController:[BoyiaWebController new]];
    
//    self.window.rootViewController = [[UINavigationController alloc] initWithRootViewController:[BoyiaWebController new]];
    [self.window makeKeyAndVisible];
}


-(void)sceneDidDisconnect:(UIScene *)scene {
    // Called as the scene is being released by the system.
    // This occurs shortly after the scene enters the background, or when its session is discarded.
    // Release any resources associated with this scene that can be re-created the next time the scene connects.
    // The scene may re-connect later, as its session was not necessarily discarded (see `application:didDiscardSceneSessions` instead).
}


- (void)sceneDidBecomeActive:(UIScene *)scene {
    // Called when the scene has moved from an inactive state to an active state.
    // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
}


- (void)sceneWillResignActive:(UIScene *)scene {
    // Called when the scene will move from an active state to an inactive state.
    // This may occur due to temporary interruptions (ex. an incoming phone call).
}


- (void)sceneWillEnterForeground:(UIScene *)scene {
    // Called as the scene transitions from the background to the foreground.
    // Use this method to undo the changes made on entering the background.
}


- (void)sceneDidEnterBackground:(UIScene *)scene {
    // Called as the scene transitions from the foreground to the background.
    // Use this method to save data, release shared resources, and store enough scene-specific state information
    // to restore the scene back to its current state.
}


@end
