#pragma once
#include "xplayer.h"
// ����ص���������
typedef void (*fDrawCBFun)(LONG nPort, HDC hdc, void* pUserData);

extern "C" {
    // ����������ʵ��
    LONG PLAY_CreatePlayer();

    // ���ٲ�����ʵ��
    BOOL PLAY_DestroyPlayer(LONG nPort);

    // ��ý���ļ�
    BOOL PLAY_Open(LONG nPort, const char* url, void* winid);

    // ��ʼ����
    BOOL PLAY_Start(LONG nPort);

    // ֹͣ����
    BOOL PLAY_Stop(LONG nPort);

    // ע���ͼ�ص�
    BOOL PLAY_RegisterDrawFun(LONG nPort, fDrawCBFun DrawCBFun, void* pUserData);

    // ������Ƶ֡
    BOOL PLAY_Update(LONG nPort);

    // ����Ƿ��˳�
    BOOL PLAY_IsExit(LONG nPort);

    // ���ò���λ�� (����)
    BOOL PLAY_Seek(LONG nPort, long long ms);

    // ��ͣ/��������
    BOOL PLAY_Pause(LONG nPort, BOOL is_pause);

    // ��ȡ��ʱ�� (����)
    long long PLAY_GetDuration(LONG nPort);

    // ��ȡ��ǰ����λ�� (����)
    long long PLAY_GetPosition(LONG nPort);

    // �������� (0-100)
    BOOL PLAY_SetVolume(LONG nPort, int volume);

    // �������� (����1-10)
    BOOL PLAY_VolumeUp(LONG nPort, int step = 5);

    // ��С���� (����1-10)
    BOOL PLAY_VolumeDown(LONG nPort, int step = 5);

    // ���ò����ٶ� (0.5-2.0)
    BOOL PLAY_SetSpeed(LONG nPort, float speed);

    // ��ȡ��ǰ���� (0-100)
    int PLAY_GetVolume(LONG nPort);

    // ��ȡ��ǰ�����ٶ�
    float PLAY_GetSpeed(LONG nPort);

    //��ȡ��ǰ�汾��
    float PLAY_GetCurrVersion();
}