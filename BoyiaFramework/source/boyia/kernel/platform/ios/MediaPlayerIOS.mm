#ifndef MediaPlayerIOS_h
#define MediaPlayerIOS_h

#include "LGdi.h"‘

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <GLKit/GLKit.h>

// IOS mediaplayer实现
@interface MediaPlayerImpl : NSObject

@property (nonatomic, strong) AVPlayer* player;
@property (nonatomic, strong) AVPlayerItemVideoOutput* videoOutput;
@property (nonatomic, strong) CADisplayLink* displayLink;
@property (nonatomic) CGAffineTransform preferredTransform;

@property (nonatomic, assign) bool isPlaying;
@property (nonatomic, assign) bool isInitialized;
@property (nonatomic, assign) bool disposed;

-(instancetype)initWithURL:(NSURL*)url;
-(void)play;
-(void)pause;

@end

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

-(instancetype)initWithURL:(NSURL*)url {
    AVPlayerItem* item = [AVPlayerItem playerItemWithURL:url];
    return [self initWithPlayerItem:item];
}

- (instancetype)initWithPlayerItem:(AVPlayerItem*)item {
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
        
        [self createVideoOutput];
        [self addObservers:item];
        [asset loadValuesAsynchronouslyForKeys:@[ @"tracks" ] completionHandler:assetCompletionHandler];
    }
    
    return self;
}

-(void)createVideoOutput{
    NSDictionary* pixBuffAttributes = @{
        (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA),
        (id)kCVPixelBufferIOSurfacePropertiesKey : @{}
    };
    
    self.displayLink = [CADisplayLink displayLinkWithTarget:self
                                               selector:@selector(onDisplayLink:)];
    self.videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:pixBuffAttributes];
}

-(void)onDisplayLink:(CADisplayLink*)link {
    
}

-(void)addObservers:(AVPlayerItem*)item {
    // 监听item的loadedTimeRanges属性
    [item addObserver:self
         forKeyPath:@"loadedTimeRanges"
            options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
            context:timeRangeContext];
    
    // 监听item的loadedTimeRanges属性
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

// 监听对象的属性
-(void)observeValueForKeyPath:(NSString*)path
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context {
  if (context == timeRangeContext) {
  } else if (context == statusContext) {
    AVPlayerItem* item = (AVPlayerItem*)object;
    switch (item.status) {
      case AVPlayerItemStatusFailed:
        break;
      case AVPlayerItemStatusUnknown:
        break;
      case AVPlayerItemStatusReadyToPlay:
        // 视频输出到videoOutput上
        [item addOutput:self.videoOutput];
//        [self sendInitialized];
        [self updatePlayingState];
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

- (void)updatePlayingState {
  if (!self.isInitialized) {
    return;
  }
  if (self.isPlaying) {
      [self.player play];
  } else {
      [self.player pause];
  }
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

-(void)disposeChannel {
    self.disposed = true;
    [[_player currentItem] removeObserver:self forKeyPath:@"status" context:statusContext];
    [[_player currentItem] removeObserver:self
                             forKeyPath:@"loadedTimeRanges"
                                context:timeRangeContext];
    [[_player currentItem] removeObserver:self
                             forKeyPath:@"playbackLikelyToKeepUp"
                                context:playbackLikelyToKeepUpContext];
    [[_player currentItem] removeObserver:self
                             forKeyPath:@"playbackBufferEmpty"
                                context:playbackBufferEmptyContext];
    [[_player currentItem] removeObserver:self
                             forKeyPath:@"playbackBufferFull"
                                context:playbackBufferFullContext];
    [_player replaceCurrentItemWithPlayerItem:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

-(void)play {
    self.isPlaying = true;
    [self updatePlayingState];
}

-(void)pause {
    self.isPlaying = false;
    [self updatePlayingState];
}

@end

namespace util {
// TODO
class MediaPlayerIOS : public LMediaPlayer {
public:
    MediaPlayerIOS(LVoid* view);
    ~MediaPlayerIOS();

public:
    virtual void start(const String& url);
    virtual void pause();
    virtual void stop();
    virtual void seek(int progress);
    virtual void updateTexture(float* matrix);

private:
    LVoid* m_view;
};

MediaPlayerIOS::MediaPlayerIOS(LVoid* view)
    : m_view(view)
{
}

MediaPlayerIOS::~MediaPlayerIOS()
{
}

void MediaPlayerIOS::start(const String& url)
{
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

LMediaPlayer* LMediaPlayer::create(LVoid* view)
{
    return new MediaPlayerIOS(view);
}
}

#endif
