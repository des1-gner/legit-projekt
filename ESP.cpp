/*
Syn's AyyWare Framework 2015
*/

#include "ESP.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "GlowManager.h"
#include "EdgyLagComp.h"
#include "SDK.h"
#include "MiscClasses.h"

void CEsp::Init()
{
	BombCarrier = nullptr;
}

// Yeah dude we're defo gunna do some sick moves for the esp yeah
void CEsp::Move(CUserCmd *pCmd,bool &bSendPacket) 
{

}



void CEsp::BacktrackingCross(IClientEntity* pEntity)
{
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		player_info_t pinfo;
		if (pEntity && pEntity != hackManager.pLocal() && !pEntity->IsDormant())
		{
			if (Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{
				if (Menu::Window.LegitBotTab.AimbotBacktrack.GetState())
				{
					if (hackManager.pLocal()->IsAlive())
					{
						for (int t = 0; t < 12; ++t)
						{
							Vector screenbacktrack[64][12];

							if (headPositions[i][t].simtime && headPositions[i][t].simtime + 1 > hackManager.pLocal()->GetSimulationTime())
							{
								if (Render::WorldToScreen(headPositions[i][t].hitboxPos, screenbacktrack[i][t]))
								{

									Interfaces::Surface->DrawSetColor(Color(255, 0, 0, 255));
									Interfaces::Surface->DrawOutlinedRect(screenbacktrack[i][t].x, screenbacktrack[i][t].y, screenbacktrack[i][t].x + 2, screenbacktrack[i][t].y + 2);

								}
							}
						}
					}
					else
					{
						memset(&headPositions[0][0], 0, sizeof(headPositions));
					}
				}
			}
		}
	}
}

// Main ESP Drawing loop
void CEsp::Draw()
{
	IClientEntity *pLocal = hackManager.pLocal();



	// Loop through all active entitys
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		// Get the entity
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		player_info_t pinfo;

		// The entity isn't some laggy peice of shit or something
		if (pEntity &&  pEntity != pLocal && !pEntity->IsDormant())
		{
			// Radar
			if (Menu::Window.VisualsTab.OtherRadar.GetState())
			{
				DWORD m_bSpotted = NetVar.GetNetVar(0x839EB159);
				*(char*)((DWORD)(pEntity) + m_bSpotted) = 1;
			}

			// Is it a player?!
			if (/*Menu::Window.VisualsTab.FiltersPlayers.GetState() && */Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{
				DrawPlayer(pEntity, pinfo);
			}

			// ~ Other ESP's here (items and shit) ~ //
			ClientClass* cClass = (ClientClass*)pEntity->GetClientClass();

			// Dropped weapons
			if (Menu::Window.VisualsTab.FiltersWeapons.GetState() && cClass->m_ClassID != (int)CSGOClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)CSGOClassID::CDEagle || cClass->m_ClassID == (int)CSGOClassID::CAK47)))
			{
				DrawDrop(pEntity, cClass);
			}

			// If entity is the bomb
				if (cClass->m_ClassID == (int)CSGOClassID::CPlantedC4)
					DrawBombPlanted(pEntity, cClass);

				if (cClass->m_ClassID == (int)CSGOClassID::CC4)
					DrawBomb(pEntity, cClass);

			// If entity is a chicken
			if (Menu::Window.VisualsTab.FiltersChickens.GetState())
			{
				if (cClass->m_ClassID == (int)CSGOClassID::CChicken)
					DrawChicken(pEntity, cClass);
			}
			
			CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
			CUserCmd* pCmd = cmdlist;


			if (Menu::Window.VisualsTab.Grenades.GetState())
				DrawDrop(pEntity, cClass);

		}

	}


	// Anti Flash
	if (Menu::Window.VisualsTab.OtherNoFlash.GetState())
	{
		DWORD m_flFlashMaxAlpha = NetVar.GetNetVar(0xFE79FB98);
		*(float*)((DWORD)pLocal + m_flFlashMaxAlpha) = 0;
	}
	DWORD m_flFlashMaxAlpha = NetVar.GetNetVar(0xFE79FB98);
	if (!Menu::Window.VisualsTab.OtherNoFlash.GetState())
		*(float*)((DWORD)pLocal + m_flFlashMaxAlpha) = 255;
}


