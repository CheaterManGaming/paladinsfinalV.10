

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <stdint.h>
#include <thread>
#include <list>

#include "paladin.h"
#include "overlay.h"
#include "pku.h"
#include "Mhyprot/mhyprot.hpp"

using X2295::D3DOverlay;


UEngine CurrentUEngine;
ULocalPlayer CurrentLocalPlayer;
APlayerController CurrentController;
APawn CurrentAcknowledgedPawn;
ACamera CurrentCamera;
AWorldInfo CurrentWorldInfo;
APawn CurrentPawnList;
APawn LockedPawn;

bool IsValid(uint64_t adress)
{
	if (adress == 0 || adress == 0xCCCCCCCCCCCCCCCC) {
		return false;
	}
	return true;
}

void CallAimbot()
{
	if (!IsValid(LockedPawn.data)) return;
	int Health = LockedPawn.GetHealth();
	if (Health > 1)
	{
		FRotator AimRotation = FRotator{ 0,0,0 };
		bool isVisible = LockedPawn.GetMesh().IsVisible(CurrentWorldInfo.GetTimeSeconds());
		if (isVisible)
		{
			FVector TargetLocation = LockedPawn.GetLocation();
			if (PKU.b_isPredictionAim)
			{
				auto currProjectiles = CurrentAcknowledgedPawn.GetWeapon().GetProjectiles();
				if (currProjectiles.Length() > 0)
				{
					PKU.fl_Speed = currProjectiles.GetById(0).GetSpeed();
				}
				FVector TargetVelocity = LockedPawn.GetVelocity();
				float TravelTime = vema::GetDistance(CurrentAcknowledgedPawn.GetLocation(), TargetLocation) / PKU.fl_Speed;
				TargetLocation = {
					(TargetLocation.X + TargetVelocity.X * TravelTime),
					(TargetLocation.Y + TargetVelocity.Y * TravelTime),
					 TargetLocation.Z
				};
			}
			FVector RealLocation = CurrentCamera.GetRealLocation();

			vema::AimAtVector(TargetLocation + FVector(0, 0, LockedPawn.GetEyeHeight()), RealLocation, AimRotation);

			AimRotation.Yaw = vema::ClampYaw(AimRotation.Yaw);
			AimRotation.Pitch = vema::ClampPitch(AimRotation.Pitch);

			if (PKU.b_AIm)
			{
				if (PKU.in_AimType == 0)
				{
					vema::AimAtVector(TargetLocation + FVector(0, 0, LockedPawn.GetEyeHeight()), RealLocation, AimRotation);
				}

				//Body Aim
				else if (PKU.in_AimType == 1)
				{
					vema::AimAtVector(TargetLocation + FVector(0, 0, LockedPawn.GetEyeHeight() / 2), RealLocation, AimRotation);
				}
			}

			if (PKU.b_Smoothing)
			{
				FRotator currentRotation = CurrentController.GetRotation();
				currentRotation.Roll = 0;

				auto diff = currentRotation - AimRotation;

				auto realDiff = diff;
				auto a = vema::ClampYaw(currentRotation.Yaw);
				auto b = vema::ClampYaw(AimRotation.Yaw);
				const auto Full360 = Const_URotation180 * 2;

				auto dist1 = -(a - b + Full360) % Full360;
				auto dist2 = (b - a + Full360) % Full360;

				auto dist = dist1;
				if (abs(dist2) < abs(dist1)) {
					dist = dist2;
				}

				auto smoothAmount = PKU.fl_SmoothingValue;

				if (PKU.b_LockWhenClose && abs(dist) + abs(diff.Pitch) < Const_URotation180 / 100) {
					smoothAmount = 1;
				}

				diff.Yaw = (int)(dist * smoothAmount);
				diff.Pitch = (int)(diff.Pitch * smoothAmount);
				AimRotation = currentRotation + diff;
			}
			AimRotation.Pitch = AimRotation.Pitch;
			CurrentController.SetRotation(AimRotation);
		}
	}
	else
	{
		PKU.b_Locked = false;
		LockedPawn = APawn{};
		return;
	}
}


