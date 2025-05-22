#pragma once

class GameSession;

class GameSessionManager
{
public:
	void Add(shared_ptr<GameSession> session);
	void Remove(shared_ptr<GameSession> session);
	void Broadcast(shared_ptr<SendBuf> sendBuffer);

private:
	mutex lock_;
	set<shared_ptr<GameSession>> Sessions_;
};

extern GameSessionManager GSessionManager;