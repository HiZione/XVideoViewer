#pragma once
#include <map>
#include <mutex>
#include <windows.h>

class XPlayer;

class XPlayerManager
{
public:
    static XPlayer* GetPlayer(LONG nPort);
    static bool AddPlayer(LONG nPort, XPlayer* player);
    static bool RemovePlayer(LONG nPort);

private:
    static std::map<LONG, XPlayer*> players_;
    static std::mutex mutex_;
};