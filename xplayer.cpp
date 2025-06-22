#include "xplayer.h"
#include "xaudio_play.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
}
using namespace std;
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "gdi32.lib")

//暂停或者播放
void XPlayer::Pause(bool is_pause)
{
    XThread::Pause(is_pause);
    demux_.Pause(is_pause);
    audio_decode_.Pause(is_pause);
    video_decode_.Pause(is_pause);
    XAudioPlay::Instance()->Pause(is_pause);
}

bool XPlayer::ShouldExit()
{
    // 如果有视图，使用视图的退出状态
    if (view_) {
        return view_->IsExit();
    }

    // 没有视图时，使用播放器自身的退出标志
    return is_exit_;
}

void XPlayer::EnableRGBDrawing(bool enable) {
    use_rgb_drawing_ = enable;
}

// 将视频帧转换为RGB格式
AVFrame* XPlayer::ConvertToRGB(AVFrame* frame)
{
    if (!frame || !frame->data[0]) return nullptr;

    // 如果已经是RGB格式，直接返回
    if (frame->format == AV_PIX_FMT_RGBA ||
        frame->format == AV_PIX_FMT_BGRA ||
        frame->format == AV_PIX_FMT_ARGB)
    {
        return frame;
    }

    // 初始化转换上下文
    if (!sws_ctx_) {
        sws_ctx_ = sws_getContext(
            frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, AV_PIX_FMT_BGRA, // 使用BGRA格式兼容Windows
            SWS_BILINEAR, nullptr, nullptr, nullptr);

        if (!sws_ctx_) {
            cerr << "Failed to create SwsContext" << endl;
            return frame;
        }
    }

    // 创建RGB帧
    if (!rgb_frame_) {
        rgb_frame_ = av_frame_alloc();
        rgb_frame_->format = AV_PIX_FMT_BGRA;
        rgb_frame_->width = frame->width;
        rgb_frame_->height = frame->height;

        if (av_frame_get_buffer(rgb_frame_, 0) < 0) {
            cerr << "Failed to allocate RGB frame buffer" << endl;
            av_frame_free(&rgb_frame_);
            return frame;
        }
    }

    // 执行转换
    sws_scale(sws_ctx_,
        frame->data, frame->linesize,
        0, frame->height,
        rgb_frame_->data, rgb_frame_->linesize);

    return rgb_frame_;
}

HDC XPlayer::CreateOffscreenDC(AVFrame* frame)
{
    if (!frame || !frame->data[0]) return nullptr;

    // 清理之前的资源
    if (offscreen_dc_) {
        if (offscreen_bitmap_) {
            DeleteObject(offscreen_bitmap_);
            offscreen_bitmap_ = nullptr;
        }
        DeleteDC(offscreen_dc_);
        offscreen_dc_ = nullptr;
    }

    // 创建内存DC
    HDC screen_dc = GetDC(nullptr);
    offscreen_dc_ = CreateCompatibleDC(screen_dc);
    ReleaseDC(nullptr, screen_dc);

    if (!offscreen_dc_) {
        cerr << "Failed to create off-screen DC" << endl;
        return nullptr;
    }

    // 创建位图
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = frame->width;
    bmi.bmiHeader.biHeight = -frame->height; // 负值表示从上到下的位图
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // 32位色
    bmi.bmiHeader.biCompression = BI_RGB;

    offscreen_bits_ = nullptr;
    offscreen_bitmap_ = CreateDIBSection(
        offscreen_dc_, &bmi, DIB_RGB_COLORS, (void**)&offscreen_bits_, nullptr, 0);

    if (!offscreen_bitmap_ || !offscreen_bits_) {
        cerr << "Failed to create DIB section" << endl;
        DeleteDC(offscreen_dc_);
        offscreen_dc_ = nullptr;
        return nullptr;
    }

    // 将位图选入DC
    SelectObject(offscreen_dc_, offscreen_bitmap_);

    // 将帧数据复制到离屏表面
    if (frame->format == AV_PIX_FMT_BGRA) {
        // 逐行复制，处理可能的行填充
        for (int y = 0; y < frame->height; y++) {
            memcpy(offscreen_bits_ + y * frame->width * 4,
                frame->data[0] + y * frame->linesize[0],
                frame->width * 4);
        }
    }
    else {
        // 转换为BGRA格式
        AVFrame* bgra_frame = ConvertToRGB(frame);
        if (bgra_frame) {
            // 逐行复制，处理可能的行填充
            for (int y = 0; y < bgra_frame->height; y++) {
                memcpy(offscreen_bits_ + y * bgra_frame->width * 4,
                    bgra_frame->data[0] + y * bgra_frame->linesize[0],
                    bgra_frame->width * 4);
            }
        }
    }

    return offscreen_dc_;
}

