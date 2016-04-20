#pragma once

typedef void (*fnHookCallback)();

#define VCTL_CTRL 0x1
#define VCTL_ALT 0x2
#define VCTL_SHIFT 0x4
#define VCTL_WIN 0x8


#define HOTKEY_STATE_NONE 0
#define HOTKEY_STATE_DOWN 1
#define HOTKEY_STATE_UP 2
#define HOTKEY_STATE_STOP -1

typedef struct hotkey_state_s
{
	int vKey;
	int vControl;
	int state;
	fnHookCallback cb;
	bool is_delete;
}hotkey_state_t;



class CHotKey
{
	int press_stats[256];
	int events_count;
	hotkey_state_t* events;
public:
	CHotKey(void);
	~CHotKey(void);
	bool m_init;

	void Update();
	int ControlState();
	bool IsPress(int vKey);

	//Ê±ÖÓ»Øµ÷
	static VOID CALLBACK TimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);
	void AddMonitor(int vKey, fnHookCallback cb, int vControl = HOTKEY_STATE_NONE);
	void DelMon(int vKey,int vControl=HOTKEY_STATE_NONE);
};


extern CHotKey HotKey;