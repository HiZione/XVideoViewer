#pragma once
#include "xplayer.h"
// 定义回调函数类型
typedef void (*fDrawCBFun)(LONG nPort, HDC hdc, void* pUserData);

extern "C" {
    // 创建播放器实例
    LONG PLAY_CreatePlayer();

    // 销毁播放器实例
    BOOL PLAY_DestroyPlayer(LONG nPort);

    // 打开媒体文件
    BOOL PLAY_Open(LONG nPort, const char* url, void* winid);

    // 开始播放
    BOOL PLAY_Start(LONG nPort);

    // 停止播放
    BOOL PLAY_Stop(LONG nPort);

    // 注册绘图回调
    BOOL PLAY_RegisterDrawFun(LONG nPort, fDrawCBFun DrawCBFun, void* pUserData);

    // 更新视频帧
    BOOL PLAY_Update(LONG nPort);

    // 检查是否退出
    BOOL PLAY_IsExit(LONG nPort);

    // 设置播放位置 (毫秒)
    BOOL PLAY_Seek(LONG nPort, long long ms);

    // 暂停/继续播放
    BOOL PLAY_Pause(LONG nPort, BOOL is_pause);

    // 获取总时长 (毫秒)
    long long PLAY_GetDuration(LONG nPort);

    // 获取当前播放位置 (毫秒)
    long long PLAY_GetPosition(LONG nPort);

    // 设置音量 (0-100)
    BOOL PLAY_SetVolume(LONG nPort, int volume);

    // 增加音量 (步长1-10)
    BOOL PLAY_VolumeUp(LONG nPort, int step = 5);

    // 减小音量 (步长1-10)
    BOOL PLAY_VolumeDown(LONG nPort, int step = 5);

    // 设置播放速度 (0.5-2.0)
    BOOL PLAY_SetSpeed(LONG nPort, float speed);

    // 获取当前音量 (0-100)
    int PLAY_GetVolume(LONG nPort);

    // 获取当前播放速度
    float PLAY_GetSpeed(LONG nPort);

    //获取当前版本号
    float PLAY_GetCurrVersion();
}