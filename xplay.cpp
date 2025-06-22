#include "xplay.h"
#include "xplayer_manager.h"
#include <Windows.h>

// 创建播放器实例
LONG PLAY_CreatePlayer()
{
    static LONG nextPort = 0;
    XPlayer* player = new XPlayer();
    LONG port = InterlockedIncrement(&nextPort);

    if (!XPlayerManager::AddPlayer(port, player)) {
        delete player;
        return -1;
    }
    return port;
}

// 销毁播放器实例
BOOL PLAY_DestroyPlayer(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    player->Stop();
    delete player;
    return XPlayerManager::RemovePlayer(nPort);
}

// 打开媒体文件
BOOL PLAY_Open(LONG nPort, const char* url, void* winid)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    return player->Open(url, winid);
}

// 开始播放
BOOL PLAY_Start(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    player->Start();
    return TRUE;
}

// 停止播放
BOOL PLAY_Stop(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    player->Stop();
    return TRUE;
}

// 注册绘图回调
BOOL PLAY_RegisterDrawFun(LONG nPort, fDrawCBFun DrawCBFun, void* pUserData)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    return player->RegisterDrawCallback(DrawCBFun, pUserData);
}

// 更新视频帧
BOOL PLAY_Update(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    player->Update();
    return TRUE;
}

// 检查是否退出
BOOL PLAY_IsExit(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    return player ? player->ShouldExit() : TRUE;
}

// 设置播放位置 (毫秒)
BOOL PLAY_Seek(LONG nPort, long long ms)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    return player->Seek(ms);
}

// 暂停/继续播放
BOOL PLAY_Pause(LONG nPort, BOOL is_pause)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    player->Pause(is_pause != FALSE);
    return TRUE;
}

// 获取总时长 (毫秒)
long long PLAY_GetDuration(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    return player ? player->total_ms() : 0;
}

// 获取当前播放位置 (毫秒)
long long PLAY_GetPosition(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    return player ? player->pos_ms() : 0;
}

BOOL PLAY_SetVolume(LONG nPort, int volume)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    // 限制音量范围在0-100之间
    volume = max(0, min(100, volume));
    player->SetVolume(volume);
    return TRUE;
}

BOOL PLAY_VolumeUp(LONG nPort, int step)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    // 限制步长在1-10之间
    step = max(1, min(10, step));
    int newVolume = min(100, player->GetVolume() + step);
    player->SetVolume(newVolume);
    return TRUE;
}

BOOL PLAY_VolumeDown(LONG nPort, int step)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    // 限制步长在1-10之间
    step = max(1, min(10, step));
    int newVolume = max(0, player->GetVolume() - step);
    player->SetVolume(newVolume);
    return TRUE;
}

BOOL PLAY_SetSpeed(LONG nPort, float speed)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    // 限制速度在0.5-2.0之间
    speed = max(0.5f, min(2.0f, speed));
    player->SetSpeed(speed);
    return TRUE;
}

int PLAY_GetVolume(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return -1;
    return player->GetVolume();
}

float PLAY_GetSpeed(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return -1.0f;
    return player->GetSpeed();
}

float PLAY_GetCurrVersion()
{
    return 3.0f;
}
