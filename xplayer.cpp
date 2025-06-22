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

//��ͣ���߲���
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
    // �������ͼ��ʹ����ͼ���˳�״̬
    if (view_) {
        return view_->IsExit();
    }

    // û����ͼʱ��ʹ�ò�����������˳���־
    return is_exit_;
}

void XPlayer::EnableRGBDrawing(bool enable) {
    use_rgb_drawing_ = enable;
}

// ����Ƶ֡ת��ΪRGB��ʽ
AVFrame* XPlayer::ConvertToRGB(AVFrame* frame)
{
    if (!frame || !frame->data[0]) return nullptr;

    // ����Ѿ���RGB��ʽ��ֱ�ӷ���
    if (frame->format == AV_PIX_FMT_RGBA ||
        frame->format == AV_PIX_FMT_BGRA ||
        frame->format == AV_PIX_FMT_ARGB)
    {
        return frame;
    }

    // ��ʼ��ת��������
    if (!sws_ctx_) {
        sws_ctx_ = sws_getContext(
            frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, AV_PIX_FMT_BGRA, // ʹ��BGRA��ʽ����Windows
            SWS_BILINEAR, nullptr, nullptr, nullptr);

        if (!sws_ctx_) {
            cerr << "Failed to create SwsContext" << endl;
            return frame;
        }
    }

    // ����RGB֡
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

    // ִ��ת��
    sws_scale(sws_ctx_,
        frame->data, frame->linesize,
        0, frame->height,
        rgb_frame_->data, rgb_frame_->linesize);

    return rgb_frame_;
}

HDC XPlayer::CreateOffscreenDC(AVFrame* frame)
{
    if (!frame || !frame->data[0]) return nullptr;

    // ����֮ǰ����Դ
    if (offscreen_dc_) {
        if (offscreen_bitmap_) {
            DeleteObject(offscreen_bitmap_);
            offscreen_bitmap_ = nullptr;
        }
        DeleteDC(offscreen_dc_);
        offscreen_dc_ = nullptr;
    }

    // �����ڴ�DC
    HDC screen_dc = GetDC(nullptr);
    offscreen_dc_ = CreateCompatibleDC(screen_dc);
    ReleaseDC(nullptr, screen_dc);

    if (!offscreen_dc_) {
        cerr << "Failed to create off-screen DC" << endl;
        return nullptr;
    }

    // ����λͼ
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = frame->width;
    bmi.bmiHeader.biHeight = -frame->height; // ��ֵ��ʾ���ϵ��µ�λͼ
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32; // 32λɫ
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

    // ��λͼѡ��DC
    SelectObject(offscreen_dc_, offscreen_bitmap_);

    // ��֡���ݸ��Ƶ���������
    if (frame->format == AV_PIX_FMT_BGRA) {
        // ���и��ƣ�������ܵ������
        for (int y = 0; y < frame->height; y++) {
            memcpy(offscreen_bits_ + y * frame->width * 4,
                frame->data[0] + y * frame->linesize[0],
                frame->width * 4);
        }
    }
    else {
        // ת��ΪBGRA��ʽ
        AVFrame* bgra_frame = ConvertToRGB(frame);
        if (bgra_frame) {
            // ���и��ƣ�������ܵ������
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

    // ��������������Դ
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

//������Ƶ����λ�ã�����
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
    // ��Ӹ�ʽ��ʾ - ���ǹؼ��޸�
    if (strstr(url, ".flv") || strstr(url, "type=flv")) {
        demux_.set_format_hint("flv");
    }

    // ���װ
    if (!demux_.Open(url))
        return false;

    // ��Ƶ����
    auto vp = demux_.CopyVideoPara();
    if (vp && vp->para && vp->para->width > 0 && vp->para->height > 0 && vp->para->format != -1)
    {
        // ��Ƶ��ʱ��
        this->total_ms_ = vp->total_ms;

        if (!video_decode_.Open(vp->para))
        {
            return false;
        }

        // ������Ƶ������
        video_decode_.set_stream_index(demux_.video_index());

        // ����������С
        video_decode_.set_block_size(100);

        // ��Ƶ��ʾ
        if (!view_)
            view_ = XVideoView::Create();
        view_->set_win_id(winid);

        // �����Ƿ�����RGB��ͼѡ���ʼ����ʽ
        if (use_rgb_drawing_) {
            // ʹ��BGRA��ʽ��ʼ����ͼ����ת����ʽƥ�䣩
            if (!view_->Init(vp->para->width, vp->para->height, XVideoView::BGRA))
            {
                cerr << "Failed to initialize video view with BGRA format" << endl;
                return false;
            }
        }
        else {
            // ʹ��ԭʼ��ʽ��ʼ����ͼ
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

    // ��Ƶ����
    auto ap = demux_.CopyAudioPara();
    if (ap)
    {
        // ��Ƶ����
        if (!audio_decode_.Open(ap->para))
        {
            return false;
        }

        // ����������С
        audio_decode_.set_block_size(100);

        // ������Ƶ������
        audio_decode_.set_stream_index(demux_.audio_index());

        // ֡����
        audio_decode_.set_frame_cache(true);

        // ��ʼ����Ƶ����
        XAudioPlay::Instance()->Open(*ap);
    }
    else
    {
        demux_.set_syn_type(XSYN_VIDEO); // ������Ƶͬ��
    }

    // ����������
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

//��Ⱦ��Ƶ ������Ƶ
void XPlayer::Update()
{
    // ��Ⱦ��Ƶ
    if (view_)
    {
        auto f = video_decode_.GetFrame();
        if (f)
        {
            AVFrame* render_frame = f; // Ĭ��ʹ��ԭʼ֡

            // ����ͼ�ص�
            if (draw_callback_) {
                // ������������DC
                HDC offscreen_dc = CreateOffscreenDC(f);
                if (offscreen_dc) {
                    // �����û��ص�����
                    draw_callback_(port_, offscreen_dc, draw_user_data_);

                    // ��ȡ�޸ĺ��λͼ����
                    if (offscreen_bits_) {
                        // ���޸ĺ�����ݸ��ƻ�֡
                        if (f->format == AV_PIX_FMT_BGRA) {
                            // ���и��ƣ�������ܵ������
                            for (int y = 0; y < f->height; y++) {
                                memcpy(f->data[0] + y * f->linesize[0],
                                    offscreen_bits_ + y * f->width * 4,
                                    f->width * 4);
                            }
                        }
                        else if (rgb_frame_) {
                            // ���и��ƣ�������ܵ������
                            for (int y = 0; y < rgb_frame_->height; y++) {
                                memcpy(rgb_frame_->data[0] + y * rgb_frame_->linesize[0],
                                    offscreen_bits_ + y * rgb_frame_->width * 4,
                                    rgb_frame_->width * 4);
                            }
                        }
                    }
                }
            }

            // ��Ⱦ��Ƶ֡
            if (use_rgb_drawing_) {
                // �����û��RGB֡������ת��
                if (!rgb_frame_ || rgb_frame_->width != f->width || rgb_frame_->height != f->height) {
                    render_frame = ConvertToRGB(f);
                }
                else {
                    // �Ѿ������RGB֡��ֱ��ʹ��
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

    // ������Ƶ
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
