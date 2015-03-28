#pragma once
class GameTimer
{
private:
	GameTimer(void);
	~GameTimer(void);
public:
	DECLARE_INSTANCE(GameTimer);
	UINT_PTR TimerId;
	
	void Attach();
	void Detach();

	static VOID CALLBACK Timer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
};

