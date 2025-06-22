#include "xdemux.h"
#include <iostream>
#include <thread>
#include "xtools.h"
using namespace std;
extern "C" {
#include <libavformat/avformat.h>
}
void PrintErr(int err);
#define BERR(err) if(err!= 0){PrintErr(err);return 0;}

AVFormatContext* XDemux::Open(const char* url)
{
    AVFormatContext* c = nullptr;
    AVDictionary* opts = nullptr;

    // ���FLV��ʽ���
    if (strstr(url, ".flv") || strstr(url, "type=flv")) {
        // ǿ��ʹ��FLV���װ��
        const AVInputFormat* iformat = av_find_input_format("flv");
        if (!iformat) {
            cerr << "FLV demuxer not found!" << endl;
            return nullptr;
        }

        // ���HTTP-FLV�ض�����
        av_dict_set(&opts, "flvtransport", "tcp", 0);  // ʹ��TCP����
        av_dict_set(&opts, "fflags", "nobuffer", 0);   // ���ٻ���
        av_dict_set(&opts, "analyzeduration", "100000", 0); // ���ͷ���ʱ��
        av_dict_set(&opts, "probesize", "100000", 0);  // ����̽���С

        auto re = avformat_open_input(&c, url, iformat, &opts);
        if (re != 0) {
            PrintErr(re);
            if (opts) av_dict_free(&opts);
            return nullptr;
        }
    }
    else {
        // Ĭ�ϲ�������
        av_dict_set(&opts, "analyzeduration", "20000000", 0);
        av_dict_set(&opts, "probesize", "20000000", 0);
        av_dict_set(&opts, "stimeout", "1000000", 0); // ��ʱʱ�� 1 ��

        auto re = avformat_open_input(&c, url, nullptr, &opts);
        if (re != 0) {
            PrintErr(re);
            if (opts) av_dict_free(&opts);
            return nullptr;
        }
    }

    if (opts)
        av_dict_free(&opts);

    // ��ȡ����Ϣ
    auto re = avformat_find_stream_info(c, nullptr);
    if (re < 0) {
        PrintErr(re);
        avformat_close_input(&c);
        return nullptr;
    }

    // ��ӡ��ʽ��Ϣ
    av_dump_format(c, 0, url, 0);
    return c;
}

bool XDemux::Read(AVPacket* pkt)
{
    unique_lock<mutex> lock(mux_);
    if (!c_)return false;
    auto re = av_read_frame(c_, pkt);
    if (re < 0) {
        PrintErr(re);
        return false;
    }
    // ��������������ʱ��
    last_time_ = NowMs();
    return true;
}

bool XDemux::Seek(long long pts, int stream_index)
{
    unique_lock<mutex> lock(mux_);
    if (!c_)return false;
    auto re = av_seek_frame(c_, stream_index, pts,
        AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    if (re < 0) {
        PrintErr(re);
        return false;
    }
    return true;
}