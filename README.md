# 音视频播放器SDK #
### 内容介绍 ###
##### 概述 #####
本 SDK 主要用于视频播放，提供了视频文件的打开、播放、暂停、恢复播放、倍速播放以及跳转等功能；支持 RTSP、HTTP、本地文件、预览以及转录；支持用户绘图回调，水印、文字、图片绘制等，以下是对 SDK 中各个类和主要接口的详细说明。
##### 特别说明 #####
本SDL在x64平台开发，提供ffmpeg、sdl2 头文件及其库文件，提供main.cpp测试文件！
### 使用说明 ###
包含xplay.h文件即可调用以下接口！！！
### API接口 ###

##### 创建播放器实例 #####
``` cpp
LONG PLAY_CreatePlayer();
```
##### 销毁播放器实例 #####
``` cpp
BOOL PLAY_DestroyPlayer(LONG nPort);
```
##### 打开媒体文件 #####
``` cpp
BOOL PLAY_Open(LONG nPort, const char* url, void* winid);
```
##### 开始播放 #####
```cpp
BOOL PLAY_Start(LONG nPort);
```
##### 停止播放 #####
```cpp
BOOL PLAY_Stop(LONG nPort);
```
##### 注册绘图回调 #####
```cpp
BOOL PLAY_RegisterDrawFun(LONG nPort, fDrawCBFun DrawCBFun, void* pUserData);
```
##### 更新视频帧 #####
```cpp
BOOL PLAY_Update(LONG nPort);
```
##### 检查是否退出 ######
```cpp
BOOL PLAY_IsExit(LONG nPort);
```
##### 设置播放位置 (毫秒) #####
```cpp
BOOL PLAY_Seek(LONG nPort, long long ms);
```
##### 暂停/继续播放 #####
```cpp
BOOL PLAY_Pause(LONG nPort, BOOL is_pause);
```
##### 获取总时长 (毫秒) #####
```cpp
long long PLAY_GetDuration(LONG nPort);
```
##### 获取当前播放位置 (毫秒) #####
```cpp
long long PLAY_GetPosition(LONG nPort);
```
##### 设置音量 (0-100) #####
```cpp
BOOL PLAY_SetVolume(LONG nPort, int volume);
```
##### 增加音量 (步长1-10)#####
```cpp
BOOL PLAY_VolumeUp(LONG nPort, int step = 5);
```
##### 减小音量 (步长1-10) #####
```cpp
BOOL PLAY_VolumeDown(LONG nPort, int step = 5);
```
##### 设置播放速度 (0.5-2.0) #####
```cpp
BOOL PLAY_SetSpeed(LONG nPort, float speed);
```
##### 获取当前音量 (0-100) #####
```cpp
int PLAY_GetVolume(LONG nPort);
```
##### 获取当前播放速度 #####
```cpp
float PLAY_GetSpeed(LONG nPort);
```
##### 获取当前版本号 #####
```cpp
float PLAY_GetCurrVersion();
```
