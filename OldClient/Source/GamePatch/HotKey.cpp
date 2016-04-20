#include "StdAfx.h"
#include "HotKey.h"
#include "math.h"

CHotKey HotKey;

CHotKey::CHotKey(void):m_init(false),events_count(0),events(0)
{
	memset(&press_stats,0,sizeof(press_stats));
}

CHotKey::~CHotKey(void)
{
}
bool CHotKey::IsPress(int vKey)
{
	return press_stats[vKey] < 0;
}

int CHotKey::ControlState()
{
	int state = 0;

	if(IsPress(VK_CONTROL)){
		state |= VCTL_CTRL;
	}
	if(IsPress(VK_MENU)){
		state |= VCTL_ALT;
	}
	if(IsPress(VK_SHIFT)){
		state |= VCTL_SHIFT;
	}
	if(IsPress(VK_LWIN) || IsPress(VK_RWIN)){
		state |= VCTL_WIN;
	}

	return state;
}
void CHotKey::Update()
{
	for(int i = 0x01; i < 0xFF; i++){
		press_stats[i] = GetAsyncKeyState(i);
	}

	for(int i = 0; i < events_count;i++)
	{
		int vkey = abs(events[i].vKey);
		int press = press_stats[vkey];

		if(press == 0)
		{
			if(events[i].state == HOTKEY_STATE_DOWN){
				events[i].state = HOTKEY_STATE_UP;
			} else {
				events[i].state = HOTKEY_STATE_NONE;
			}
		} else if(press < 0) {
			if(events[i].state == HOTKEY_STATE_NONE){
				events[i].state = HOTKEY_STATE_DOWN;
			}
			if(events[i].vKey < 0) continue;
		}
	

		if(events[i].state > 0 && events[i].state != HOTKEY_STATE_STOP)
		{
			events[i].state = HOTKEY_STATE_STOP;
			if(events[i].vControl != ControlState()){
				continue;
			}

			events[i].cb();
		}
	}
}
VOID CALLBACK CHotKey::TimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime)
{
	HotKey.Update();
}
void CHotKey::AddMonitor(int vKey, fnHookCallback cb,int vControl)
{
	hotkey_state_t *state;
	if(events == NULL)
	{
		state = new hotkey_state_t();

		state->vKey = vKey;
		state->cb = cb;
		state->vControl = vControl;
		events = state;

		events_count = 1;
	}
	else
	{
		state = new hotkey_state_t[events_count + 1];
		memcpy(state,events,sizeof(hotkey_state_t) * events_count);
		state[events_count].vKey = vKey;
		state[events_count].cb = cb;
		state[events_count].vControl = vControl;
		events_count++;
		delete[] events;
		events = state;
	}

}
void CHotKey::DelMon(int vKey,int vControl)
{
	int found_count = 0;
	if(events == NULL) return;
	
	hotkey_state_t *state;
	for(int i=0;i<events_count;i++)
	{
		if(events[i].vKey == vKey && events[i].vControl == vControl)
		{
			found_count++;
		}
	}

	int index = 0;

	state = new hotkey_state_t[events_count - found_count];
	for(int i=0;i<events_count;i++)
	{
		if(!(events[i].vKey == vKey && events[i].vControl == vControl))
		{
			state[index++] = events[i];
		}
	}

	delete[] events;
	events_count = events_count - found_count;
	events = state;
}