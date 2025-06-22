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
    // ���廭ͼ�ص���������
    typedef void (*fDrawCBFun)(LONG nPort, HDC hdc, void* pUserData);

    // ������Ƶ��ͼ
    void SetView(XVideoView* view);

    LONG GetPort() const { return port_; }
    
    // ע�ửͼ�ص�����
    bool RegisterDrawCallback(fDrawCBFun DrawCBFun, void* pUserData);

    // �ص���������Ƶ��
    void Do(AVPacket* pkt) override;

    // ������Ƶ ��ʼ�����ź���Ⱦ
    bool Open(const char* url, void* winid);

    void Stop();

    // ���߳� ����ͬ��
    void Main() override;

    // ���� ���װ ����Ƶ���� �� ����ͬ�����߳�
    void Start();

    // ��Ⱦ��Ƶ ������Ƶ
    void Update();

    void SetSpeed(float s);

    // ��ʱ�� ����
    long long total_ms() { return total_ms_; }

    // ��ǰ���ŵ�λ�� ����
    long long pos_ms() { return pos_ms_; }

    // ������Ƶ����λ�ã�����
    bool Seek(long long ms);

    void Pause(bool is_pause) override;

    // ��鲥�����Ƿ�Ӧ���˳�
    bool ShouldExit();

    // �������� (0-100)
    void SetVolume(int volume) {
        audio_volume_ = max(0, min(100, volume));
        if (XAudioPlay::Instance()) {
            XAudioPlay::Instance()->set_volume(audio_volume_);
        }
    }

    // ��ȡ��ǰ����
    int GetVolume() const {
        return audio_volume_;
    }

    // ��ȡ��ǰ�����ٶ�
    float GetSpeed() const {
        return play_speed_;
    }

protected:
    // ����/����RGB��ͼ�ص�
    void EnableRGBDrawing(bool enable);

    //�˿ں�
    LONG port_ = -1;

    // ����Ƶ֡ת��ΪRGB��ʽ
    AVFrame* ConvertToRGB(AVFrame* frame);

    // ������������DC
    HDC CreateOffscreenDC(AVFrame* frame);

    // ����ת����Դ
    void CleanupConversion();
    bool is_exit_ = false;
    long long total_ms_ = 0;
    long long pos_ms_ = 0;
    int audio_volume_ = 80;      // Ĭ������80
    float play_speed_ = 1.0f;    // Ĭ���ٶ�1.0
    XDemuxTask demux_;              // ���װ
    XDecodeTask audio_decode_;      // ��Ƶ����
    XDecodeTask video_decode_;      // ��Ƶ����
    XVideoView* view_ = nullptr;    // ��Ƶ��Ⱦ

    bool use_rgb_drawing_ = false;  // �Ƿ�����RGB��ͼ�ص�
    struct SwsContext* sws_ctx_ = nullptr; // ��ʽת��������
    AVFrame* rgb_frame_ = nullptr;    // RGB֡������

    // ��ͼ�ص���س�Ա
    fDrawCBFun draw_callback_ = nullptr;
    void* draw_user_data_ = nullptr;
    HDC offscreen_dc_ = nullptr;     // ��������DC
    HBITMAP offscreen_bitmap_ = nullptr; // ����λͼ
    unsigned char* offscreen_bits_ = nullptr; // ����λͼ����ָ��
};