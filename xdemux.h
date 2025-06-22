#pragma once
#include "xformat.h"
class XDemux :public XFormat
{
public:
    /// <summary>
    /// �򿪽��װ
    /// </summary>
    /// <param name="url">���װ��ַ ֧��rtsp</param>
    /// <returns>ʧ�ܷ���nullptr</returns>
    static AVFormatContext* Open(const char* url);

    /// <summary>
    /// ��ȡһ֡����
    /// </summary>
    /// <param name="pkt">�������</param>
    /// <returns>�Ƿ�ɹ�</returns>
    bool Read(AVPacket* pkt);

    bool Seek(long long pts,int stream_index);

    void set_format_hint(const char* hint) {
        format_hint_ = hint ? hint : "";
    }

private:
    std::string format_hint_; // ��ʽ��ʾ
};

