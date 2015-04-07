#pragma once
class ChatFixer
{
public:
	ChatFixer(void);
	~ChatFixer(void);

	DECLARE_INSTANCE(ChatFixer);
	void Attach();
	
	static void Proc();
};