void XPlayer::CleanupConversion()
{
    if (sws_ctx_) {
        sws_freeContext(sws_ctx_);
        sws_ctx_ = nullptr;
    }

    if (rgb_frame_) {
        av_frame_free(&rgb_frame_);
        rgb_frame_ = nullptr;
    }

    // 清理离屏表面资源
    if (offscreen_bitmap_) {
        DeleteObject(offscreen_bitmap_);
        offscreen_bitmap_ = nullptr;
    }

    if (offscreen_dc_) {
        DeleteDC(offscreen_dc_);
        offscreen_dc_ = nullptr;
    }
    offscreen_bits_ = nullptr;
}

//设置视频播放位置，毫秒
bool XPlayer::Seek(long long ms)
{
    demux_.Seek(ms);
    audio_decode_.Clear();
    video_decode_.Clear();
    XAudioPlay::Instance()->Clear();
    return true;
}
void XPlayer::Stop()
{
    XThread::Stop();
    demux_.Stop();
    audio_decode_.Stop();
    video_decode_.Stop();
    Wait();
    demux_.Wait();
    audio_decode_.Wait();
    video_decode_.Wait();
    if (view_)
    {
        view_->Close();
        delete view_;
        view_ = nullptr;
    }
    XAudioPlay::Instance()->Close();
    CleanupConversion();
}
bool XPlayer::Open(const char* url, void* winid)
{
    // 添加格式提示 - 这是关键修改
    if (strstr(url, ".flv") || strstr(url, "type=flv")) {
        demux_.set_format_hint("flv");
    }

    // 解封装
    if (!demux_.Open(url))
        return false;

    // 视频参数
    auto vp = demux_.CopyVideoPara();
    if (vp && vp->para && vp->para->width > 0 && vp->para->height > 0 && vp->para->format != -1)
    {
        // 视频总时长
        this->total_ms_ = vp->total_ms;

        if (!video_decode_.Open(vp->para))
        {
            return false;
        }

        // 设置视频流索引
        video_decode_.set_stream_index(demux_.video_index());

        // 设置阻塞大小
        video_decode_.set_block_size(100);

        // 视频显示
        if (!view_)
            view_ = XVideoView::Create();
        view_->set_win_id(winid);

        // 根据是否启用RGB绘图选择初始化方式
        if (use_rgb_drawing_) {
            // 使用BGRA格式初始化视图（与转换格式匹配）
            if (!view_->Init(vp->para->width, vp->para->height, XVideoView::BGRA))
            {
                cerr << "Failed to initialize video view with BGRA format" << endl;
                return false;
            }
        }
        else {
            // 使用原始格式初始化视图
            if (!view_->Init(vp->para))
            {
                cerr << "Failed to initialize video view with original format" << endl;
                return false;
            }
        }
    }
    else
    {
        cerr << "Invalid video parameters!" << endl;
        return false;
    }

    // 音频参数
    auto ap = demux_.CopyAudioPara();
    if (ap)
    {
        // 音频解码
        if (!audio_decode_.Open(ap->para))
        {
            return false;
        }

        // 设置阻塞大小
        audio_decode_.set_block_size(100);

        // 设置音频流索引
        audio_decode_.set_stream_index(demux_.audio_index());

        // 帧缓存
        audio_decode_.set_frame_cache(true);

        // 初始化音频播放
        XAudioPlay::Instance()->Open(*ap);
    }
    else
    {
        demux_.set_syn_type(XSYN_VIDEO); // 设置视频同步
    }

    // 设置责任链
    demux_.set_next(this);
    return true;
}

