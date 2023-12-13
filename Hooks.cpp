#include "Hooks.h"
#include "Hacks.h"
#include "Menu.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "IPrediction.h"
#include "Materials.h"
#include <math.h>
#include <ctime>
#include <intrin.h>
#include "backtracking.h"
#include "EdgyLagComp.h"
#include "igameventmanager.h"
#include <Windows.h>
#include "endme.h"
#pragma comment(lib, "Winmm.lib")
#define M_PI       3.14159265358979323846   // pi


bool thirdpersonBool = false;
Vector LastAngleAA;
int Globals::Shots;
bool Globals::change;
CUserCmd* Globals::UserCmd;
int Globals::TargetID;
IClientEntity* Globals::Target;
std::map<int, QAngle>Globals::storedshit;
int Globals::missedshots;

// Funtion Typedefs
typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);

using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);
typedef float(__stdcall *oGetViewModelFOV)();


// Function Pointers to the originals
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
FireEventClientSideFn oFireEventClientSide;
RenderViewFn oRenderView;

// Hook function prototypes
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event);
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(/*void* self, int edx,*/ float frametime, CUserCmd* pCmd);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);

// VMT Managers
namespace Hooks
{
	// VMT Managers
	Utilities::Memory::VMTManager VMTPanel; // Hooking drawing functions
	Utilities::Memory::VMTManager VMTClient; // Maybe CreateMove
	Utilities::Memory::VMTManager VMTClientMode; // CreateMove for functionality
	Utilities::Memory::VMTManager VMTModelRender; // DrawModelEx for chams
	Utilities::Memory::VMTManager VMTPrediction; // InPrediction for no vis recoil
	Utilities::Memory::VMTManager VMTPlaySound; // Autoaccept 
	Utilities::Memory::VMTManager VMTRenderView;
};

DamageEventListener* DamageListener;

// Undo our hooks
void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
}


// Initialise all our hooks
void Hooks::Initialise()
{
	// Panel hooks for drawing to the screen via surface functions
	VMTPanel.Initialise((DWORD*)Interfaces::Panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);
	//Utilities::Log("Paint Traverse Hooked");

	// No Visual Recoi	l
	VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);
	Utilities::Log("InPrediction Hooked");

	// Chams
	VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);
	Utilities::Log("DrawModelExecute Hooked");

	// Setup ClientMode Hooks
	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);

	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);

	// Setup client hooks
	VMTClient.Initialise((DWORD*)Interfaces::Client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 36);

}

bool __stdcall CreateMoveClient_Hooked(/*void* self, int edx,*/ float frametime, CUserCmd* pCmd)
{
	if (!pCmd->command_number)
		return true;


	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;


		// Backup for safety
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);

		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++) {
			IClientEntity* pBaseEntity = Interfaces::EntList->GetClientEntity(i);
			if (pBaseEntity && !pBaseEntity->IsDormant() && pBaseEntity != hackManager.pLocal()) globalsh.OldSimulationTime[i] = pBaseEntity->GetSimulationTime();
		}

		// Do da hacks
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal && pLocal->IsAlive())
			Hacks::MoveHacks(pCmd, bSendPacket);
		backtracking->legitBackTrack(pCmd, pLocal);

		//Movement Fix
		//GameUtils::CL_FixMove(pCmd, origView);
		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f); // if pCmd->viewangles.x > 89, set pCmd->viewangles.x instead of 0.0f on first
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);

		// Original shit for movement correction
		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (forward > 450) forward = 450;
		if (right > 450) right = 450;
		if (up > 450) up = 450;
		if (forward < -450) forward = -450;
		if (right < -450) right = -450;
		if (up < -450) up = -450;
		pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
		pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
		pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);

		// Angle normalisation
		if (Menu::Window.MiscTab.OtherSafeMode.GetState())
		{
			GameUtils::NormaliseViewAngle(pCmd->viewangles);

			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}

			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log("Having to re-normalise!");
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
				Beep(750, 800); // Why does it do this
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}

		if (pCmd->viewangles.x > 90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

		if (pCmd->viewangles.x < -90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}

	}

	return false;
}




