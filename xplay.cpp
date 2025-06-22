#include "xplay.h"
#include "xplayer_manager.h"
#include <Windows.h>

// ����������ʵ��
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

// ���ٲ�����ʵ��
BOOL PLAY_DestroyPlayer(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    player->Stop();
    delete player;
    return XPlayerManager::RemovePlayer(nPort);
}

// ��ý���ļ�
BOOL PLAY_Open(LONG nPort, const char* url, void* winid)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    return player->Open(url, winid);
}

// ��ʼ����
BOOL PLAY_Start(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    player->Start();
    return TRUE;
}

// ֹͣ����
BOOL PLAY_Stop(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    player->Stop();
    return TRUE;
}

// ע���ͼ�ص�
BOOL PLAY_RegisterDrawFun(LONG nPort, fDrawCBFun DrawCBFun, void* pUserData)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    return player->RegisterDrawCallback(DrawCBFun, pUserData);
}

// ������Ƶ֡
BOOL PLAY_Update(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    player->Update();
    return TRUE;
}

// ����Ƿ��˳�
BOOL PLAY_IsExit(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    return player ? player->ShouldExit() : TRUE;
}

// ���ò���λ�� (����)
BOOL PLAY_Seek(LONG nPort, long long ms)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    return player->Seek(ms);
}

// ��ͣ/��������
BOOL PLAY_Pause(LONG nPort, BOOL is_pause)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;
    player->Pause(is_pause != FALSE);
    return TRUE;
}

// ��ȡ��ʱ�� (����)
long long PLAY_GetDuration(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    return player ? player->total_ms() : 0;
}

// ��ȡ��ǰ����λ�� (����)
long long PLAY_GetPosition(LONG nPort)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    return player ? player->pos_ms() : 0;
}

BOOL PLAY_SetVolume(LONG nPort, int volume)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    // ����������Χ��0-100֮��
    volume = max(0, min(100, volume));
    player->SetVolume(volume);
    return TRUE;
}

BOOL PLAY_VolumeUp(LONG nPort, int step)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    // ���Ʋ�����1-10֮��
    step = max(1, min(10, step));
    int newVolume = min(100, player->GetVolume() + step);
    player->SetVolume(newVolume);
    return TRUE;
}

BOOL PLAY_VolumeDown(LONG nPort, int step)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    // ���Ʋ�����1-10֮��
    step = max(1, min(10, step));
    int newVolume = max(0, player->GetVolume() - step);
    player->SetVolume(newVolume);
    return TRUE;
}

BOOL PLAY_SetSpeed(LONG nPort, float speed)
{
    XPlayer* player = XPlayerManager::GetPlayer(nPort);
    if (!player) return FALSE;

    // �����ٶ���0.5-2.0֮��
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
