#pragma once


#include <thread>
#include <iostream>
#include <mutex>
#include <list>
struct AVPacket;
struct AVCodecParameters;
struct AVRational;
struct AVFrame;
struct AVCodecContext;




//��־���� DEBUG INFO ERROR FATAL
enum XLogLevel
{
    XLOG_TYPE_DEBUG,
    XLOG_TYPE_INFO,
    XLOG_TPYE_ERROR,
    XLOG_TYPE_FATAL
};
#define LOG_MIN_LEVEL XLOG_TYPE_DEBUG
#define XLOG(s,level) \
    if(level>=LOG_MIN_LEVEL) \
    std::cout<<level<<":"<<__FILE__<<":"<<__LINE__<<":\n"\
    <<s<<std::endl;
#define LOGDEBUG(s) XLOG(s,XLOG_TYPE_DEBUG)
#define LOGINFO(s) XLOG(s,XLOG_TYPE_INFO)
#define LOGERROR(s) XLOG(s,XLOG_TPYE_ERROR)
#define LOGFATAL(s) XLOG(s,XLOG_TYPE_FATAL)

void MSleep(unsigned int ms);

//��ȡ��ǰʱ��� ����
long long NowMs();

void XFreeFrame(AVFrame** frame);

void PrintErr(int err);

//����ʱ���������
long long XRescale(long long pts,
    AVRational* src_time_base,
    AVRational* des_time_base);
class XThread
{
public:
    //�����߳�
    virtual void Start();

    //ֹͣ�̣߳������˳���־��
    virtual void Stop();

    //�ȴ��߳��˳�
    virtual void Wait();

    //ִ������ ��Ҫ����
    virtual void Do(AVPacket* pkt) {}

    //���ݵ���һ������������
    virtual void Next(AVPacket* pkt)
    {
        std::unique_lock<std::mutex> lock(m_);
        if (next_)
            next_->Do(pkt);

    }

    //������������һ���ڵ㣨�̰߳�ȫ��
    void set_next(XThread* xt)
    {
        std::unique_lock<std::mutex> lock(m_);
        next_ = xt;
    }
    //��ͣ���߲���
    virtual void Pause(bool is_pause) { is_pause_ = is_pause; };

    bool is_pause() { return is_pause_; }

protected:

    bool is_pause_ = false;

    //�߳���ں���
    virtual void Main() = 0;

    //��־�߳��˳�
    bool is_exit_ = false;
    
    //�߳�������
    int index_ = 0;


private:
    std::thread th_;
    std::mutex m_;
    XThread *next_ = nullptr;//��������һ���ڵ�

};
class XTools
{
};



//����Ƶ����
class XPara
{
public:
    AVCodecParameters* para = nullptr;  //����Ƶ����
    AVRational* time_base = nullptr;    //ʱ�����
    long long total_ms = 0;             //��ʱ�� ����

    //��������
    static XPara* Create();
    ~XPara();
private:
    //˽���ǽ�ֹ����ջ�ж���
    XPara();
};


/// <summary>
/// �̰߳�ȫavpacket list
/// </summary>
class XAVPacketList
{
public:
    AVPacket* Pop();
    void Push(AVPacket* pkt);
    int Size();
    void Clear();
private:
    std::list<AVPacket*> pkts_;

    int max_packets_ = 1000;//����б���������������
    std::mutex mux_;
};