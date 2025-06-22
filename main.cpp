#include <iostream>
#include "xplay.h"
#include "xtools.h"
#include <Windows.h>
#include <chrono>
#include <thread>
using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// �Զ�����ƻص�����
void MyDrawCallback(LONG nPort, HDC hdc, void* pUserData)
{
    HFONT hFont = CreateFont(
        36,                    
        0,                     
        0,                     
        0,                     
        FW_BOLD,               
        FALSE,                 
        FALSE,                
        FALSE,                 
        DEFAULT_CHARSET,       
        OUT_DEFAULT_PRECIS,    
        CLIP_DEFAULT_PRECIS,   
        DEFAULT_QUALITY,       
        DEFAULT_PITCH | FF_DONTCARE, 
        L"SimHei"              
    );

    // ���浱ǰ����
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    // ���ñ���ģʽΪ͸��
    SetBkMode(hdc, TRANSPARENT);
    // �����ı���ɫΪ��ɫ
    SetTextColor(hdc, RGB(255, 0, 0));

    wstring text = L"HZPlayer";
    RECT rect = { 20, 20, 300, 100 }; // �����ı���ʾ����λ�úʹ�С����
    DrawText(hdc, text.c_str(), -1, &rect, DT_LEFT | DT_TOP);

    // �ָ�ԭʼ���岢ɾ����ʱ����
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

int main(int argc, char* argv[])
{
	//��д��Ƶ�ļ�·����http��rtsp�������ļ��ȣ�
    const char* videoFile = "v1080.mp4";

    // ����������ʵ��
    LONG port = PLAY_CreatePlayer();
    if (port <= 0) {
        cerr << "Failed to create player instance!" << endl;
        return 1;
    }
    cout << "Created player on port: " << port << endl;

    // ע����ƻص�����
    if (!PLAY_RegisterDrawFun(port, MyDrawCallback, nullptr)) {
        cerr << "Failed to register draw callback!" << endl;
        PLAY_DestroyPlayer(port);
        return 1;
    }

    // ����һ��800x600�Ĵ���
    HWND hwnd = CreateWindow(L"STATIC", L"Video Player", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, nullptr, nullptr);

    if (!hwnd) {
        cerr << "Failed to create window!" << endl;
        PLAY_DestroyPlayer(port);
        return 1;
    }

    // ����Ƶ�ļ�
    if (!PLAY_Open(port, videoFile, (void*)hwnd)) {
        cerr << "Failed to open video file: " << videoFile << endl;
        PLAY_DestroyPlayer(port);
        DestroyWindow(hwnd);
        return 1;
    }

    // ��ʼ����
    if (!PLAY_Start(port)) {
        cerr << "Failed to start playback!" << endl;
        PLAY_DestroyPlayer(port);
        DestroyWindow(hwnd);
        return 1;
    }

    cout << "Player started. Playing " << videoFile << "..." << endl;
    cout << "Duration: " << PLAY_GetDuration(port) << " ms" << endl;
    
    // ��ѭ��
    MSG msg;
    while (true) {

        // ���²�����״̬
        PLAY_Update(port);

        // ����Ƿ��˳�
        if (PLAY_IsExit(port)) {
            break;
        }

        MSleep(10);
    }

    // �ͷ���Դ
    PLAY_Stop(port);
    PLAY_DestroyPlayer(port);
    DestroyWindow(hwnd);

    cout << "\nPlayback finished." << endl;
    return 0;
}