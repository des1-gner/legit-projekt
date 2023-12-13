/*
Syn's AyyWare Framework 2015
*/

#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" // for the unload meme
#include "Interfaces.h"
#include "CRC32.h"



AyyWareWindow Menu::Window;



void AyyWareWindow::Setup()
{


	RegisterTab(&LegitBotTab);
	RegisterTab(&VisualsTab);
	RegisterTab(&MiscTab);



	LegitBotTab.Setup();
	VisualsTab.Setup();
	MiscTab.Setup();


}


void CLegitBotTab::Setup()
{
	SetTitle("LEGIT");

	ActiveLabel.SetPosition(32, 48);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(78, 48);
	RegisterControl(&Active);

#pragma region Aimbot
	AimbotGroup.SetPosition(16, 48);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(376, 250);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl("Legitbot", this, &AimbotEnable);

	AimbotAutoPistol.SetFileId("aim_apistol");
	AimbotGroup.PlaceLabledControl("Automatatic Pistol", this, &AimbotAutoPistol);

	AimbotBacktrack.SetFileId("legit_backtrack");
	AimbotGroup.PlaceLabledControl("Backtrack Ticks", this, &AimbotBacktrack);

	TickModulation.SetFileId("tick_modulate");
	TickModulation.SetBoundaries(0.1f, 13.f);
	TickModulation.SetValue(13.f);
	AimbotGroup.PlaceLabledControl("Tick Modulation", this, &TickModulation);


#pragma endregion Aimbot shit
}

