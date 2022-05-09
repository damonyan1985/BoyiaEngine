#ifndef MediaPlayerIOS_h
#define MediaPlayerIOS_h

#include "LGdi.h"
#include "UIThreadClientMap.h"
#include "UIThread.h"
#include "AppManager.h"

#import "IOSRenderer.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <GLKit/GLKit.h>

static int64_t timeToMillis(CMTime time) {
  if (time.timescale == 0) return 0;
  return time.value * 1000 / time.timescale;
}

// IOS mediaplayer实现
@interface MediaPlayerImpl : NSObject

@property (nonatomic, strong) AVPlayer* player;
@property (nonatomic, strong) AVPlayerItemVideoOutput* videoOutput;
@property (nonatomic, strong) CADisplayLink* displayLink;
@property (nonatomic) CGAffineTransform preferredTransform;

@property (nonatomic, assign) bool isPlaying;
@property (nonatomic, assign) bool isInitialized;
@property (nonatomic, assign) bool disposed;
@property (nonatomic, assign) int clientId;

-(instancetype)initWithURL:(NSURL*)url andClient: (int)clientId;
-(void)play;
-(void)pause;

@end

// 这里的指针应该是表示成一个常量
static void* timeRangeContext = &timeRangeContext;
static void* statusContext = &statusContext;
static void* playbackLikelyToKeepUpContext = &playbackLikelyToKeepUpContext;
static void* playbackBufferEmptyContext = &playbackBufferEmptyContext;
static void* playbackBufferFullContext = &playbackBufferFullContext;

static inline CGFloat radiansToDegrees(CGFloat radians) {
  // Input range [-pi, pi] or [-180, 180]
  CGFloat degrees = GLKMathRadiansToDegrees((float)radians);
  if (degrees < 0) {
    // Convert -90 to 270 and -180 to 180
    return degrees + 360;
  }
  // Output degrees in between [0, 360[
  return degrees;
};

@implementation MediaPlayerImpl

// 初始化，传入url
-(instancetype)initWithURL:(NSURL*)url andClient: (int)clientId {
    AVPlayerItem* item = [AVPlayerItem playerItemWithURL:url];
    return [self initWithPlayerItem:item andClient:clientId];
}

-(instancetype)initWithPlayerItem:(AVPlayerItem*)item andClient: (int)clientId {
    self = [super init];
    if (self != nil) {
        AVAsset* asset = [item asset];
        void (^assetCompletionHandler)(void) = ^{
            if ([asset statusOfValueForKey:@"tracks" error:nil] == AVKeyValueStatusLoaded) {
                NSArray* tracks = [asset tracksWithMediaType:AVMediaTypeVideo];
                if ([tracks count] > 0) {
                    AVAssetTrack* videoTrack = tracks[0];
                    void (^trackCompletionHandler)(void) = ^{
                        if (self.disposed) return;
                        if ([videoTrack statusOfValueForKey:@"preferredTransform"
                                                    error:nil] == AVKeyValueStatusLoaded) {
                            // Rotate the video by using a videoComposition and the preferredTransform
                            self.preferredTransform = [self fixTransform:videoTrack];
                            // Note:
                            // https://developer.apple.com/documentation/avfoundation/avplayeritem/1388818-videocomposition
                            // Video composition can only be used with file-based media and is not supported for
                            // use with media served using HTTP Live Streaming.
                            AVMutableVideoComposition* videoComposition =
                                [self getVideoCompositionWithTransform:self->_preferredTransform
                                                             withAsset:asset
                                                        withVideoTrack:videoTrack];
                            item.videoComposition = videoComposition;
                        }
                    };
                    [videoTrack loadValuesAsynchronouslyForKeys:@[ @"preferredTransform" ]
                                          completionHandler:trackCompletionHandler];
                }
            }
          };
        
        self.disposed = false;
        self.isInitialized = false;
        self.isPlaying = false;
        self.player = [AVPlayer playerWithPlayerItem:item];
        self.player.actionAtItemEnd = AVPlayerActionAtItemEndNone;
        self.clientId = clientId;
        
        // 将视频纹理输出到AVPlayerItemVideoOutput中
        [self createVideoOutput];
        [self addObservers:item];
        [asset loadValuesAsynchronouslyForKeys:@[ @"tracks" ] completionHandler:assetCompletionHandler];
    }
    
    return self;
}

// 图像输出
-(void)createVideoOutput{
    NSDictionary* pixBuffAttributes = @{
        (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA),
        (id)kCVPixelBufferIOSurfacePropertiesKey : @{}
    };
    self.videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:pixBuffAttributes];
    self.displayLink = [CADisplayLink displayLinkWithTarget:self
                                               selector:@selector(onDisplayLink:)];
    
    // 初始化播放器时停止垂直同步信号
    self.displayLink.paused = YES;
    // 必须加入带有NSRunLoop的线程
    dispatch_async(dispatch_get_main_queue(), ^{
        [self->_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    });
}

