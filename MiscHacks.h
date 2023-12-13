/*
Syn's AyyWare Framework 2015
*/

#pragma once

#include "Hacks.h"
Vector GetAutostrafeView();
class CMiscHacks : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool &bSendPacket);
private:
	void AutoJump(CUserCmd *pCmd);

};



