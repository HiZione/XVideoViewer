#include <iostream>
#include "xplay.h"
#include "xtools.h"
#include <Windows.h>
#include <chrono>
#include <thread>
using namespace std;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// 自定义绘制回调函数
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

    // 保存当前字体
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    // 设置背景模式为透明
    SetBkMode(hdc, TRANSPARENT);
    // 设置文本颜色为红色
    SetTextColor(hdc, RGB(255, 0, 0));

    wstring text = L"HZPlayer";
    RECT rect = { 20, 20, 300, 100 }; // 定义文本显示区域，位置和大小调整
    DrawText(hdc, text.c_str(), -1, &rect, DT_LEFT | DT_TOP);

    // 恢复原始字体并删除临时字体
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

int main(int argc, char* argv[])
{
	//填写视频文件路径（http、rtsp、本地文件等）
    const char* videoFile = "v1080.mp4";

    // 创建播放器实例
    LONG port = PLAY_CreatePlayer();
    if (port <= 0) {
        cerr << "Failed to create player instance!" << endl;
        return 1;
    }
    cout << "Created player on port: " << port << endl;

    // 注册绘制回调函数
    if (!PLAY_RegisterDrawFun(port, MyDrawCallback, nullptr)) {
        cerr << "Failed to register draw callback!" << endl;
        PLAY_DestroyPlayer(port);
        return 1;
    }

    // 创建一个800x600的窗口
    HWND hwnd = CreateWindow(L"STATIC", L"Video Player", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, nullptr, nullptr);

    if (!hwnd) {
        cerr << "Failed to create window!" << endl;
        PLAY_DestroyPlayer(port);
        return 1;
    }

    // 打开视频文件
    if (!PLAY_Open(port, videoFile, (void*)hwnd)) {
        cerr << "Failed to open video file: " << videoFile << endl;
        PLAY_DestroyPlayer(port);
        DestroyWindow(hwnd);
        return 1;
    }

    // 开始播放
    if (!PLAY_Start(port)) {
        cerr << "Failed to start playback!" << endl;
        PLAY_DestroyPlayer(port);
        DestroyWindow(hwnd);
        return 1;
    }

    cout << "Player started. Playing " << videoFile << "..." << endl;
    cout << "Duration: " << PLAY_GetDuration(port) << " ms" << endl;
    
    // 主循环
    MSG msg;
    while (true) {

        // 更新播放器状态
        PLAY_Update(port);

        // 检查是否退出
        if (PLAY_IsExit(port)) {
            break;
        }

        MSleep(10);
    }

    // 释放资源
    PLAY_Stop(port);
    PLAY_DestroyPlayer(port);
    DestroyWindow(hwnd);

    cout << "\nPlayback finished." << endl;
    return 0;
}