// 获取视频的每一帧
-(void)onDisplayLink:(CADisplayLink*)link {
    // 通知UI更新视频纹理
    yanbo::AppManager::instance()->uiThread()->clientCallback(self.clientId);
}

-(void)addObservers:(AVPlayerItem*)item {
    // 监听item的loadedTimeRanges属性
    [item addObserver:self
         forKeyPath:@"loadedTimeRanges"
            options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
            context:timeRangeContext];
    
    // 监听item的状态属性
    [item addObserver:self
         forKeyPath:@"status"
            options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
            context:statusContext];
    
    // 监听item的playbackLikelyToKeepUp属性
    [item addObserver:self
         forKeyPath:@"playbackLikelyToKeepUp"
            options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
            context:playbackLikelyToKeepUpContext];
    
    // 监听item的playbackBufferEmpty属性
    [item addObserver:self
         forKeyPath:@"playbackBufferEmpty"
            options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
            context:playbackBufferEmptyContext];
    
    // 监听item的playbackBufferFull属性
    [item addObserver:self
         forKeyPath:@"playbackBufferFull"
            options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
            context:playbackBufferFullContext];

    // Add an observer that will respond to itemDidPlayToEndTime
    [[NSNotificationCenter defaultCenter] addObserver:self
                                           selector:@selector(itemDidPlayToEndTime:)
                                               name:AVPlayerItemDidPlayToEndTimeNotification
                                             object:item];
}

// 播放结束通知
-(void)itemDidPlayToEndTime:(NSNotification*)notification {
//  if (_isLooping) {
//    AVPlayerItem* p = [notification object];
//    [p seekToTime:kCMTimeZero completionHandler:nil];
//  } else {
//    if (_eventSink) {
//      _eventSink(@{@"event" : @"completed"});
//    }
//  }
}

// 监听对象的属性，除了对context进行比较，其实也可以对keypath进行比较
// 如[path isEqualToString:@"status"]
-(void)observeValueForKeyPath:(NSString*)path
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context {
  if (context == timeRangeContext) {
  } else if (context == statusContext) {
    AVPlayerItem* item = (AVPlayerItem*)object;
    switch (item.status) {
      case AVPlayerItemStatusFailed: // 播放失败
        break;
      case AVPlayerItemStatusUnknown: // 未知错误
        break;
      case AVPlayerItemStatusReadyToPlay: // 准备播放，类似android的onPrepared
        // 视频输出到videoOutput上
        [item addOutput:self.videoOutput];
//        [self sendInitialized];
        self.isInitialized = true;
        //[self updatePlayingState];
        // 准备完毕之后开始播放
        [self play];
        break;
    }
  } else if (context == playbackLikelyToKeepUpContext) {
    if ([[_player currentItem] isPlaybackLikelyToKeepUp]) {
        [self updatePlayingState];
    }
  } else if (context == playbackBufferEmptyContext) {
  } else if (context == playbackBufferFullContext) {
  }
}

-(void)updatePlayingState {
    if (!self.isInitialized) {
        return;
    }
    
    if (self.isPlaying) {
      [self.player play];
    } else {
      [self.player pause];
    }
    
    // 播放器播放需要设置paused为NO
    _displayLink.paused = !self.isPlaying;
}

-(AVMutableVideoComposition*)getVideoCompositionWithTransform:(CGAffineTransform)transform
                                                     withAsset:(AVAsset*)asset
                                                withVideoTrack:(AVAssetTrack*)videoTrack {
  AVMutableVideoCompositionInstruction* instruction =
      [AVMutableVideoCompositionInstruction videoCompositionInstruction];
  instruction.timeRange = CMTimeRangeMake(kCMTimeZero, [asset duration]);
  AVMutableVideoCompositionLayerInstruction* layerInstruction =
      [AVMutableVideoCompositionLayerInstruction
          videoCompositionLayerInstructionWithAssetTrack:videoTrack];
  [layerInstruction setTransform:_preferredTransform atTime:kCMTimeZero];

  AVMutableVideoComposition* videoComposition = [AVMutableVideoComposition videoComposition];
  instruction.layerInstructions = @[ layerInstruction ];
  videoComposition.instructions = @[ instruction ];

  // If in portrait mode, switch the width and height of the video
  CGFloat width = videoTrack.naturalSize.width;
  CGFloat height = videoTrack.naturalSize.height;
  NSInteger rotationDegrees =
      (NSInteger)round(radiansToDegrees(atan2(_preferredTransform.b, _preferredTransform.a)));
  if (rotationDegrees == 90 || rotationDegrees == 270) {
    width = videoTrack.naturalSize.height;
    height = videoTrack.naturalSize.width;
  }
  videoComposition.renderSize = CGSizeMake(width, height);

  // TODO(@recastrodiaz): should we use videoTrack.nominalFrameRate ?
  // Currently set at a constant 30 FPS
  videoComposition.frameDuration = CMTimeMake(1, 30);

  return videoComposition;
}