//  Yeah m8
void CEsp::DrawPlayer(IClientEntity* pEntity, player_info_t pinfo)
{
	ESPBox Box;
	Color Color;

	// Show own team false? well gtfo teammate lol
	if (Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() && (pEntity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()))
		return;


	if (GetBox(pEntity, Box))
	{
		Color = GetPlayerColor(pEntity);

		if (Menu::Window.VisualsTab.OptionsGlow.GetState())
		{
			int TeamNum = pEntity->GetTeamNum();

			if (TeamNum == TEAM_CS_T)
			{
				DrawGlow(pEntity, 255, 0, 0, 160);
			}
			else if (TeamNum == TEAM_CS_CT)
			{
				DrawGlow(pEntity, 0, 0, 255, 160);
			}
		}

		if (Menu::Window.VisualsTab.OptionsBox.GetState())
			DrawBox(Box, Color);



		if (Menu::Window.VisualsTab.OptionsName.GetState())
			DrawName(pinfo, Box);

		switch (Menu::Window.VisualsTab.OptionsVitals.GetIndex())
		{
		case 0: break;
		case 1: DrawPenis(pEntity, Box); break;
		case 2: DrawVitals(pEntity, Box); break;
		}



		if (Menu::Window.VisualsTab.OptionsInfo.GetState() || Menu::Window.VisualsTab.OptionsWeapon.GetState())
			DrawInfo(pEntity, Box);

		if (Menu::Window.VisualsTab.OptionsAimSpot.GetState())
			DrawCross(pEntity);

		if (Menu::Window.VisualsTab.OtherGlow.GetState())
			DrawGlow(Color);

		if (Menu::Window.VisualsTab.OptionsSkeleton.GetState())
			DrawSkeleton(pEntity);

		if (Menu::Window.VisualsTab.BacktrackingLol.GetState())
			BacktrackingCross(pEntity);
	}
}

// glow shit
struct Glowobject
{
	IClientEntity* pEntity;
	Vector Color;
	float Alpha;
	byte PAD[16];
	bool RenderWhenOccluded;
	bool RenderWhenUnOccluded;
	bool FullBloomRender;
	byte PAD2[17];
};

// simple sexy glow
void CEsp::DrawGlow(IClientEntity *pEntity, int r, int g, int b, int a)
{
	static uintptr_t Module = (uintptr_t)GetModuleHandle("client.dll");

	Glowobject* GlowManager = *(Glowobject**)(Utilities::Memory::FindPatternV2("client.dll", "0F 11 05 ?? ?? ?? ?? 83 C8 01 C7 05 ?? ?? ?? ?? 00 00 00 00") + 0x3);

	if (GlowManager)
	{
		Glowobject* GlowObject = &GlowManager[pEntity->GetGlowIndex()];

		if (GlowObject)
		{
			GlowObject->RenderWhenOccluded = 1;
			GlowObject->RenderWhenUnOccluded = 0;

			float glowr = (1 / 255.0f)*r;
			float glowg = (1 / 255.0f)*g;
			float glowb = (1 / 255.0f)*b;
			float glowa = (1 / 255.0f)*a;
			GlowObject->Color = Vector((1 / 255.0f)*r, (1 / 255.0f)*g, (1 / 255.0f)*b);
			GlowObject->Alpha = (1 / 255.0f)*a;
		}
	}
}