Settings legit_settings;
// Paint Traverse Hooked function
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{


	if (Menu::Window.VisualsTab.RemoveScope.GetState() && !strcmp("HudZoom", Interfaces::Panels->GetName(vguiPanel)))
		return;

	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);

	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;



	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
		if (strstr(szPanelName, "MatSystemTopPanel"))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{


		switch (legit_settings.backtrackingticks)
		{
		case 0:
			Menu::Window.LegitBotTab.TickModulation.SetValue(1.f);
			break;
		case 1:
			Menu::Window.LegitBotTab.TickModulation.SetValue(2.f);
			break;
		case 2:
			Menu::Window.LegitBotTab.TickModulation.SetValue(3.f);
			break;
		case 3:
			Menu::Window.LegitBotTab.TickModulation.SetValue(4.f);
			break;
		case 4:
			Menu::Window.LegitBotTab.TickModulation.SetValue(5.f);
			break;
		case 5:
			Menu::Window.LegitBotTab.TickModulation.SetValue(6.f);
			break;
		case 6:
			Menu::Window.LegitBotTab.TickModulation.SetValue(7.f);
			break;
		case 7:
			Menu::Window.LegitBotTab.TickModulation.SetValue(8.f);
			break;
		case 8:
			Menu::Window.LegitBotTab.TickModulation.SetValue(9.f);
			break;
		case 9:
			Menu::Window.LegitBotTab.TickModulation.SetValue(10.f);
			break;
		case 10:
			Menu::Window.LegitBotTab.TickModulation.SetValue(11.f);
			break;
		case 11:
			Menu::Window.LegitBotTab.TickModulation.SetValue(12.f);
			break;
		case 12:
			Menu::Window.LegitBotTab.TickModulation.SetValue(13.f);
			break;
		}

		static bool menu;
		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			menu = !menu;
		}

		if (menu)
		{

			//just welcome message e.g. Hello, Statik
#define UNLEN 256
			char buffer[UNLEN + 1];
			DWORD size;
			size = sizeof(buffer);
			GetUserName(buffer, &size);
			char title[UNLEN];
			char ch1[25] = "Hello, ";
			char *ch = strcat(ch1, buffer);

			//eternal 

			Render::Text(25, 7, Color(255, 255, 0, 255), Render::Fonts::ESP, ch);
			Render::Text(25, 30, Color(255, 255, 255, 255), Render::Fonts::ESP, "Bunnyhop:");
			Render::Text(25, 45, Color(255, 255, 255, 255), Render::Fonts::ESP, "Backtrack:");
			Render::Text(25, 60, Color(255, 255, 255, 255), Render::Fonts::ESP, "Ticks:");
			Render::Text(25, 75, Color(255, 255, 255, 255), Render::Fonts::ESP, "ESP:");
			Render::Text(25, 90, Color(255, 255, 255, 255), Render::Fonts::ESP, "Radar:");
			Render::Text(25, 105, Color(255, 255, 255, 255), Render::Fonts::ESP, "Crosshair:");
			Render::Text(25, 120, Color(255, 255, 255, 255), Render::Fonts::ESP, "Rank Reveal:");
			Render::Text(25, 135, Color(255, 255, 255, 255), Render::Fonts::ESP, "No Flash:");

			if (Menu::Window.MiscTab.OtherAutoJump.GetState())
				Render::Text(125, 30, Color(0, 102, 255), Render::Fonts::ESP, XorStr("ON"));
			else
				Render::Text(125, 30, Color(255, 255, 255), Render::Fonts::ESP, XorStr("OFF"));

			if (Menu::Window.LegitBotTab.AimbotBacktrack.GetState())
				Render::Text(125, 45, Color(0, 102, 255), Render::Fonts::ESP, XorStr("ON"));
			else
				Render::Text(125, 45, Color(255, 255, 255), Render::Fonts::ESP, XorStr("OFF"));

			if (true)
			{
				Render::Text(125, 60, Color(255, 255, 255, 255), Render::Fonts::ESP, XorStr(""));

				if (GetAsyncKeyState(VK_F3) & 1)
					legit_settings.backtrackingticks += 1;

				if (legit_settings.backtrackingticks > 12)
					legit_settings.backtrackingticks = 0;

				if (legit_settings.backtrackingticks < 0)
					legit_settings.backtrackingticks = 12;

				switch (legit_settings.backtrackingticks)
				{
				case 0:
					Render::Text(125, 60, Color(255, 255, 255, 255), Render::Fonts::ESP, XorStr("1"));
					break;
				case 1:
					Render::Text(125, 60, Color(255, 255, 255, 255), Render::Fonts::ESP, XorStr("2"));
					break;
				case 2:
					Render::Text(125, 60, Color(255, 255, 255, 255), Render::Fonts::ESP, XorStr("3"));
					break;
				case 3:
					Render::Text(125, 60, Color(255, 255, 255, 255), Render::Fonts::ESP, XorStr("4"));
					break;
				case 4:
					Render::Text(125, 60, Color(255, 255, 0, 255), Render::Fonts::ESP, XorStr("5"));
					break;
				case 5:
					Render::Text(125, 60, Color(255, 255, 0, 255), Render::Fonts::ESP, XorStr("6"));
					break;
				case 6:
					Render::Text(125, 60, Color(255, 255, 0, 255), Render::Fonts::ESP, XorStr("7"));
					break;
				case 7:
					Render::Text(125, 60, Color(255, 255, 0, 255), Render::Fonts::ESP, XorStr("8"));
					break;
				case 8:
					Render::Text(125, 60, Color(255, 255, 0, 255), Render::Fonts::ESP, XorStr("9"));
					break;
				case 9:
					Render::Text(125, 60, Color(255, 0, 0, 255), Render::Fonts::ESP, XorStr("10"));
					break;
				case 10:
					Render::Text(125, 60, Color(255, 0, 0, 255), Render::Fonts::ESP, XorStr("11"));
					break;
				case 11:
					Render::Text(125, 60, Color(255, 0, 0, 255), Render::Fonts::ESP, XorStr("12"));
					break;
				case 12:
					Render::Text(125, 60, Color(255, 0, 0, 255), Render::Fonts::ESP, XorStr("13"));
					break;
				}
			}

			if (Menu::Window.VisualsTab.OptionsBox.GetState())
				Render::Text(125, 75, Color(0, 102, 255), Render::Fonts::ESP, XorStr("ON"));
			else
				Render::Text(125, 75, Color(255, 255, 255), Render::Fonts::ESP, XorStr("OFF"));

			if (Menu::Window.VisualsTab.OtherRadar.GetState())
				Render::Text(125, 90, Color(0, 102, 255), Render::Fonts::ESP, XorStr("ON"));
			else
				Render::Text(125, 90, Color(255, 255, 255), Render::Fonts::ESP, XorStr("OFF"));

			if (Menu::Window.VisualsTab.OtherCrosshair.GetState())
				Render::Text(125, 105, Color(0, 102, 255), Render::Fonts::ESP, XorStr("ON"));
			else
				Render::Text(125, 105, Color(255, 255, 255), Render::Fonts::ESP, XorStr("OFF"));

			if (Menu::Window.VisualsTab.OptionsCompRank.GetState())
				Render::Text(125, 120, Color(0, 102, 255), Render::Fonts::ESP, XorStr("ON"));
			else
				Render::Text(125, 120, Color(255, 255, 255), Render::Fonts::ESP, XorStr("OFF"));

			if (Menu::Window.VisualsTab.OtherNoFlash.GetState())
				Render::Text(125, 135, Color(0, 102, 255), Render::Fonts::ESP, XorStr("ON"));
			else
				Render::Text(125, 135, Color(255, 255, 255), Render::Fonts::ESP, XorStr("OFF"));
		}

		else if (!menu)
		{

		}


		static float mememe6;
		int crosskey = VK_F6;
		if (crosskey >= 0 && GUI.GetKeyState(crosskey) && abs(mememe6 - Interfaces::Globals->curtime) > 1)
		{
			if (Menu::Window.VisualsTab.OtherCrosshair.GetState() == false)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("crosshair 0");
				Menu::Window.VisualsTab.Active.SetState(true);
				Menu::Window.VisualsTab.OtherCrosshair.SetState(true);
				mememe6 = Interfaces::Globals->curtime;
			}
			else if (Menu::Window.VisualsTab.OtherCrosshair.GetState() == true)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("crosshair 1");
				Menu::Window.VisualsTab.Active.SetState(false);
				Menu::Window.VisualsTab.OtherCrosshair.SetState(false);
				mememe6 = Interfaces::Globals->curtime;
			}
		}


		static float mememe1;
		int bhoptogglekey = VK_F1;
		if (bhoptogglekey >= 0 && GUI.GetKeyState(bhoptogglekey) && abs(mememe1 - Interfaces::Globals->curtime) > 1)
		{
			if (Menu::Window.MiscTab.OtherAutoJump.GetState() == false)
			{
				Menu::Window.MiscTab.OtherAutoJump.SetState(true);
				mememe1 = Interfaces::Globals->curtime;
			}
			else if (Menu::Window.MiscTab.OtherAutoJump.GetState() == true)
			{
				Menu::Window.MiscTab.OtherAutoJump.SetState(false);
				mememe1 = Interfaces::Globals->curtime;
			}
		}

		static float mememe2;
		int backtrackkey = VK_F2;
		if (backtrackkey >= 0 && GUI.GetKeyState(backtrackkey) && abs(mememe2 - Interfaces::Globals->curtime) > 1)
		{
			if (Menu::Window.LegitBotTab.AimbotBacktrack.GetState() == false)
			{
				Menu::Window.VisualsTab.Active.SetState(true);
				Menu::Window.LegitBotTab.AimbotBacktrack.SetState(true);
				Menu::Window.VisualsTab.BacktrackingLol.SetState(true);
				mememe2 = Interfaces::Globals->curtime;
			}
			else if (Menu::Window.LegitBotTab.AimbotBacktrack.GetState() == true)
			{
				Menu::Window.VisualsTab.Active.SetState(false);
				Menu::Window.LegitBotTab.AimbotBacktrack.SetState(false);
				Menu::Window.VisualsTab.BacktrackingLol.SetState(false);
				mememe2 = Interfaces::Globals->curtime;
			}
		}

		static float mememe4;
		int espboxkey = VK_F4;
		if (espboxkey >= 0 && GUI.GetKeyState(espboxkey) && abs(mememe4 - Interfaces::Globals->curtime) > 1)
		{
			if (Menu::Window.VisualsTab.OptionsBox.GetState() == false)
			{
				Menu::Window.VisualsTab.Active.SetState(true);
				Menu::Window.VisualsTab.OptionsBox.SetState(true);
				Menu::Window.VisualsTab.OptionsName.SetState(true);
				Menu::Window.VisualsTab.GrenadeTracer.SetState(true);
				Menu::Window.VisualsTab.OptionsSkeleton.SetState(true);
				Menu::Window.VisualsTab.FiltersPlayers.SetState(true);
				Menu::Window.VisualsTab.FiltersEnemiesOnly.SetState(true);
				mememe4 = Interfaces::Globals->curtime;
			}
			else if (Menu::Window.VisualsTab.OptionsBox.GetState() == true)
			{
				Menu::Window.VisualsTab.Active.SetState(false);
				Menu::Window.VisualsTab.OptionsBox.SetState(false);
				Menu::Window.VisualsTab.OptionsName.SetState(false);
				Menu::Window.VisualsTab.GrenadeTracer.SetState(false);
				Menu::Window.VisualsTab.OptionsSkeleton.SetState(false);
				Menu::Window.VisualsTab.FiltersPlayers.SetState(false);
				Menu::Window.VisualsTab.FiltersEnemiesOnly.SetState(false);
				mememe4 = Interfaces::Globals->curtime;
			}
		}

		static float mememe5;
		int radarkey = VK_F5;
		if (radarkey >= 0 && GUI.GetKeyState(radarkey) && abs(mememe5 - Interfaces::Globals->curtime) > 1)
		{
			if (Menu::Window.VisualsTab.OtherRadar.GetState() == false)
			{
				Menu::Window.VisualsTab.Active.SetState(true);
				Menu::Window.VisualsTab.OtherRadar.SetState(true);
				mememe5 = Interfaces::Globals->curtime;
			}
			else if (Menu::Window.VisualsTab.OtherRadar.GetState() == true)
			{
				Menu::Window.VisualsTab.Active.SetState(false);
				Menu::Window.VisualsTab.OtherRadar.SetState(false);
				mememe5 = Interfaces::Globals->curtime;
			}
		}

		static float mememe7;
		int rankkey = VK_F7;
		if (rankkey >= 0 && GUI.GetKeyState(rankkey) && abs(mememe7 - Interfaces::Globals->curtime) > 1)
		{
			if (Menu::Window.VisualsTab.OptionsCompRank.GetState() == false)
			{
				Menu::Window.VisualsTab.Active.SetState(true);
				Menu::Window.VisualsTab.OptionsCompRank.SetState(true);
				mememe7 = Interfaces::Globals->curtime;
			}
			else if (Menu::Window.VisualsTab.OptionsCompRank.GetState() == true)
			{
				Menu::Window.VisualsTab.Active.SetState(false);
				Menu::Window.VisualsTab.OptionsCompRank.SetState(false);
				mememe7 = Interfaces::Globals->curtime;
			}
		}

		static float mememe8;
		int flashkey = VK_F8;
		if (flashkey >= 0 && GUI.GetKeyState(flashkey) && abs(mememe8 - Interfaces::Globals->curtime) > 1)
		{
			if (Menu::Window.VisualsTab.OtherNoFlash.GetState() == false)
			{
				Menu::Window.VisualsTab.Active.SetState(true);
				Menu::Window.VisualsTab.OtherNoFlash.SetState(true);
				mememe8 = Interfaces::Globals->curtime;
			}
			else if (Menu::Window.VisualsTab.OtherNoFlash.GetState() == true)
			{
				Menu::Window.VisualsTab.Active.SetState(false);
				Menu::Window.VisualsTab.OtherNoFlash.SetState(false);
				mememe8 = Interfaces::Globals->curtime;
			}
		}

		IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
		RECT scrn = Render::GetViewport();

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
			Hacks::DrawHacks();

		// Update and draw the menu
		Menu::DoUIFrame();
	}


}

