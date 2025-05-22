#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameSessionManager GSessionManager;

void GameSessionManager::Add(shared_ptr<GameSession> session)
{
	scoped_lock(lock_);
	Sessions_.insert(session);
}

void GameSessionManager::Remove(shared_ptr<GameSession> session)
{
	scoped_lock(lock_);
	Sessions_.erase(session);
}

void GameSessionManager::Broadcast(shared_ptr<SendBuf> sendBuffer)
{
	scoped_lock(lock_);
	for (auto gamesession : Sessions_)
	{
		gamesession->Send(sendBuffer);
	}
}