// Gets the 2D bounding box for the entity
// Returns false on failure nigga don't fail me
bool CEsp::GetBox(IClientEntity* pEntity, CEsp::ESPBox &result)
{
	// Variables
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;

	// Get the locations
	vOrigin = pEntity->GetOrigin();
	min = pEntity->collisionProperty()->GetMins() + vOrigin;
	max = pEntity->collisionProperty()->GetMaxs() + vOrigin;

	// Points of a 3d bounding box
	Vector points[] = 
	{
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) 
	};

	// Get screen positions
	if (!Render::WorldToScreen(points[3], flb) || !Render::WorldToScreen(points[5], brt)
		|| !Render::WorldToScreen(points[0], blb) || !Render::WorldToScreen(points[4], frt)
		|| !Render::WorldToScreen(points[2], frb) || !Render::WorldToScreen(points[1], brb)
		|| !Render::WorldToScreen(points[6], blt) || !Render::WorldToScreen(points[7], flt))
		return false;

	// Put them in an array (maybe start them off in one later for speed?)
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	// Init this shit
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	// Find the bounding corners for our box
	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	// Width / height
	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;

	return true;
}

// Get an entities color depending on team and vis ect
Color CEsp::GetPlayerColor(IClientEntity* pEntity)
{
	int TeamNum = pEntity->GetTeamNum();
	bool IsVis = GameUtils::IsVisible(hackManager.pLocal(), pEntity, (int)CSGOHitboxID::Head);

	Color color;

	if (TeamNum == TEAM_CS_T)
	{


		if (IsVis)
		{
			int red = 255;
			int green = 255;
			int blue = 255;
			int alpha = 255;
			
			color = Color(red, green, blue, alpha);
		}
		else
		{
			int red = 255;
			int green = 255;
			int blue = 255;
			int alpha = 255;

			color = Color(red, green, blue, alpha);
		}
	}
	else
	{

		if (IsVis)
		{
			int red = 255;
			int green = 255;
			int blue = 255;
			int alpha = 255;

			color = Color(red, green, blue, alpha);
		}
		else
		{
			int red = 255;
			int green = 255;
			int blue = 255;
			int alpha = 255;

			color = Color(red, green, blue, alpha);
		}
	}


	return color;
}

// 2D  Esp box
void CEsp::DrawBox(CEsp::ESPBox size, Color color)
{
	if (Menu::Window.VisualsTab.OptionsBox.GetState())
	{
		//2D box
		Render::Outline(size.x, size.y, size.w, size.h, Color(255, 255, 255, 255));//color);
		Render::Outline(size.x - 1, size.y - 1, size.w + 2, size.h + 2, Color(0, 110, 255, 255));
		Render::Outline(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 110, 255, 255));
	}
}





// Unicode Conversions
static wchar_t* CharToWideChar(const char* text)
{
	size_t size = strlen(text) + 1;
	wchar_t* wa = new wchar_t[size];
	mbstowcs_s(NULL, wa, size/4, text, size);
	return wa;
}

// Player name
void CEsp::DrawName(player_info_t pinfo, CEsp::ESPBox size)
{
	if (strlen(pinfo.name) > 16)
	{
		pinfo.name[12] = 0;
		strcat(pinfo.name, "...");
		RECT nameSize = Render::GetTextSize(Render::Fonts::MenuBold, pinfo.name);
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 16, Color(255, 255, 255, 255), Render::Fonts::MenuBold, pinfo.name);
	}
	else
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::MenuBold, pinfo.name);
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 16, Color(255, 255, 255, 255), Render::Fonts::MenuBold, pinfo.name);
	}

}

#define Surface Interfaces::Surface

void DrawOutlinedRect(int x, int y, int w, int h, Color col)
{
	Surface->DrawSetColor(col);
	Surface->DrawOutlinedRect(x, y, x + w, y + h);
}

void DrawOutlinedCircle(int x, int y, int r, Color col)
{
	Surface->DrawSetColor(col);
	Surface->DrawOutlinedCircle(x, y, r, 1);
}

void DrawLine(int x0, int y0, int x1, int y1, Color col)
{
	Surface->DrawSetColor(col);
	Surface->DrawLine(x0, y0, x1, y1);
}