void RenderVisual()
{
	auto width = D3DOverlay::getWidth();
	auto height = D3DOverlay::getHeight();

	CurrentUEngine = GetUEngine(PakuAlam.dwProcess_Base);
	CurrentLocalPlayer = CurrentUEngine.GetLocalPlayer();
	CurrentController = CurrentLocalPlayer.GetController();
	CurrentAcknowledgedPawn = CurrentController.GetAcknowledgedPawn();
	CurrentCamera = CurrentController.GetCamera();
	CurrentWorldInfo = CurrentController.GetWorldInfo();
	CurrentPawnList = CurrentWorldInfo.GetPawnList();
	CurrentAcknowledgedPawn.SetGlowhack(PKU.b_glowEnabled);
	CurrentAcknowledgedPawn.GetWeapon().SetPerspective(PKU.b_thirdPerson);
	PKU.in_CurrentHealth = CurrentAcknowledgedPawn.GetHealth();
	PKU.fl_CurrentFOV = CurrentCamera.GetDeafultFov() * CurrentController.GetFovMultiplier();


	if (PKU.b_Aimbot)
	{
		if (PKU.b_AimFOV)
		{
			D3DOverlay::DrawCircle(width / 2, height / 2, vema::GetFOV(PKU.fl_AimFov), PKU.FovColor);
		}
	}

	APawn CurrentPawn = CurrentPawnList;
	int players = 0;
	bool isAimbotActive = PKU.b_Aimbot && GetAimKey();

	if (!isAimbotActive || !IsValid(LockedPawn.data) || !LockedPawn.GetMesh().IsVisible(CurrentWorldInfo.GetTimeSeconds())) {
		LockedPawn = APawn{};
		PKU.b_Locked = false;
	}

	float ClosestDistance = 999999.0f;

	auto currentTeamIndex = CurrentAcknowledgedPawn.GetPlayerReplicationInfo().GetTeamInfo().GetTeamIndex();

	while (IsValid(CurrentPawn.data))
	{
		APawn nextPawn = CurrentPawn.GetNextPawn();
		if (!IsValid(CurrentPawn.data))
		{
			CurrentPawn = nextPawn;
			continue;
		}

		int Hp = CurrentPawn.GetHealth();
		players++;
		auto repInfo = CurrentPawn.GetPlayerReplicationInfo();
		if (!IsValid(repInfo.data)) {
			CurrentPawn = nextPawn;
			continue;
		}

		auto teamInfo = repInfo.GetTeamInfo();
		if (!IsValid(teamInfo.data)) {
			CurrentPawn = nextPawn;
			continue;
		}

		auto teamIndex = teamInfo.GetTeamIndex();

		if (teamIndex == currentTeamIndex ||
			CurrentPawn.data == CurrentAcknowledgedPawn.data ||
			Hp < 1 || Hp > 10000)
		{
			CurrentPawn = nextPawn;
			continue;
		}

		auto mesh = CurrentPawn.GetMesh();
		if (!IsValid(mesh.data)) {
			CurrentPawn = nextPawn;
			continue;
		}


		FBoxSphereBounds PlayerHitbox = mesh.GetBounds();

		FVector bottom, head, pawnPos;
		Vec2 bottompos, headpos, pos;

		bottom = vema::VectorSubtract(PlayerHitbox.Origin, PlayerHitbox.BoxExtent);
		head = vema::VectorAdd(PlayerHitbox.Origin, PlayerHitbox.BoxExtent);

		pawnPos = CurrentPawn.GetLocation();

		FVector posHead = pawnPos;
		FRotator Rotation = CurrentController.GetRotation();
		FVector RealLocation = CurrentCamera.GetRealLocation();

		bool isPawnVisible = mesh.IsVisible(CurrentWorldInfo.GetTimeSeconds());
		auto posw2s = vema::W2S(pawnPos, pos, Rotation, RealLocation, PKU.fl_CurrentFOV);
		auto tracerDistance = vema::VectorMagnitude(vema::VectorSubtract(RealLocation, pawnPos)) / 50.f;
		float red = max(0, 250 - tracerDistance) / 255.f;
		float blue = min(250, tracerDistance) / 255.f;
		Vec2 normalizedHead;
		normalizedHead.x = pos.x - SCX;
		normalizedHead.y = pos.y - SCY;
		vema::Normalize(normalizedHead);
		const float offPKUCircle = 10.0f;

		auto tracerColor = ImColor(p1, blue , p2);
		if (p3)
		{
			tracerColor = ImColor(0.f, 255.f / 255.f, 0.f, 1.f);
		}

		auto te1 = vema::W2S(bottom, bottompos, Rotation, RealLocation, PKU.fl_CurrentFOV);
		auto te2 = vema::W2S(head, headpos, Rotation, RealLocation, PKU.fl_CurrentFOV);

		try
		{
			if (te1 && te2 && posw2s)

			{

				float box_height = abs(headpos.y - bottompos.y);
				float box_width = box_height * 0.70;

				//After PKUting Visual cheat
				if (GetAsyncKeyState(VK_F1) & 1)
					PKU.b_Visual = !PKU.b_Visual;

				//After PKUting Aimbot cheat
				if (GetAsyncKeyState(VK_F2) & 1)
					PKU.PKU.b_Aimbot = !PKU.PKU.b_Aimbot;

				if (PKU.b_Visual)
				{
					//BOX
					if (PKU.b_EspBox)
					{
						if (PKU.in_BoxType == 0)
						{
							D3DOverlay::DrawBox(PKU.BoxVisColor, pos.x - (box_width / 2), headpos.y, box_width, box_height);
						}
						else if (PKU.in_BoxType == 1)
						{
							D3DOverlay::DrawCorneredBox(pos.x - (box_width / 2), headpos.y, box_width, box_height, PKU.BoxVisColor, 1.5f);
						}
					}
					//HP
					if (PKU.b_EspHealth)
					{

						auto maxHP = CurrentPawn.GetMaxHealth();
						auto procentage = Hp * 100 / maxHP;

						float width = box_width / 10;
						if (width < 2.f) width = 2.;
						if (width > 3) width = 3;

						D3DOverlay::ProgressBar(pos.x - (box_width / 2), headpos.y, width, bottompos.y - headpos.y, procentage, 100, PKU.HealthBarColor, true);
					}
					//Name
					if (PKU.b_EspName)
					{
						D3DOverlay::DrawString(ImVec2(pos.x + (box_width / 1.8), headpos.y), repInfo.GetName().ToString(), PKU.NameBarColor);
					}
					//Line
					if (PKU.b_EspLine)
					{

						if (PKU.in_LineType == 0)
						{
							D3DOverlay::DrawLine(ImVec2(static_cast<float>(width / 2), static_cast<float>(height)), ImVec2(bottompos.x, bottompos.y), PKU.LineColor, 1.3f); //LINE FROM BOTTOM SCREEN
						}
						if (PKU.in_LineType == 1)
						{
							D3DOverlay::DrawLine(ImVec2(static_cast<float>(width / 2), 0.f), ImVec2(bottompos.x, bottompos.y), PKU.LineColor, 1); //LINE FROM TOP SCREEN
						}
						if (PKU.in_LineType == 2)
						{
							D3DOverlay::DrawLine(ImVec2(static_cast<float>(width / 2), static_cast<float>(height / 2)), ImVec2(bottompos.x, bottompos.y), PKU.LineColor, 1.3f); //LINE FROM BOTTOM SCREEN
						}

					}
				}
				if (PKU.b_TracerMenu)
				{
					//BoxTracer
					if (PKU.b_Boxtracer)
					{
						if (PKU.in_BoxType == 0)
						{
							D3DOverlay::DrawBox(tracerColor, pos.x - (box_width / 2), headpos.y, box_width, box_height);
						}
						else if (PKU.in_BoxType == 1)
						{
							D3DOverlay::DrawCorneredBox(pos.x - (box_width / 2), headpos.y, box_width, box_height, tracerColor, 1.5f);
						}
					}
					//LineTracers
					if (PKU.b_EspLineTracer)
					{

						if (PKU.in_LineType == 0)
						{
							D3DOverlay::DrawLine(ImVec2(static_cast<float>(width / 2), static_cast<float>(height)), ImVec2(bottompos.x, bottompos.y), tracerColor, 1.3f); //LINE FROM BOTTOM SCREEN
						}
						if (PKU.in_LineType == 1)
						{
							D3DOverlay::DrawLine(ImVec2(static_cast<float>(width / 2), 0.f), ImVec2(bottompos.x, bottompos.y), tracerColor, 1); //LINE FROM TOP SCREEN
						}
						if (PKU.in_LineType == 2)
						{
							D3DOverlay::DrawLine(ImVec2(static_cast<float>(width / 2), static_cast<float>(height / 2)), ImVec2(bottompos.x, bottompos.y), tracerColor, 1.3f); //LINE FROM BOTTOM SCREEN
						}

					}
					//Glow
					if (PKU.b_glowEnabled)
					{
					
					}
				}
				if (isAimbotActive && isPawnVisible && !PKU.b_Locked)
				{
					Vec2 headPos;
					if (vema::W2S(posHead, headPos, Rotation, RealLocation, PKU.fl_CurrentFOV))
					{
						//TODO PKUTING
						float ScreenCX = PakuAlam.ScreenWidth / 2;
						float ScreenCY = PakuAlam.ScreenHeight / 2;
						float cx = headPos.x;
						float cy = headPos.y;
						float radiusx_ = PKU.fl_AimFov * (ScreenCX / PKU.fl_CurrentFOV);
						float radiusy_ = PKU.fl_AimFov * (ScreenCY / PKU.fl_CurrentFOV);
						float crosshairDistance = vema::GetCrosshairDistance(cx, cy, ScreenCX, ScreenCY);
						if (tracerDistance < 30.f || cx >= ScreenCX - radiusx_ && cx <= ScreenCX + radiusx_ && cy >= ScreenCY - radiusy_ && cy <= ScreenCY + radiusy_)
						{
							if (crosshairDistance < ClosestDistance)
							{
								ClosestDistance = crosshairDistance;
								LockedPawn = CurrentPawn;
							}
						}
						CurrentPawn = nextPawn;
						continue;
					}
				}
			}
			CurrentPawn = nextPawn;
		}
		catch (const std::exception&) {
			CurrentPawn = nextPawn;
			continue;
		}
	}

	if (IsValid(LockedPawn.data))
	{
		PKU.b_Locked = true;
		CallAimbot();
	}
}

