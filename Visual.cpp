/*
Syn's AyyWare Framework 2015
*/

#include "Visuals.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "MathFunctions.h"



// Any init here
void CVisuals::Init()
{
	// Idk
}

// Don't really need to do anything in here
void CVisuals::Move(CUserCmd *pCmd, bool &bSendPacket) 
{
	
}

// Main ESP Drawing loop
void CVisuals::Draw()
{
	
	// Crosshair
	if (Menu::Window.VisualsTab.OtherCrosshair.GetState())
		DrawCrosshair();


}

// Draw a basic crosshair
void CVisuals::DrawCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;
	Render::Line(MidX - 8, MidY - 0, MidX + 8, MidY + 0, Color(255, 0, 0, 200));
	Render::Line(MidX + 0, MidY - 8, MidX - 0, MidY + 8, Color(255, 0, 0, 200));
	Render::Line(MidX - 4, MidY - 0, MidX + 4, MidY + 0, Color(255, 255, 255, 255));
	Render::Line(MidX + 0, MidY - 4, MidX - 0, MidY + 4, Color(255, 255, 255, 255));
}