// Draw a health bar. For Tf2 when a bar is bigger than max health a second bar is displayed
void CEsp::DrawPenis(IClientEntity* pEntity, CEsp::ESPBox size)
{

		float h = (size.h);
		float offset = (h / 4.f) + 5;
		float w = h / 64.f;
		float health = pEntity->GetHealth();
		UINT hp = h - (UINT)((h * health) / 100); // Percentage

		int Red = 255 - (health*2.55);
		int Green = health*2.55;

		DrawOutlinedRect((size.x - 6) - 1, size.y - 1, 3, h + 2, Color(0, 0, 0, 255));

		DrawLine((size.x - 6), size.y + hp, (size.x - 6), size.y + h, Color(Red, Green, 0, 255));
}

void CEsp::DrawVitals(IClientEntity* pEntity, CEsp::ESPBox size)
{
	ESPBox HealthBar = size;
	HealthBar.y += (HealthBar.h + 6);
	HealthBar.h = 4;

	float HealthValue = pEntity->GetHealth();
	float HealthPerc = HealthValue / 100.f;
	float flBoxes = std::ceil(HealthValue / 10.f);
	float Width = (size.w * HealthPerc);
	HealthBar.w = Width;
	float h = (size.h);
	float offset = (h / 4.f) + 5;
	float w = h / 64.f;
	float health = pEntity->GetHealth();
	float flMultiplier = 12 / 360.f; flMultiplier *= flBoxes - 1;
	Color ColHealth = Color::FromHSB(flMultiplier, 1, 1);

	UINT hp = h - (UINT)((h * health) / 100); // Percentage

	int Red = 255 - (health*2.55);
	int Green = health*2.55;

/*	Render::Outline((size.x - 6) + 1, size.y - 2, 3, h + 3, Color(0, 0, 0, 230));
	Render::Outline((size.x - 6) - 2, size.y - 2, 3, h + 3, Color(0, 0, 0, 230));
	Render::Outline((size.x - 6) - 1, size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
	Render::Outline((size.x - 6) - 1, size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
	Render::Line((size.x - 6), size.y + hp, (size.x - 6), size.y + h, Color(Red, Green, 0, 255));*/
DrawOutlinedRect((size.x - 6), size.y - 1, 3, h + 2, Color(0, 0, 0, 255));
DrawOutlinedRect(size.x - 5, size.y - 1, 1, (h / 10) * flBoxes + 1, ColHealth);
//DrawLine((size.x - 6), size.y + hp, (size.x - 6), size.y + h, Color(Red, Green, 0, 255));

for (int i = 0; i < 10; i++)
{
	DrawLine((size.x - 6), size.y + i * (h / 10) - 1, size.x - 3, size.y + i * (h / 10), Color::Black());
}
//Render::Text((size.x - 6) - 27, size.y + hp - 4, Color(255, 255, 255, 255), Render::Fonts::ESP, nameBuffer);
//for (int i = 0; i < 10; i++)
//	Render::Line(size.x, size.y + i * h, size.x + 6, size.y + i * h, Color(0, 0, 0));
}


// Cleans the internal class name up to something human readable and nice
std::string CleanItemName(std::string name)
{
	std::string Name = name;
	// Tidy up the weapon Name
	if (Name[0] == 'C')
		Name.erase(Name.begin());

	// Remove the word Weapon
	auto startOfWeap = Name.find("Weapon");
	if (startOfWeap != std::string::npos)
		Name.erase(Name.begin() + startOfWeap, Name.begin() + startOfWeap + 6);

	return Name;
}

// Anything else: weapons, class state? idk
void CEsp::DrawInfo(IClientEntity* pEntity, CEsp::ESPBox size)
{

}

// Little cross on their heads
void CEsp::DrawCross(IClientEntity* pEntity)
{
	Vector cross = pEntity->GetHeadPos(), screen;
	static int Scale = 2;
	if (Render::WorldToScreen(cross, screen))
	{
		Render::Clear(screen.x - Scale, screen.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen.x - (Scale * 2), screen.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen.x - Scale - 1, screen.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(250, 250, 250, 160));
		Render::Clear(screen.x - (Scale * 2) - 1, screen.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(250, 250, 250, 160));
	}
}

