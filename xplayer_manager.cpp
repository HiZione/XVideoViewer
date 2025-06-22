// xplayer_manager.cpp
#include "xplayer_manager.h"
#include "xplayer.h"

std::map<LONG, XPlayer*> XPlayerManager::players_;
std::mutex XPlayerManager::mutex_;

XPlayer* XPlayerManager::GetPlayer(LONG nPort)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = players_.find(nPort);
    if (it != players_.end()) {
        return it->second;
    }
    return nullptr;
}

bool XPlayerManager::AddPlayer(LONG nPort, XPlayer* player)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (players_.find(nPort) != players_.end()) {
        return false; // ¶Ë¿ÚÒÑ´æÔÚ
    }
    players_[nPort] = player;
    return true;
}

bool XPlayerManager::RemovePlayer(LONG nPort)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = players_.find(nPort);
    if (it != players_.end()) {
        players_.erase(it);
        return true;
    }
    return false;
}