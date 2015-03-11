#pragma once
class PSHelper
{
public:
	PSHelper(void);
	~PSHelper(void);

	DECLARE_INSTANCE(PSHelper);

	DWORD GetMainThreadId();
};