// Draws a dropped CS:GO Item
void CEsp::DrawDrop(IClientEntity* pEntity, ClientClass* cClass)
{
	Vector Box;
	CBaseCombatWeapon* Weapon = (CBaseCombatWeapon*)pEntity;
	IClientEntity* plr = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)Weapon->GetOwnerHandle());
	if (!plr && Render::WorldToScreen(Weapon->GetOrigin(), Box))
	{

		if (Menu::Window.VisualsTab.OptionsInfo.GetState())
		{
			std::string ItemName = CleanItemName(cClass->m_pNetworkName);
			RECT TextSize = Render::GetTextSize(Render::Fonts::ESP, ItemName.c_str());
			Render::Text(Box.x - (TextSize.right / 2), Box.y - 16, Color(255, 255, 255, 255), Render::Fonts::ESP, ItemName.c_str());
		}
	}
}

void CEsp::DrawGrenades(IClientEntity* pEntity)
{


	Vector vGrenadePos2D = Vector(0.f, 0.f, 0.f);
	Vector vGrenadePos3D = Vector(0.f, 0.f, 0.f);
	float fGrenadeModelSize = 0.0f;
	Color colGrenadeColor = Color(0, 0, 0, 0);
	const model_t *model = pEntity->GetModel();

	if (pEntity->GetClientClass())
	{
		if (model)
		{
			studiohdr_t* hdr = Interfaces::ModelInfo->GetStudiomodel(model);
			if (hdr)
			{
				std::string hdrName = hdr->name;
				if (hdrName.find("thrown") != std::string::npos)
				{
					vGrenadePos3D = pEntity->GetOrigin();
					fGrenadeModelSize = hdr->hull_max.DistTo(hdr->hull_min);

					if (!Render::WorldToScreen(vGrenadePos3D, vGrenadePos2D))
						return;

					if (hdrName.find("flash") != std::string::npos)
					{
						//Warning color
						colGrenadeColor = Color(0, 0, 200, 255);
					}
					else if (hdrName.find("incendiarygrenade") != std::string::npos || hdrName.find("molotov") != std::string::npos || hdrName.find("fraggrenade") != std::string::npos)
					{
						//Dangerous color
						colGrenadeColor = Color(200, 0, 0, 255);
					}

					else if (hdrName.find("smoke") != std::string::npos || hdrName.find("decoy") != std::string::npos)
					{
						//Peaceful color
						colGrenadeColor = Color(0, 200, 0, 255);
					}
					D::DrawOutlinedCircle(static_cast<int>(vGrenadePos2D.x), static_cast<int>(vGrenadePos2D.y), static_cast<int>(fGrenadeModelSize), colGrenadeColor);
				}
			}
		}
	}
}

// Draws a chicken
void CEsp::DrawChicken(IClientEntity* pEntity, ClientClass* cClass)
{
	ESPBox Box;

	if (GetBox(pEntity, Box))
	{
		player_info_t pinfo; strcpy_s(pinfo.name, "Chicken");
		//if (Menu::Window.VisualsTab.OptionsBox.GetState())
			DrawBox(Box, Color(255,255,255,255));

		if (Menu::Window.VisualsTab.OptionsName.GetState())
			DrawName(pinfo, Box);
	}
}

// Draw the planted bomb and timer
void CEsp::DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass)
{
	// Null it out incase bomb has been dropped or planted
	BombCarrier = nullptr;

	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)pEntity;

	if (Render::WorldToScreen(vOrig, vScreen))
	{
		float flBlow = Bomb->GetC4BlowTime();
		float TimeRemaining = flBlow - (Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase());
		char buffer[64];
		sprintf_s(buffer, "Bomb %.1f", TimeRemaining);
		Render::Text(vScreen.x, vScreen.y, Color(255, 255, 255, 255), Render::Fonts::ESP, buffer);
	}
}