// InPrediction Hooked Function
bool __stdcall Hooked_InPrediction()
{

	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = Interfaces::Prediction;
	result = origFunc(ecxVal);

	// If we are in the right place where the player view is calculated
	// Calculate the change in the view and get rid of it
	if (Menu::Window.VisualsTab.OtherNoVisualRecoil.GetState() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{
		IClientEntity* pLocalEntity = NULL;

		float* m_LocalViewAngles = NULL;

		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}

		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();

		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}

	return result;
}

// DrawModelExec for chams and shit
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	PVOID pebp;
	__asm mov pebp, ebp;
	bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
	bool& bSendPacket = *pbSendPacket;
	Color color;
	float flColor[3] = { 0.f };
	bool DontDraw = false;

	const char* ModelName = Interfaces::ModelInfo->GetModelName((model_t*)pInfo.pModel);
	IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Menu::Window.VisualsTab.Active.GetState())
	{
		if (pLocal->IsAlive())
		{


			// Player Chams
			int ChamsStyle = Menu::Window.VisualsTab.OptionsChams.GetIndex();
			int HandsStyle = Menu::Window.VisualsTab.OtherNoHands.GetIndex();
			if (ChamsStyle != 0 /*&& Menu::Window.VisualsTab.FiltersPlayers.GetState() */ && strstr(ModelName, "models/player"))
			{
				if (pLocal)
				{

					IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);
					if (pModelEntity)
					{
						IClientEntity *local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
						if (local)
						{

							if (!hackManager.pLocal()->IsAlive())
								return;

							if (pModelEntity->GetTeamNum() != local->GetTeamNum())
							{
								float alpha = 1.f;

								if (pModelEntity->HasGunGameImmunity())
									alpha = 0.5f;

								int red = 255;
								int green = 255;
								int blue = 255;

								flColor[0] = red / 255.f;
								flColor[1] = green / 255.f;
								flColor[2] = blue / 255.f;

								Interfaces::RenderView->SetColorModulation(flColor);
								Interfaces::RenderView->SetBlend(alpha);
								if (Menu::Window.VisualsTab.ChamsXQZ.GetState())

									oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

								Interfaces::RenderView->SetColorModulation(flColor);
								Interfaces::RenderView->SetBlend(alpha);
							}
							else
							{
								color.SetColor(255, 255, 255, 255);
							}

							/*if (bSendPacket)
							{
							bool LbyColor = false; // u can make LBY INDICATOR. When LbyColor is true. Color will be Green , if false it will be White
							float NormalColor[3] = { 1, 1, 1 };
							float lbyUpdateColor[3] = { 0, 1, 0 };
							Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
							Interfaces::ModelRender->ForcedMaterialOverride(open);
							pLocal->DrawModel(1);
							Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
							}*/
						}
					}
				}
			}
			else if (HandsStyle != 0 && strstr(ModelName, "arms"))
			{
				if (HandsStyle == 1)
				{
					DontDraw = true;
				}
				else if (HandsStyle == 2)
				{
					if (pLocal->IsAlive())
					{
						//color.SetColor(HandR, HandG, HandB);
						//IMaterial *GlassChams = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER);
						static float colors[3] = { 255.f, 20.f, 20.f };
						IMaterial *GlassHands = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/service_medal_2016/glass_lvl4", "Model textures");
						IMaterial* Hands = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						//Hands->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
						if (Menu::Window.VisualsTab.HandXQZ.GetState())
						{
							Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						}
						else
						{
							Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						}
						//Interfaces::RenderView->SetColorModulation(colors);
						//Interfaces::RenderView->SetBlend(0.3);
						Interfaces::ModelRender->ForcedMaterialOverride(GlassHands);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
					//ForceMaterial(color, GlassHands, false);
				}
				else if (HandsStyle == 3)
				{

					if (pLocal->IsAlive())
					{
						//color.SetColor(HandR, HandG, HandB);
						IMaterial *TestHands2 = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/trophy_majors/gold", "Model textures");
						IMaterial* Hands = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						//Hands->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
						if (Menu::Window.VisualsTab.HandXQZ.GetState())
						{
							Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						}
						else
						{
							Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						}
						Interfaces::ModelRender->ForcedMaterialOverride(TestHands2);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
				else if (HandsStyle == 4)
				{
					if (pLocal->IsAlive())
					{
						color.SetColor(255, 50, 0);
						//				static float colors[3] = { 40.f, 255.f, 65.f };
						IMaterial* Hands = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						Hands->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
						if (Menu::Window.VisualsTab.HandXQZ.GetState())
						{
							Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						}
						else
						{
							Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						}
						//	Interfaces::RenderView->SetColorModulation(colors);
						Interfaces::ModelRender->ForcedMaterialOverride(Hands);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}
				else if (HandsStyle == 5)
				{
					if (pLocal->IsAlive())
					{
						color.SetColor(255, 50, 0);
						IMaterial* Hands = Interfaces::MaterialSystem->FindMaterial(ModelName, "Model textures");
						Hands->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
						if (Menu::Window.VisualsTab.HandXQZ.GetState())
						{
							Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
						}
						else
						{
							Hands->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
						}
						Interfaces::ModelRender->ForcedMaterialOverride(Hands);
						oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
					}
				}

				else
				{
					static int counter = 0;
					static float colors[3] = { 1.f, 0.f, 0.f };

					if (colors[counter] >= 1.0f)
					{
						colors[counter] = 1.0f;
						counter += 1;
						if (counter > 2)
							counter = 0;
					}
					else
					{
						int prev = counter - 1;
						if (prev < 0) prev = 2;
						colors[prev] -= 0.05f;
						colors[counter] += 0.05f;
					}

					Interfaces::RenderView->SetColorModulation(colors);
					Interfaces::RenderView->SetBlend(0.3);
				}
			}
			else if (ChamsStyle != 0 && Menu::Window.VisualsTab.FiltersWeapons.GetState() && strstr(ModelName, "_dropped.mdl"))
			{
				color.SetColor(175, 175, 175, 255);
			}
		}
	}

	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces::ModelRender->ForcedMaterialOverride(NULL);
}


void NormalizedAngle(float& flAngle)
{
	if (std::isnan(flAngle)) flAngle = 0.0f;
	if (std::isinf(flAngle)) flAngle = 0.0f;

	float flRevolutions = flAngle / 360;

	if (flAngle > 180 || flAngle < -180)
	{
		if (flRevolutions < 0)
			flRevolutions = -flRevolutions;

		flRevolutions = round(flRevolutions);

		if (flAngle < 0)
			flAngle = (flAngle + 360 * flRevolutions);
		else
			flAngle = (flAngle - 360 * flRevolutions);
	}
}

static inline bool IsNearEqual(float v1, float v2, float Tolerance)
{
	return std::abs(v1 - v2) <= std::abs(Tolerance);
}

// Hooked FrameStageNotify for removing visual recoil
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	CCacheAngle OptionsManager;

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	}
	oFrameStageNotify(curStage);
}

bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event)
{

	return oFireEventClientSide(ECX, Event);
}

void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		if (Menu::Window.VisualsTab.Active.GetState() && pLocal->IsAlive() && !pLocal->IsScoped())
		{
			if (pSetup->fov = 90)
				pSetup->fov = Menu::Window.VisualsTab.OtherFOV.GetValue();
		}
		oOverrideView(ecx, edx, pSetup);
	}

}

void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		if (!localplayer)
			return;


		if (Menu::Window.VisualsTab.Active.GetState())
			fov += Menu::Window.VisualsTab.OtherViewmodelFOV.GetValue();
	}
}

float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)();

	GetViewModelFOV(fov);

	return fov;
}

void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);

	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
}