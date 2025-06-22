#pragma once
#include "xtools.h"
#include "xdemux_task.h"
#include "xdecode_task.h"
#include "xvideo_view.h"
#include <Windows.h> 
#include "xaudio_play.h"


class XPlayer :public XThread
{
public:
    // 定义画图回调函数类型
    typedef void (*fDrawCBFun)(LONG nPort, HDC hdc, void* pUserData);

    // 设置视频视图
    void SetView(XVideoView* view);

    LONG GetPort() const { return port_; }
    
    // 注册画图回调函数
    bool RegisterDrawCallback(fDrawCBFun DrawCBFun, void* pUserData);

    // 回调接收音视频包
    void Do(AVPacket* pkt) override;

    // 打开音视频 初始化播放和渲染
    bool Open(const char* url, void* winid);

    void Stop();

    // 主线程 处理同步
    void Main() override;

    // 开启 解封装 音视频解码 和 处理同步的线程
    void Start();

    // 渲染视频 播放音频
    void Update();

    void SetSpeed(float s);

    // 总时长 毫秒
    long long total_ms() { return total_ms_; }

    // 当前播放的位置 毫秒
    long long pos_ms() { return pos_ms_; }

    // 设置视频播放位置，毫秒
    bool Seek(long long ms);

    void Pause(bool is_pause) override;

    // 检查播放器是否应该退出
    bool ShouldExit();

    // 设置音量 (0-100)
    void SetVolume(int volume) {
        audio_volume_ = max(0, min(100, volume));
        if (XAudioPlay::Instance()) {
            XAudioPlay::Instance()->set_volume(audio_volume_);
        }
    }

    // 获取当前音量
    int GetVolume() const {
        return audio_volume_;
    }

    // 获取当前播放速度
    float GetSpeed() const {
        return play_speed_;
    }

protected:
    // 启用/禁用RGB绘图回调
    void EnableRGBDrawing(bool enable);

    //端口号
    LONG port_ = -1;

    // 将视频帧转换为RGB格式
    AVFrame* ConvertToRGB(AVFrame* frame);

    // 创建离屏表面DC
    HDC CreateOffscreenDC(AVFrame* frame);

    // 清理转换资源
    void CleanupConversion();
    bool is_exit_ = false;
    long long total_ms_ = 0;
    long long pos_ms_ = 0;
    int audio_volume_ = 80;      // 默认音量80
    float play_speed_ = 1.0f;    // 默认速度1.0
    XDemuxTask demux_;              // 解封装
    XDecodeTask audio_decode_;      // 音频解码
    XDecodeTask video_decode_;      // 视频解码
    XVideoView* view_ = nullptr;    // 视频渲染

    bool use_rgb_drawing_ = false;  // 是否启用RGB绘图回调
    struct SwsContext* sws_ctx_ = nullptr; // 格式转换上下文
    AVFrame* rgb_frame_ = nullptr;    // RGB帧缓冲区

    // 画图回调相关成员
    fDrawCBFun draw_callback_ = nullptr;
    void* draw_user_data_ = nullptr;
    HDC offscreen_dc_ = nullptr;     // 离屏表面DC
    HBITMAP offscreen_bitmap_ = nullptr; // 离屏位图
    unsigned char* offscreen_bits_ = nullptr; // 离屏位图数据指针
};