-(CGAffineTransform)fixTransform:(AVAssetTrack*)videoTrack {
    CGAffineTransform transform = videoTrack.preferredTransform;
    // TODO(@recastrodiaz): why do we need to do this? Why is the preferredTransform incorrect?
    // At least 2 user videos show a black screen when in portrait mode if we directly use the
    // videoTrack.preferredTransform Setting tx to the height of the video instead of 0, properly
    // displays the video https://github.com/flutter/flutter/issues/17606#issuecomment-413473181
    if (transform.tx == 0 && transform.ty == 0) {
        NSInteger rotationDegrees = (NSInteger)round(radiansToDegrees(atan2(transform.b, transform.a)));
        NSLog(@"TX and TY are 0. Rotation: %ld. Natural width,height: %f, %f", (long)rotationDegrees,
              videoTrack.naturalSize.width, videoTrack.naturalSize.height);
        if (rotationDegrees == 90) {
          NSLog(@"Setting transform tx");
          transform.tx = videoTrack.naturalSize.height;
          transform.ty = 0;
        } else if (rotationDegrees == 270) {
          NSLog(@"Setting transform ty");
          transform.tx = 0;
          transform.ty = videoTrack.naturalSize.width;
        }
    }
    return transform;
}

// 拷贝videoOutput中的数据生成CVPixelBufferRef
-(CVPixelBufferRef)copyPixelBuffer {
    CMTime outputItemTime = [self.videoOutput itemTimeForHostTime:CACurrentMediaTime()];
    if ([self.videoOutput hasNewPixelBufferForItemTime:outputItemTime]) {
        // 根据时间获取图像数据
        return [self.videoOutput copyPixelBufferForItemTime:outputItemTime itemTimeForDisplay:NULL];
    } else {
        return NULL;
    }
}

-(void)disposePlayer {
    self.disposed = true;
    [[self.player currentItem] removeObserver:self
                               forKeyPath:@"status"
                                  context:statusContext];
    [[self.player currentItem] removeObserver:self
                             forKeyPath:@"loadedTimeRanges"
                                context:timeRangeContext];
    [[self.player currentItem] removeObserver:self
                             forKeyPath:@"playbackLikelyToKeepUp"
                                context:playbackLikelyToKeepUpContext];
    [[self.player currentItem] removeObserver:self
                             forKeyPath:@"playbackBufferEmpty"
                                context:playbackBufferEmptyContext];
    [[self.player currentItem] removeObserver:self
                             forKeyPath:@"playbackBufferFull"
                                context:playbackBufferFullContext];
    [self.player replaceCurrentItemWithPlayerItem:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

// 播放视频
-(void)play {
    self.isPlaying = true;
    [self updatePlayingState];
}

// 暂停播放
-(void)pause {
    self.isPlaying = false;
    [self updatePlayingState];
}

// 获取当前播放时长
-(int64_t)position {
  return timeToMillis([self.player currentTime]);
}

/// 获取视频总时长
-(int64_t)duration {
  return timeToMillis([[self.player currentItem] duration]);
}

@end

namespace util {
// TODO
class MediaPlayerIOS : public LMediaPlayer, public yanbo::UIThreadClient {
public:
    MediaPlayerIOS(LVoid* view);
    ~MediaPlayerIOS();

public:
    virtual void start(const String& url);
    virtual void pause();
    virtual void stop();
    virtual void seek(int progress);
    virtual void updateTexture(float* matrix);
    virtual LVoid onClientCallback();

private:
    LVoid* m_view;
    MediaPlayerImpl* m_impl;
};

MediaPlayerIOS::MediaPlayerIOS(LVoid* view)
    : m_view(view)
    , m_impl(kBoyiaNull)
{
}

MediaPlayerIOS::~MediaPlayerIOS()
{
}

void MediaPlayerIOS::start(const String& url)
{
    NSURL* reqUrl = [NSURL URLWithString:STR_TO_OCSTR(url)];
    m_impl = [[MediaPlayerImpl alloc] initWithURL:reqUrl andClient:getClientId()];
}

void MediaPlayerIOS::updateTexture(float* matrix)
{
}

void MediaPlayerIOS::seek(int progress)
{
}

void MediaPlayerIOS::pause()
{
}

void MediaPlayerIOS::stop()
{
}

void MediaPlayerIOS::onClientCallback()
{
    // 处理每一帧数据
    CVPixelBufferRef pixelBuffer = [m_impl copyPixelBuffer];
//    size_t width = CVPixelBufferGetWidth(pixelBuffer);
//    size_t height = CVPixelBufferGetHeight(pixelBuffer);
//
//    OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);
//    if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange ||
//        pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
//        // TODO
//    } else if (pixelFormat == kCVPixelFormatType_32BGRA) {
//
//    }
    
    
    setPlayerId((LIntPtr)pixelBuffer);
    
    // 绘制当前的view
    yanbo::UIThread::instance()->drawUI(m_view);
}

LMediaPlayer* LMediaPlayer::create(LVoid* view)
{
    return new MediaPlayerIOS(view);
}
}

#endif