void CVisualTab::Setup()
{
	SetTitle("VISUALS");

	ActiveLabel.SetPosition(32, 48);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(78, 48);
	RegisterControl(&Active);

#pragma region Options
	OptionsGroup.SetText("Options");
	OptionsGroup.SetPosition(16, 48);
	OptionsGroup.SetSize(376, 250);
	RegisterControl(&OptionsGroup);

	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	OptionsGroup.PlaceLabledControl("Enemy Only", this, &FiltersEnemiesOnly);

	OptionsBox.SetFileId("otr_recoilhair");
/*	OptionsBox.AddItem("Off");
	OptionsBox.AddItem("Full");
	OptionsBox.AddItem("Corner");*/
	OptionsGroup.PlaceLabledControl("Show Box", this, &OptionsBox);

	OptionsFillBox.SetFileId("opt_Fill");
	OptionsGroup.PlaceLabledControl("Box Fill", this, &OptionsFillBox);

	OptionsName.SetFileId("opt_name");
	OptionsGroup.PlaceLabledControl("Show Name", this, &OptionsName);

	OptionsVitals.SetFileId("opt_hp");
	OptionsVitals.AddItem("Off");
	OptionsVitals.AddItem("Solid Bar");
	OptionsVitals.AddItem("Sectioned Bar");
	OptionsGroup.PlaceLabledControl("Health", this, &OptionsVitals);

	OptionsWeapon.SetFileId("opt_weapon");
	OptionsGroup.PlaceLabledControl("Identify Weapon", this, &OptionsWeapon);

	OptionsInfo.SetFileId("opt_info");
	OptionsGroup.PlaceLabledControl("Show Info", this, &OptionsInfo);

	BacktrackingLol.SetFileId("opt_backdot");
	OptionsGroup.PlaceLabledControl("Backtrack Visualization", this, &BacktrackingLol);

	OptionsChams.SetFileId("opt_chams");
	OptionsChams.AddItem("Off");
	OptionsChams.AddItem("Material");
	OptionsChams.AddItem("Color");
	OptionsGroup.PlaceLabledControl("Chams", this, &OptionsChams);

	ChamsXQZ.SetFileId("opt_xqz");
	OptionsGroup.PlaceLabledControl("XQZ Chams", this, &ChamsXQZ);

	OptionsSkeleton.SetFileId("opt_bone");
	OptionsGroup.PlaceLabledControl("Draw Bones", this, &OptionsSkeleton);

	/*OptionsGlow.SetFileId("opt_glow");
	OptionsGroup.PlaceLabledControl("Glow", this, &OptionsGlow);*/

//	OptionsAimSpot.SetFileId("opt_aimspot");
//	OptionsGroup.PlaceLabledControl("Head Cross", this, &OptionsAimSpot);

//	OptionsCompRank.SetFileId("opt_comprank");
//	OptionsGroup.PlaceLabledControl("Player Ranks", this, &OptionsCompRank);


	FiltersWeapons.SetFileId("ftr_weaps");
	OptionsGroup.PlaceLabledControl("Show Items", this, &FiltersWeapons);

	ShowImpacts.SetFileId("opt_impacts");
	OptionsGroup.PlaceLabledControl("Show Impacts", this, &ShowImpacts);

//	GrenadeTracer.SetFileId("opt_grentrail");
//	OptionsGroup.PlaceLabledControl("Show Grenade Trail", this, &GrenadeTracer);

	IsScoped.SetFileId("opt_scoped");
	OptionsGroup.PlaceLabledControl("Show Scoped", this, &IsScoped);

	HasDefuser.SetFileId("opt_dontbealoserbuyadefuser");
	OptionsGroup.PlaceLabledControl("Show Defuser", this, &HasDefuser);

	IsDefusing.SetFileId("opt_hedufusinglel");
	OptionsGroup.PlaceLabledControl("Show Defusing", this, &IsDefusing);

	RemoveScope.SetFileId("opt_RemoveScope");
	OptionsGroup.PlaceLabledControl("Remove Scope", this, &RemoveScope);

	OptionsBarrel.SetFileId("aa_Barrel");
	OptionsGroup.PlaceLabledControl("Line Tracing", this, &OptionsBarrel);

/*	OptionsLBY.SetFileId("aa_LBY");
	OptionsGroup.PlaceLabledControl("LBY indicator", this, &OptionsLBY);*/

	//OtherGlow.SetFileId("aa_glow");
	//OptionsGroup.PlaceLabledControl("Spoof Spectator", this, &OtherGlow);

#pragma endregion Setting up the Options controls

/*#pragma region Filters
	FiltersGroup.SetText("Filters");
	FiltersGroup.SetPosition(225, 48);
	FiltersGroup.SetSize(193, 430);
	RegisterControl(&FiltersGroup);

	//FiltersAll.SetFileId("ftr_all");
	//FiltersGroup.PlaceLabledControl("All", this, &FiltersAll);

	//FiltersPlayers.SetFileId("ftr_players");
	//FiltersGroup.PlaceLabledControl("Players", this, &FiltersPlayers);

//	FiltersChickens.SetFileId("ftr_chickens");
//	FiltersGroup.PlaceLabledControl("Chickens", this, &FiltersChickens);

	//FiltersC4.SetFileId("ftr_c4");
	//FiltersGroup.PlaceLabledControl("Bomb", this, &FiltersC4);
#pragma endregion Setting up the Filters controls*/

#pragma region Other
	OtherGroup.SetText("Other");
	OtherGroup.SetPosition(408, 48);
	OtherGroup.SetSize(360, 360);
	RegisterControl(&OtherGroup);

/*	OtherRecoilCrosshair.SetFileId("otr_recoilhair");
	OtherRecoilCrosshair.AddItem("Off");
	OtherRecoilCrosshair.AddItem("Recoil");
	OtherRecoilCrosshair.AddItem("Spread");
	OtherGroup.PlaceLabledControl("Dynamic Crosshair", this, &OtherRecoilCrosshair);*/

	OtherRadar.SetFileId("otr_radar");
	OtherGroup.PlaceLabledControl("Radar", this, &OtherRadar);

	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	OtherGroup.PlaceLabledControl("Adjust Visual Recoil", this, &OtherNoVisualRecoil);

	OtherNoFlash.SetFileId("otr_noflash");
	OtherGroup.PlaceLabledControl("Remove Flash", this, &OtherNoFlash);

	OtherNoSmoke.SetFileId("otr_nosmoke");
	OtherGroup.PlaceLabledControl("Remove Smoke", this, &OtherNoSmoke);

	OtherCrosshair.SetFileId("otr_cross");
	OtherGroup.PlaceLabledControl("cross", this, &OtherCrosshair);


//	OtherEnemyCircle.SetFileId("otr_enemccirc");
//	OtherGroup.PlaceLabledControl("Enemy Circle", this, &OtherEnemyCircle);

	OtherNoHands.SetFileId("otr_hands");
	OtherNoHands.AddItem("Off");
	OtherNoHands.AddItem("None");
	OtherNoHands.AddItem("Glass");
	OtherNoHands.AddItem("Gold");
	OtherNoHands.AddItem("Wireframe");
	OtherNoHands.AddItem("Mixed");
	OtherGroup.PlaceLabledControl("Hand Adjustments", this, &OtherNoHands);

	HandXQZ.SetFileId("otr_handxqz");
	OtherGroup.PlaceLabledControl("XQZ", this, &HandXQZ);

	OtherFOV.SetFileId("otr_fov");
	OtherFOV.SetBoundaries(0.f, 180.f);
	OtherFOV.SetValue(90.f);
	OtherGroup.PlaceLabledControl("Override FOV", this, &OtherFOV);

	OtherViewmodelFOV.SetFileId("otr_viewfov");
	OtherViewmodelFOV.SetBoundaries(0.f, 180.f);
	OtherViewmodelFOV.SetValue(0.f);
	OtherGroup.PlaceLabledControl("Viewmodel FOV", this, &OtherViewmodelFOV);

	/*Test.SetFileId("rgb_test");
	Test.SetBoundaries(0.f, 0.9f);
	Test.SetValue(0.f);
	OtherGroup.PlaceLabledControl(" ", this, &Test);*/

#pragma endregion Setting up the Other controls

#pragma region WorldOptions
/*	CvarGroup.SetText("World");
	CvarGroup.SetPosition(434, 320);
	CvarGroup.SetSize(334, 170);
	RegisterControl(&CvarGroup);

	NightMode.SetFileId("otr_nightmode");
	NightMode.SetState(false);
	CvarGroup.PlaceLabledControl("Night-Mode", this, &NightMode);

	AmbientExposure.SetFileId("otr_ambientexposure");
	AmbientExposure.SetBoundaries(0.f, 2.f);
	AmbientExposure.SetValue(2.f);
	CvarGroup.PlaceLabledControl("Ambient-Exposure", this, &AmbientExposure);

	AmbientRed.SetFileId("otr_ambientred");
	AmbientRed.SetBoundaries(0.f, .5f);
	AmbientRed.SetValue(0.f);
	CvarGroup.PlaceLabledControl("Ambient-Red", this, &AmbientRed);

	AmbientGreen.SetFileId("otr_ambientgreen");
	AmbientGreen.SetBoundaries(0.f, .5f);
	AmbientGreen.SetValue(0.f);
	CvarGroup.PlaceLabledControl("Ambient-Green", this, &AmbientGreen);

	AmbientBlue.SetFileId("otr_ambientblue");
	AmbientBlue.SetBoundaries(0.f, .5f);
	AmbientBlue.SetValue(0.f);
	CvarGroup.PlaceLabledControl("Ambient-Blue", this, &AmbientBlue);

	AmbientSkybox.SetFileId("otr_skyboxchanger");
	AmbientSkybox.AddItem("Disabled");
	AmbientSkybox.AddItem("Skybox 1");
	AmbientSkybox.AddItem("Skybox 2");
	AmbientSkybox.AddItem("Skybox 3");
	AmbientSkybox.AddItem("Skybox 4");
	CvarGroup.PlaceLabledControl("Change-Skybox", this, &AmbientSkybox);*/

#pragma endregion Setting up the Other controls
}

void CMiscTab::Setup()
{
	SetTitle("MISC");



	#pragma endregion*/

#pragma region Other
	OtherGroup.SetPosition(16, 48);
	OtherGroup.SetSize(360, 430);
	OtherGroup.SetText("Other");
	RegisterControl(&OtherGroup);

	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceLabledControl("Bunnyhop", this, &OtherAutoJump);




	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.SetState(true);
	OtherGroup.PlaceLabledControl("Spread Server", this, &OtherSafeMode);
}


#pragma endregion other random options




#pragma endregion




void Menu::SetupMenu()
{
	Window.Setup();

	GUI.RegisterWindow(&Window);
	GUI.BindWindow(VK_INSERT, &Window);
}

void Menu::DoUIFrame()
{
	// General Processing

	// If the "all filter is selected tick all the others
	if (Window.VisualsTab.FiltersAll.GetState())
	{
		Window.VisualsTab.FiltersC4.SetState(true);
		Window.VisualsTab.FiltersChickens.SetState(true);
		Window.VisualsTab.FiltersPlayers.SetState(true);
		Window.VisualsTab.FiltersWeapons.SetState(true);
	}

	GUI.Update();
	//GUI.Draw();


}