void XPlayer::SetView(XVideoView* view) {
    if (view_) {
        delete view_;
    }
    view_ = view;
}

bool XPlayer::RegisterDrawCallback(fDrawCBFun DrawCBFun, void* pUserData) {
    if (!DrawCBFun) return false;
    EnableRGBDrawing(true);
    draw_callback_ = DrawCBFun;
    draw_user_data_ = pUserData;
    return true;
}

void XPlayer::Do(AVPacket* pkt)
{
    if(audio_decode_.is_open())
        audio_decode_.Do(pkt);
    if(video_decode_.is_open())
        video_decode_.Do(pkt);
}
void XPlayer::Start()
{
    demux_.Start();
    if(video_decode_.is_open())
        video_decode_.Start();
    if (audio_decode_.is_open())
        audio_decode_.Start();
    XThread::Start();
}

//渲染视频 播放音频
void XPlayer::Update()
{
    // 渲染视频
    if (view_)
    {
        auto f = video_decode_.GetFrame();
        if (f)
        {
            AVFrame* render_frame = f; // 默认使用原始帧

            // 处理画图回调
            if (draw_callback_) {
                // 创建离屏表面DC
                HDC offscreen_dc = CreateOffscreenDC(f);
                if (offscreen_dc) {
                    // 调用用户回调函数
                    draw_callback_(port_, offscreen_dc, draw_user_data_);

                    // 获取修改后的位图数据
                    if (offscreen_bits_) {
                        // 将修改后的数据复制回帧
                        if (f->format == AV_PIX_FMT_BGRA) {
                            // 逐行复制，处理可能的行填充
                            for (int y = 0; y < f->height; y++) {
                                memcpy(f->data[0] + y * f->linesize[0],
                                    offscreen_bits_ + y * f->width * 4,
                                    f->width * 4);
                            }
                        }
                        else if (rgb_frame_) {
                            // 逐行复制，处理可能的行填充
                            for (int y = 0; y < rgb_frame_->height; y++) {
                                memcpy(rgb_frame_->data[0] + y * rgb_frame_->linesize[0],
                                    offscreen_bits_ + y * rgb_frame_->width * 4,
                                    rgb_frame_->width * 4);
                            }
                        }
                    }
                }
            }

            // 渲染视频帧
            if (use_rgb_drawing_) {
                // 如果还没有RGB帧，进行转换
                if (!rgb_frame_ || rgb_frame_->width != f->width || rgb_frame_->height != f->height) {
                    render_frame = ConvertToRGB(f);
                }
                else {
                    // 已经处理过RGB帧，直接使用
                    render_frame = rgb_frame_;
                }

                view_->DrawFrame(render_frame);
            }
            else {
                view_->DrawFrame(render_frame);
            }

            XFreeFrame(&f);
        }
    }

    // 播放音频
    auto au = XAudioPlay::Instance();
    auto f = audio_decode_.GetFrame();
    if (!f) return;
    au->Push(f);
    XFreeFrame(&f);
}

void XPlayer::SetSpeed(float speed) {
    play_speed_ = max(0.5f, min(2.0f, speed));
    if (XAudioPlay::Instance()) {
        XAudioPlay::Instance()->SetSpeed(play_speed_);
    }
}
void XPlayer::Main()
{
    long long syn = 0;
    auto au = XAudioPlay::Instance();
    auto ap = demux_.CopyAudioPara();
    auto vp = demux_.CopyVideoPara();
    video_decode_.set_time_base(vp->time_base);
    while (!is_exit_)
    {
        if (is_pause())
        {
            MSleep(1);
            continue;
        }
        this->pos_ms_ = video_decode_.cur_ms();
        if (ap)
        {
            syn = XRescale(au->cur_pts(), ap->time_base, vp->time_base);
            audio_decode_.set_syn_pts(au->cur_pts() + 10000);
            video_decode_.set_syn_pts(syn);
        }
        MSleep(1);
    }
}