ImFont* info = nullptr;
IDirect3DTexture9* logggo = nullptr;
ImFont* info_big = nullptr;
ImFont* two = nullptr;
ImFont* three = nullptr;
ImFont* tabsf = nullptr;
ImFont* ee = nullptr;
ImVec2 pos;
ImDrawList* draw;

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

float dpi_scale = 1.f;


static int tabs;
static int subtabs;

auto Render(FLOAT aWidth, FLOAT aHeight)->VOID
{

	if (GetAsyncKeyState(VK_INSERT) & 1)
		PKU.b_MenuShow = !PKU.b_MenuShow;

	if (PKU.b_MenuShow)
	{
		ImGui::SetNextWindowSize(ImVec2(630, 350));// 450,426
		ImGui::PushFont(info);
		//ImGui::PushFont(X2295::DefaultFont);
		ImGui::Begin(enc("PALADINS HACK 2022 // Credit by : Derisman"), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

		//Visual Function
		ImGui::SetCursorPos(ImVec2(19, 80));
		ImGui::BeginGroup();
		//D3DOverlay::TabButton(enc("Visual"), &SET.in_tab_index, 0, true);
		//D3DOverlay::TabButton(enc("Aimbot"), &SET.in_tab_index, 1, false);
		//D3DOverlay::TabButton(enc("Misc"), &SET.in_tab_index, 2, false);
		if (ImGui::tab("Color ESP", 0 == tabs))
			tabs = 0;
		if (ImGui::tab("Tracer ESP", 1 == tabs))
			tabs = 1;
		if (ImGui::tab("Aimbot", 2 == tabs))
			tabs = 2;
		if (ImGui::tab("Misc", 3 == tabs))
			tabs = 3;
		if (ImGui::tab("Colors", 4 == tabs))
			tabs = 4;
		//if (ImGui::tab("Special Box", 4 == tabs))
			//tabs = 4;

		ImGui::EndGroup();

		ImGui::PopFont();

		ImGui::PushFont(info);

		static bool da = false;
		static int net = 0;
		static int combo = 0;

		if (tabs == 0)
		{
			ImGui::SetCursorPos(ImVec2(180, 30));
			ImGui::BeginChild("ESP 1", ImVec2(235, 382));
			{
				ImGui::SetCursorPos(ImVec2(10, 20));
				ImGui::BeginGroup();
				{
					ImGui::Checkbox(enc("Colors ESP", &da), &PKU.b_Visual);
					if (PKU.b_Visual)
					{
						ImGui::Checkbox(enc("ESP Box"), &PKU.b_EspBox);
						if (PKU.b_EspBox)
						{
							ImGui::Combo(enc("Box Type"), &PKU.in_BoxType, PKU.BoxTypes, 2);
						}
						ImGui::Checkbox(enc("ESP Line"), &PKU.b_EspLine);
						if (PKU.b_EspLine)
						{
							ImGui::Combo(enc("Line Type"), &PKU.in_LineType, PKU.LineTypes, 3);
						}
						ImGui::Checkbox(enc("ESP Name"), &PKU.b_EspName);
						ImGui::Checkbox(enc("ESP HealthBar"), &PKU.b_EspHealth);
					}

				}
				ImGui::PushFont(info);
				ImGui::EndGroup();
			}
		}
		if (tabs == 1)
		{
			ImGui::SetCursorPos(ImVec2(180, 30));
			ImGui::BeginChild("ESP 2", ImVec2(235, 382));
			{
				ImGui::SetCursorPos(ImVec2(10, 20));
				ImGui::BeginGroup();
				{
					ImGui::Checkbox(enc("Tracers ESP", &da), &PKU.b_TracerMenu);
					if (PKU.b_TracerMenu)
					{
						//Glow
						ImGui::Checkbox(enc("ESP Glow"), &PKU.b_glowEnabled);
						//BOX Tracers
						ImGui::Checkbox(enc("Box Tracer"), &PKU.b_Boxtracer);
						if (PKU.b_Boxtracer)
						{
							ImGui::Combo(enc("Box Type"), &PKU.in_BoxType, PKU.BoxTypes, 2);
						}
						//Line Tracers
						ImGui::Checkbox(enc("Line Tracers"), &PKU.b_EspLineTracer);
						if (PKU.b_EspLineTracer)
						{
							ImGui::Combo(enc("Line Type"), &PKU.in_LineType, PKU.LineTypes, 3);
						}
					}
				}
				ImGui::PushFont(info);
				ImGui::EndGroup();
			}
		}
		if (tabs == 2)
		{
			ImGui::SetCursorPos(ImVec2(180, 30));
			ImGui::BeginChild("AIM", ImVec2(235, 382));
			{
				ImGui::SetCursorPos(ImVec2(10, 20));
				ImGui::BeginGroup();
				{
					ImGui::Checkbox(enc("Enabled Aimbot", &da), &PKU.b_Aimbot);
					if (PKU.b_Aimbot)
					{
						ImGui::Checkbox(enc("Aim Type"), &PKU.b_AIm);
						if (PKU.b_AIm)
						{
							ImGui::Combo(enc("AIM"), &PKU.in_AimType, PKU.AimType, 2);;
						}
						ImGui::Text(enc("SELECT KEY"));
						ImGui::Combo("AimKey", &PKU.in_AimKey, keyItems, IM_ARRAYSIZE(keyItems));
						ImGui::Checkbox(enc("Draw FOV"), &PKU.b_AimFOV);
						ImGui::SliderFloat(enc("value"), &PKU.fl_SmoothingValue, 0.0f, 1.0f);
						if (PKU.b_AimFOV)
						{
							ImGui::SliderFloat(enc("FOV Value"), &PKU.fl_AimFov, 5.f, 90.f);
						}
					}
				}
				ImGui::PushFont(info);
				ImGui::EndGroup();
			}
		}
		if (tabs == 3)
		{
			ImGui::SetCursorPos(ImVec2(180, 30));
			ImGui::BeginChild("Misc", ImVec2(235, 382));
			{
				ImGui::SetCursorPos(ImVec2(10, 20));
				ImGui::BeginGroup();
				{
					ImGui::Checkbox(enc("Enable Misc"), &PKU.b_Misc);
					if (PKU.b_Misc)
					{
						ImGui::Checkbox(enc("PredictionAim"), &PKU.b_isPredictionAim);
						ImGui::Checkbox(enc("LockWhenClose"), &PKU.b_LockWhenClose);
						ImGui::Checkbox(enc("thirdPerson"), &PKU.b_thirdPerson);
					}
				}
			}
			ImGui::PushFont(info);
			ImGui::EndGroup();
		}
		if (tabs == 4)
		{
			ImGui::SetCursorPos(ImVec2(180, 50));
			ImGui::BeginChild("Colors", ImVec2(235, 382));
			{
				ImGui::SetCursorPos(ImVec2(10, 40));
				ImGui::BeginGroup();
				{
					//BoxColor
					if (PKU.b_EspBox)
					{
						if (ImGui::ColorEdit4(enc("Box"), PKU.fl_BoxVisColor))
						{
							PKU.BoxVisColor = ImColor(PKU.fl_BoxVisColor[0], PKU.fl_BoxVisColor[1], PKU.fl_BoxVisColor[2], PKU.fl_BoxVisColor[3]);
						}
					}

					//LineColor
					if (PKU.b_EspLine)
					{
						if (ImGui::ColorEdit4(enc("Line"), PKU.fl_LineColor))
						{
							PKU.LineColor = ImColor(PKU.fl_LineColor[0], PKU.fl_LineColor[1], PKU.fl_LineColor[2], PKU.fl_LineColor[3]);
						}
					}

					//NameColor
					if (PKU.b_EspName)
					{
						if (ImGui::ColorEdit4(enc("Name"), PKU.fl_NameBarColor))
						{
							PKU.NameBarColor = ImColor(PKU.fl_NameBarColor[0], PKU.fl_NameBarColor[1], PKU.fl_NameBarColor[2], PKU.fl_NameBarColor[3]);
						}
					}

					//HealthBarBar
					if (PKU.b_EspHealth)
					{
						if (ImGui::ColorEdit4(enc("Health"), PKU.fl_HealthBarColor))
						{
							PKU.HealthBarColor = ImColor(PKU.fl_HealthBarColor[0], PKU.fl_HealthBarColor[1], PKU.fl_HealthBarColor[2], PKU.fl_HealthBarColor[3]);
						}
					}

					//FovColor
					if (PKU.b_AimFOV)
					{
						if (ImGui::ColorEdit4(enc("FOV"), PKU.fl_FovColor))
						{
							PKU.FovColor = ImColor(PKU.fl_FovColor[0], PKU.fl_FovColor[1], PKU.fl_FovColor[2], PKU.fl_FovColor[3]);
						}
					}
				}
				ImGui::PushFont(info);
				ImGui::EndGroup();
			}
		}
		ImGui::PopFont();
		ImGui::End();
	}
	RenderVisual();
}


int main()
{

	system(skCrypt("sc stop mhyprot2")); // RELOAD DRIVER JUST IN CASE
	system(skCrypt("CLS")); // CLEAR

	auto process_name = skCrypt("Paladins.exe");
	auto process_id = GetProcessId(process_name);
	if (!process_id)
	{
		printf(skCrypt("[!] process \"%s\ was not found\n"), process_name);
		return -1;
	}

	printf(skCrypt("[+] %s (%d)\n"), process_name, process_id);

	//
	// initialize its service, etc
	//
	if (!mhyprot::init())
	{
		printf(skCrypt("[!] failed to initialize vulnerable driver\n"));
		return -1;
	}

	if (!mhyprot::driver_impl::driver_init(
		false, // print debug
		false // print seedmap
	))
	{
		printf(skCrypt("[!] failed to initialize driver properly\n"));
		mhyprot::unload();
		return -1;
	}
	PakuAlam.dwProcess_Base = GetProcessBase(process_id);
	if (!PakuAlam.dwProcess_Base) {
		printf(skCrypt("[!] failed to get baseadress\n"));
		mhyprot::unload();
		return -1;
	}

	//printf("[+] Game Base is 0x%llX\n", process_base);
	IMAGE_DOS_HEADER dos_header = read<IMAGE_DOS_HEADER>(PakuAlam.dwProcess_Base);
	printf(skCrypt("[+] Game header Magic is 0x%llX\n"), dos_header.e_magic);
	if (dos_header.e_magic != 0x5A4D) {
		printf(skCrypt("[!] Game header Magic should be 0x5A4D\n"));
	}

	HWND hDesktop = GetDesktopWindow(); (PakuAlam.dwProcessId);
	if (IsWindow(hDesktop))

	{
		if (D3DOverlay::InitOverlay(_T("xxxxx"), _T("xxxxxxxxxx")))
		{

			PakuAlam.ScreenWidth = D3DOverlay::getWidth();
			PakuAlam.ScreenHeight = D3DOverlay::getHeight();

			ImGuiIO& io = ImGui::GetIO();
			X2295::DefaultFont = io.Fonts->AddFontDefault();
			X2295::Century_Gothic = io.Fonts->AddFontFromFileTTF("Century_Gothic.ttf", 16);
			io.Fonts->Build();

			D3DOverlay::SetUserRender(Render);
			while (D3DOverlay::MsgLoop() && D3DOverlay::AttachWindow(hDesktop)) {};
			D3DOverlay::UninitOverlay();
		}
	}
	return 0;
}