// Draw the bomb if it's dropped, or store the player who's carrying 
void CEsp::DrawBomb(IClientEntity* pEntity, ClientClass* cClass)
{
	// Null it out incase bomb has been dropped or planted
	BombCarrier = nullptr;
	CBaseCombatWeapon *BombWeapon = (CBaseCombatWeapon *)pEntity;
	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	bool adopted = true;
	HANDLE parent = BombWeapon->GetOwnerHandle();
	if (parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		IClientEntity* pParentEnt = (Interfaces::EntList->GetClientEntityFromHandle(parent));
		if (pParentEnt && pParentEnt->IsAlive())
		{
			BombCarrier = pParentEnt;
			adopted = false;
		}
	}

	if (adopted)
	{
		if (Render::WorldToScreen(vOrig, vScreen))
		{
			Render::Text(vScreen.x, vScreen.y, Color(112, 230, 20, 255), Render::Fonts::ESP, "Bomb");
		}
	}
}

void DrawBoneArray(int* boneNumbers, int amount, IClientEntity* pEntity, Color color)
{
	Vector LastBoneScreen;
	for (int i = 0; i < amount; i++)
	{
		Vector Bone = pEntity->GetBonePos(boneNumbers[i]);
		Vector BoneScreen;

		if (Render::WorldToScreen(Bone, BoneScreen))
		{
			if (i>0)
			{
				Render::Line(LastBoneScreen.x, LastBoneScreen.y, BoneScreen.x, BoneScreen.y, color);
			}
		}
		LastBoneScreen = BoneScreen;
	}
}

void DrawBoneTest(IClientEntity *pEntity)
{
	for (int i = 0; i < 127; i++)
	{
		Vector BoneLoc = pEntity->GetBonePos(i);
		Vector BoneScreen;
		if (Render::WorldToScreen(BoneLoc, BoneScreen))
		{
			char buf[10];
			_itoa_s(i, buf, 10);
			Render::Text(BoneScreen.x, BoneScreen.y, Color(255, 255, 255, 180), Render::Fonts::ESP, buf);
		}
	}
}

void CEsp::DrawSkeleton(IClientEntity* pEntity)
{
	studiohdr_t* pStudioHdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);

			if (Render::WorldToScreen(vParent, sParent) && Render::WorldToScreen(vChild, sChild))
			{
				Render::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(255, 50, 50, 255));
			}
		}
	}
}

DWORD GlowManager = *(DWORD*)(Utilities::Memory::FindPatternV2("client.dll", "0F 11 05 ?? ?? ?? ?? 83 C8 01 C7 05 ?? ?? ?? ?? 00 00 00 00") + 3);


void CEsp::DrawGlow(Color color)
{


	CGlowObjectManager* GlowObjectManager = (CGlowObjectManager*)GlowManager;

	for (int i = 0; i < GlowObjectManager->size; ++i)
	{
		CGlowObjectManager::GlowObjectDefinition_t* glowEntity = &GlowObjectManager->m_GlowObjectDefinitions[i];
		IClientEntity* Entity = glowEntity->getEntity();

		if (glowEntity->IsEmpty() || !Entity)
			continue;

		switch (Entity->GetClientClass()->m_ClassID)
		{

		case 35:
			if (Menu::Window.VisualsTab.OtherGlow.GetState() && Entity->GetClientClass()->m_ClassID == (int)CSGOClassID::CCSPlayer)
			{
				/*if (Menu::Window.VisualsTab.FiltersPlayers.GetState())*/
					glowEntity->set(Color(color));
			}
		}
	}
}

void CEsp::GrenadeTrace()
{
}

void CEsp::GrenadeTraceOffMeme()
{
}

void CEsp::ShowImpacts()
{
}

void CEsp::ShowImpactsOffMeme()
{
}



