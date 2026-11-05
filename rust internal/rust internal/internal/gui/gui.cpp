#define _HAS_STD_BYTE 0
#define NOMINMAX
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <vector>
#include <string>
#include <d3d11.h>
#include <d3dx11tex.h>
#include <functional>
#include <filesystem>
#include <algorithm>
#include <animations.hpp>
#include "managers/FontManager.hpp"
#include "managers/ImageManager.hpp"
#include "compbuilder/CompBuilder.hpp"
#include "managers/WidgetsManager.hpp"
#include "managers/StyleManager.hpp"
#include "fonts.h"
#include "unicodes.hpp"
#include "managers/TabsManager.hpp"
#include "managers/ChildManager.hpp"
#include "managers/PopupManager.hpp"
#include "managers/LangManager.hpp"
#include "managers/SearchManager.hpp"
#include "../setting.hpp"
#include "../Structs.hpp"
#include "../custommodel.hpp"
#include "../chams.hpp"
#include <shlobj.h>
#include <gui.hpp>

#pragma comment(lib, "freetype64.lib")

using namespace ImGui;

#include "managers/NotifyManager.hpp"
#include "../ui_render.hpp"

namespace {
	struct BoneOption {
		const char* name;
		int id;
	};

	void color_edit3(WidgetsManager& widgets, const char* label, float col[3]) {
		float col4[4] = { col[0], col[1], col[2], 1.f };
		if (widgets.ColorEdit(label, col4)) {
			col[0] = col4[0];
			col[1] = col4[1];
			col[2] = col4[2];
		}
	}

	std::vector<std::string> list_configs() {
		std::vector<std::string> configs;
		const auto dir = std::filesystem::temp_directory_path() / L"data";
		if (!std::filesystem::exists(dir))
			return configs;

		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file() || entry.path().extension() != L".cfg")
				continue;

			const auto stem = entry.path().stem().u8string();
			if (!stem.empty())
				configs.push_back(std::string(stem.begin(), stem.end()));
		}

		std::sort(configs.begin(), configs.end());
		return configs;
	}
}

void GUI::initialize(ID3D11Device* device) {
	if (initialized)
		return;

	size = ImVec2{ 820, 560 };

	GImGui->IO.IniFilename = "";

	StyleManager::get().Styles();
	StyleManager::get().Colors();

	fonts.resize(fonts_size);
	fonts.at(font).setup(b_font, sizeof(b_font), { 14, 12 }, GetIO().Fonts->GetGlyphRangesCyrillic());

	const static ImWchar icons_ranges[] = { 0x1 + 16000, 0x1 + 17170, 0 };
	fonts.at(icons).setup(glyphter, sizeof(glyphter), { 14, 12 }, icons_ranges);

	LangManager::get().initialize();

	ChildManager& cm = ChildManager::get();
	auto& widgets = WidgetsManager::get();

	TabsManager::get().add_page(0, [&]() {
		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Aimbot")); {
				widgets.Bind(LangManager::get().translate("Enable Aim"), &setting::combat::aim_key, &setting::combat::aim_enable);
				widgets.Checkbox(LangManager::get().translate("Silent Aim"), &setting::combat::use_pSilent);

				if (setting::combat::use_pSilent) {
					widgets.Checkbox(LangManager::get().translate("Auto Fire"), &setting::combat::auto_fire);
				}

				if (setting::combat::aim_enable) {
					widgets.SliderFloat(LangManager::get().translate("Fov"), &setting::combat::aim_fov, 1.f, 1500.0f, u8"%.0f");
					if (!setting::combat::use_pSilent) {
						widgets.SliderFloat(LangManager::get().translate("Smooth"), &setting::combat::aim_smooth, 1.0f, 20.0f, "%.1f");
					}

					static int selected_part = 0;
					widgets.Combo(LangManager::get().translate("Bone"), &selected_part, { LangManager::get().translate("Head"), LangManager::get().translate("Neck"), LangManager::get().translate("Pelvis"), LangManager::get().translate("Body") });

					switch (selected_part) {
					case 0: setting::combat::target_bone = BoneList::head; break;
					case 1: setting::combat::target_bone = BoneList::neck; break;
					case 2: setting::combat::target_bone = BoneList::pelvis; break;
					case 4: setting::combat::target_bone = BoneList::l_middle2; break;
					}
				}
			}
			cm.endchild();

			cm.beginchild(LangManager::get().translate("Filters"));
			{
				widgets.Checkbox(LangManager::get().translate("Visible Check"), &setting::combat::visible_check_combat_type);

				ImGui::Spacing();
				widgets.Checkbox(LangManager::get().translate("Ignore Npc"), &setting::combat::ignore_npc_combat_type);
				widgets.Checkbox(LangManager::get().translate("Ignore Team"), &setting::combat::ignore_team_combat_type);
				widgets.Checkbox(LangManager::get().translate("Ignore Sleepers"), &setting::combat::ignore_sleepers_combat_type);
			}
			cm.endchild();
		}

		EndGroup();
		SameLine();

		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Manipulator"));
			{
				widgets.Bind(LangManager::get().translate("Manipulator"), &setting::combat::manipulated_key, &setting::combat::enable_manipulated);

				Spacing();
				widgets.SliderFloat(LangManager::get().translate("Max Angles"), &setting::combat::total_points, 10.f, 89.0f, "%0.f");
				widgets.SliderFloat(LangManager::get().translate("Manip Distance"), &setting::combat::manip_distance, 0.1f, 3.5f);

				Spacing();
				widgets.Checkbox(LangManager::get().translate("Hitscan"), &setting::combat::enable_hitscan);
				if (setting::combat::enable_hitscan) {
					widgets.SliderFloat(LangManager::get().translate("Hitscan Range"), &setting::combat::hitscan_range, 10.0f, 300.0f, "%.0f");
				}
			}
			cm.endchild();
		}
		EndGroup();
		});

	TabsManager::get().add_page(0, [&]() {
		cm.beginchild(LangManager::get().translate("Drawing")); {
			widgets.Checkbox(LangManager::get().translate("Draw Aim Fov"), &setting::combat::draw_aim_fov);
			color_edit3(widgets, LangManager::get().translate("Aim Fov Color"), setting::combat::aim_fov_color);

			Spacing();
			widgets.Checkbox(LangManager::get().translate("Draw Aim Target"), &setting::combat::draw_aim_target);
			color_edit3(widgets, LangManager::get().translate("Aim Target Color"), setting::combat::aim_target_color);

			Spacing();
			widgets.Checkbox(LangManager::get().translate("Draw Manipulator Angles"), &setting::combat::draw_manipulated);
		}
		cm.endchild();
		});

	TabsManager::get().add_page(1, [&]() {
		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("ESP")); {
				widgets.Checkbox(LangManager::get().translate("Enable ESP"), &setting::visuals::enable_esp);

				if (setting::visuals::enable_esp) {
					ImGui::Spacing();
					widgets.Checkbox(LangManager::get().translate("Box"), &setting::visuals::box_esp);
					widgets.Checkbox(LangManager::get().translate("Name"), &setting::visuals::name_esp);
					widgets.Checkbox(LangManager::get().translate("Health"), &setting::visuals::health_esp);
					widgets.Checkbox(LangManager::get().translate("Distance"), &setting::visuals::distance_esp);
					widgets.Checkbox(LangManager::get().translate("Skeleton"), &setting::visuals::skeleton_esp);

					widgets.Checkbox(LangManager::get().translate("Inventory"), &setting::visuals::inventory_esp, 0, 0, [&]() {
						widgets.SliderFloat(LangManager::get().translate("Icon Size"), &setting::visuals::inventory_icon_size, 16.f, 64.f, "%.0f");
						});
				}
			}
			cm.endchild();

			cm.beginchild(LangManager::get().translate("Filters"));
			{
				widgets.Checkbox(LangManager::get().translate("Ignore Npc"), &setting::visuals::ignore_npc_visuals_type);
				widgets.Checkbox(LangManager::get().translate("Ignore Team"), &setting::visuals::ignore_team_visuals_type);
				widgets.Checkbox(LangManager::get().translate("Ignore Sleepers"), &setting::visuals::ignore_sleepers_visuals_type);
			}
			cm.endchild();
		}

		EndGroup();
		SameLine();

		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Colors"));
			{
				color_edit3(widgets, LangManager::get().translate("Box Color"), setting::visuals::box_color);
				color_edit3(widgets, LangManager::get().translate("Name Color"), setting::visuals::name_color);
				color_edit3(widgets, LangManager::get().translate("Distance Color"), setting::visuals::distance_color);
				color_edit3(widgets, LangManager::get().translate("Skeleton Color"), setting::visuals::skeleton_color);

				Spacing();
				color_edit3(widgets, LangManager::get().translate("Npc"), setting::visuals::npc_color);
				color_edit3(widgets, LangManager::get().translate("Team"), setting::visuals::team_color);
				color_edit3(widgets, LangManager::get().translate("Sleepers"), setting::visuals::sleepers_color);
			}
			cm.endchild();
		}
		EndGroup();
		});

	TabsManager::get().add_page(1, [&]() {
		BeginGroup();
		{
			cm.beginchild("Chams");
			{
				widgets.Checkbox("Enable Chams", &setting::visuals::chams::enabled);

				if (widgets.Button("Browse Folder", { CalcItemWidth(), 0 })) {
					char folder[MAX_PATH] = {};

					HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

					BROWSEINFOA bi = {};
					bi.hwndOwner = NULL;
					bi.lpszTitle = "Select bundles folder";
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

					LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
					if (pidl) {
						if (SHGetPathFromIDListA(pidl, folder)) {
							if (folder[0]) {
								strncpy_s(setting::visuals::chams::bundle_path, folder, _TRUNCATE);
								chams::scan_bundles(true);
								if (!chams::bundle_list.empty()) {
									setting::visuals::chams::bundle_index = 0;
									chams::chams_mat = 0;
								}
							}
						}
						CoTaskMemFree(pidl);
					}

					if (SUCCEEDED(hr)) {
						CoUninitialize();
					}
				}

				if (setting::visuals::chams::bundle_path[0]) {
					ImGui::TextWrapped("Path: %s", setting::visuals::chams::bundle_path);
				}

				if (!chams::scan_done || chams::bundle_list.empty()) {
					chams::scan_bundles(false);
				}

				if (!chams::bundle_list.empty()) {
					std::vector<const char*> bundle_names;
					bundle_names.reserve(chams::bundle_list.size());
					for (const auto& b : chams::bundle_list) {
						bundle_names.push_back(b.name.c_str());
					}

					if (setting::visuals::chams::bundle_index >= (int)chams::bundle_list.size()) {
						setting::visuals::chams::bundle_index = 0;
					}

					int prev_idx = setting::visuals::chams::bundle_index;
					if (widgets.Combo("Bundle", &setting::visuals::chams::bundle_index, bundle_names)) {
						if (prev_idx != setting::visuals::chams::bundle_index) {
							chams::chams_mat = 0;
						}
					}

					if (chams::load_status[0]) {
						ImGui::TextDisabled("Status: %s", chams::load_status);
					}
				}
				else {
					ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No bundles found");
					if (ImGui::Button("Refresh")) {
						chams::scan_bundles(true);
					}
				}
			}
			cm.endchild();
		}
		EndGroup();
		});

	TabsManager::get().add_page(1, [&]() {
		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Environment"));
			{
				widgets.Checkbox(LangManager::get().translate("Time Changer"), &setting::visuals::time_changer);
				if (setting::visuals::time_changer) {
					widgets.SliderFloat(LangManager::get().translate("Time"), &setting::visuals::time_value, 0.0f, 24.0f, "%.1f");
				}

				Spacing();
				widgets.Checkbox(LangManager::get().translate("Bright Night"), &setting::visuals::bright_night, 0, 0, [&]() {
					widgets.ColorEdit(LangManager::get().translate("Night color"), setting::visuals::night_color);
					});

				Spacing();
				widgets.Checkbox(LangManager::get().translate("Stars Changer"), &setting::visuals::star_changer_enabled, 0, 0, [&]() {
					widgets.SliderFloat(LangManager::get().translate("Size"), &setting::visuals::star_size, 0.1f, 15.0f, "%.1f");
					widgets.SliderFloat(LangManager::get().translate("Brightness"), &setting::visuals::star_brightness, 0.1f, 15.0f, "%.1f");
					});
			}
			cm.endchild();
		}

		EndGroup();
		SameLine();

		cm.beginchild(LangManager::get().translate("Bullet tracers"));
		{
			widgets.Checkbox(LangManager::get().translate("Draw Tracers"), &setting::visuals::draw_bullet_tracers);
			color_edit3(widgets, LangManager::get().translate("Tracer Color"), setting::visuals::bullet_tracers_color);

			widgets.SliderFloat(LangManager::get().translate("Lifetime"), &setting::visuals::max_bullet_lifetime, 0.1f, 5.0f, "%.1fs");
			widgets.SliderFloat(LangManager::get().translate("Thickness"), &setting::visuals::bullets_tracers_thickness, 0.1f, 5.0f, "%.1f");
		}
		cm.endchild();

		cm.beginchild(LangManager::get().translate("Resource"));
		{
			widgets.Checkbox(LangManager::get().translate("Resource ESP"), &setting::visuals::resource_esp, 0, 0, [&]() {
				widgets.SliderFloat(LangManager::get().translate("Distance"), &setting::visuals::ore_distance, 10.0f, 500.0f, "%.0fm");

				Spacing();
				widgets.Checkbox(LangManager::get().translate("Stone"), &setting::visuals::draw_stone);
				widgets.Checkbox(LangManager::get().translate("Metal"), &setting::visuals::draw_metal);
				widgets.Checkbox(LangManager::get().translate("Sulfur"), &setting::visuals::draw_sulfur);
				widgets.Checkbox(LangManager::get().translate("Hemp"), &setting::visuals::draw_hemp);

				Spacing();
				color_edit3(widgets, LangManager::get().translate("Stone Color"), setting::visuals::stone_color);
				color_edit3(widgets, LangManager::get().translate("Metal Color"), setting::visuals::metal_color);
				color_edit3(widgets, LangManager::get().translate("Sulfur Color"), setting::visuals::sulfur_color);
				color_edit3(widgets, LangManager::get().translate("Hemp Color"), setting::visuals::hemp_color);
				});
		}
		cm.endchild();

		});

	TabsManager::get().add_page(1, [&]() {
		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Camera"));
			{
				widgets.Checkbox(LangManager::get().translate("FOV Changer"), &setting::visuals::fov_changer);
				widgets.SliderFloat(LangManager::get().translate("FOV Value"), &setting::visuals::fov_value, 70.0f, 150.0f, u8"%.0f");

				widgets.Bind(LangManager::get().translate("Zoom"), &setting::visuals::zoom_key, &setting::visuals::zoom_enable);
				widgets.SliderFloat(LangManager::get().translate("Zoom Fov"), &setting::visuals::zoom_value, 5.0f, 65.0f, u8"%.0f");
			}
			cm.endchild();
		}

		EndGroup();
		SameLine();

		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Crosshair")); {
				widgets.Checkbox(LangManager::get().translate("Enable crosshair"), &setting::visuals::crosshair, 0, 0, [&]() {

					color_edit3(widgets, LangManager::get().translate("Crosshair color"), setting::visuals::crosshair_color);
					widgets.Checkbox(LangManager::get().translate("Rainbow"), &setting::visuals::rainbow_crosshair);

					widgets.SliderFloat(LangManager::get().translate("Size"), &setting::visuals::crosshair_size, 1.0f, 20.0f, "%.1f");
					widgets.Combo(LangManager::get().translate("Type"), &setting::visuals::crosshair_type, { LangManager::get().translate("Cross"), LangManager::get().translate("Dot"), LangManager::get().translate("Rotating cross") });
					});
			}
			cm.endchild();

			cm.beginchild(LangManager::get().translate("Local Player")); {
				widgets.Checkbox(LangManager::get().translate("Custom Model"), &setting::visuals::custommodel::enabled, 0, 0, [&]() {
					if (widgets.Button(LangManager::get().translate("Browse"))) {
						custommodel::browse();
					}

					char display_path[MAX_PATH] = {};
					const char* filename = strrchr(setting::visuals::custommodel::path, '\\');
					if (!filename) filename = strrchr(setting::visuals::custommodel::path, '/');

					if (filename) strncpy_s(display_path, filename + 1, _TRUNCATE);
					else strncpy_s(display_path, setting::visuals::custommodel::path, _TRUNCATE);

					if (display_path[0]) {
						TextDisabled("%s", display_path);
					}

					if (widgets.Button(LangManager::get().translate("Load"))) {
						custommodel::load();
					}

					if (setting::visuals::custommodel::status) {
						TextDisabled("%s", setting::visuals::custommodel::status);
					}

					widgets.Checkbox(LangManager::get().translate("Rainbow"), &setting::visuals::custommodel::rainbow::enabled);
					if (setting::visuals::custommodel::rainbow::enabled) {
						widgets.SliderFloat(LangManager::get().translate("Opacity"), &setting::visuals::custommodel::rainbow::opacity, 0.0f, 1.0f, "%.2f");
					}
					});

				Spacing();
				widgets.Checkbox(LangManager::get().translate("Reload Indicator"), &setting::misc::reload_indicator);
			}
			cm.endchild();
		}
		EndGroup();
		});

	TabsManager::get().add_page(2, [&]() {
		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Weapon"));
			{
				widgets.Checkbox(LangManager::get().translate("No Recoil"), &setting::misc::no_recoil);
				widgets.Checkbox(LangManager::get().translate("No Spread"), &setting::misc::no_spread);
				widgets.Checkbox(LangManager::get().translate("No Sway"), &setting::misc::no_sway);
				widgets.Checkbox(LangManager::get().translate("Insta Eoka"), &setting::misc::insta_eoka);
				widgets.Checkbox(LangManager::get().translate("Silent Reload"), &setting::misc::silent_reload);
			}
			cm.endchild();

			EndGroup();
			SameLine();

			cm.beginchild(LangManager::get().translate("Bullet Mods"));
			{
				widgets.Checkbox(LangManager::get().translate("Wall Shot"), &setting::misc::wall_shot);
				widgets.Checkbox(LangManager::get().translate("Bullet TP"), &setting::misc::bullet_tp);
			}
			cm.endchild();
		}
		EndGroup();
		});

	TabsManager::get().add_page(3, [&]() {
		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Movement"));
			{
				widgets.Checkbox(LangManager::get().translate("Infinite Jump"), &setting::movement::infinite_jump);
				widgets.Checkbox(LangManager::get().translate("No Mini Sprint"), &setting::movement::no_mini_sprint);

				ImGui::Spacing();
				widgets.Bind(LangManager::get().translate("Spider"), &setting::movement::spider_key, &setting::movement::enable_spider);
				widgets.Bind(LangManager::get().translate("Air Stack"), &setting::movement::airStack_key, &setting::movement::enable_airStack);
				widgets.Bind(LangManager::get().translate("Tp To Head"), &setting::movement::tp_to_head_key, &setting::movement::enable_tp_to_head);

				if (setting::movement::no_mini_sprint) {
					ImGui::Spacing();

					widgets.Bind(LangManager::get().translate("Speed Hack"), &setting::movement::speed_key, &setting::movement::enable_speedHack);
					widgets.SliderFloat(LangManager::get().translate("Speed Value"), &setting::movement::speed_value, 6.0f, 15.0f, "%.1f");
				}
			}
			cm.endchild();

			EndGroup();
			SameLine();

			cm.beginchild(LangManager::get().translate("Anti-Aim"));
			{
				widgets.Checkbox(LangManager::get().translate("Anti Aim"), &setting::movement::anti_aim);
			}
			cm.endchild();
		}
		EndGroup();
		});

	TabsManager::get().add_page(3, [&]() {
		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Player"));
			{
				widgets.Checkbox(LangManager::get().translate("Insta Pickup Wounded"), &setting::movement::insta_pickup_player);
				widgets.Checkbox(LangManager::get().translate("Can Wield Items"), &setting::movement::can_wield_items);
				widgets.Checkbox(LangManager::get().translate("Fast Loot"), &setting::movement::fast_loot);
			}
			cm.endchild();

			cm.beginchild(LangManager::get().translate("Sound"));
			{
				widgets.Checkbox(LangManager::get().translate("Hit Sound"), &setting::menu::hit_sound);
				widgets.SliderInt(LangManager::get().translate("All Sounds Volume"), &setting::misc::sound_volume, 0, 1000, "%d");
			}
			cm.endchild();
		}
		EndGroup();

		SameLine();

		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Unload")); {
				widgets.Separator();

				ImColor red;
				float h, s, v;
				ColorConvertRGBtoHSV(GetStyleColorVec4(ImGuiCol_Scheme).x, GetStyleColorVec4(ImGuiCol_Scheme).y, GetStyleColorVec4(ImGuiCol_Scheme).z, h, s, v);
				ColorConvertHSVtoRGB(0.0f, s, v, red.Value.x, red.Value.y, red.Value.z);
				red.Value.w = GImGui->Style.Alpha;

				PushStyleColor(ImGuiCol_Button, red.Value);
				PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ red.Value.x / 1.15f, red.Value.y / 1.15f, red.Value.z / 1.15f, GImGui->Style.Alpha });
				PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ red.Value.x / 1.35f, red.Value.y / 1.35f, red.Value.z / 1.35f, GImGui->Style.Alpha });
				if (widgets.Button(LangManager::get().translate("Unload cheat"), { CalcItemWidth(), 0 })) {
					setting::menu::exit = true;
					NotifyManager::get().add(LangManager::get().translate("Unloading..."), NotifyManager::notify_info);
				}
				PopStyleColor(3);
			}
			cm.endchild();
		}
		EndGroup();
		});

	TabsManager::get().add_page(4, [&]() {
		static int cfg = 0;
		static char cfg_name[64] = "default";
		static std::vector<std::string> configs = list_configs();
						cfg = configs.empty() ? 0 : std::clamp(cfg, 0, static_cast<int>(configs.size()) - 1);

		BeginGroup();
		{
			PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
			BeginChild("configs", { GetWindowWidth() / 2 - 14 - GImGui->Style.ItemSpacing.x / 2, 0 }, ImGuiChildFlags_Border);
			{
				SetCursorPos({ 14, 14 });
				PushItemFlag(ImGuiItemFlags_NoNav, true);
				widgets.TextField("##name", cfg_name, sizeof(cfg_name), { GetWindowWidth() - 28 - GetFrameHeight() - 10, GetFrameHeight() }, LangManager::get().translate("Config name..."), i_tag_01);
				PopItemFlag();
				SameLine(0, 10);
				if (CompBuilder::get().Button("add", { GetFrameHeight(), GetFrameHeight() }, [&](CompBuilder::ButtonEnv env) {
					auto col = col_anim(col_anim(GetColorU32(ImGuiCol_Button), GetColorU32(ImGuiCol_ButtonHovered), env.anim.hover), GetColorU32(ImGuiCol_ButtonActive), env.anim.held);
					GetWindowDrawList()->AddRectFilled(env.bb.Min, env.bb.Max, col, 2);
					GetWindowDrawList()->AddText(fonts[icons].get(14), 14, env.bb.GetCenter() - ImVec2{ 7, 7 }, GetColorU32(ImGuiCol_TextButton), i_plus);
					})) {
					if (strlen(cfg_name) == 0) {
						NotifyManager::get().add(LangManager::get().translate("Enter config name!"), NotifyManager::notify_error);
					}
					else {
						setting::config::save(cfg_name);
						configs = list_configs();
						cfg = configs.empty() ? 0 : std::clamp(cfg, 0, static_cast<int>(configs.size()) - 1);
						NotifyManager::get().add(LangManager::get().translate("Config saved"), NotifyManager::notify_success);
					}
				}

				Separator();
				SetCursorPosY(GetCursorPosY() - GImGui->Style.ItemSpacing.y);

				PushStyleVar(ImGuiStyleVar_WindowPadding, { 14, 14 });
				BeginChild("list", { 0, 0 }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					cm.smoothscroll();

					if (configs.empty()) {
						TextDisabled(LangManager::get().translate("No configs found"));
					}

					for (int i = 0; i < static_cast<int>(configs.size()); ++i) {
						const char* label = configs[i].c_str();
						ImRect bb{ GetCurrentWindow()->DC.CursorPos, GetCurrentWindow()->DC.CursorPos + ImVec2{ GetWindowWidth() - GImGui->Style.WindowPadding.x * 2, GetFrameHeight() } };
						if (CompBuilder::get().Selectable(label, cfg == i, bb, [&](const CompBuilder::SelectableEnv& env) {
							ImColor col = col_anim(col_anim(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_TextHovered), env.anim.hover), GetColorU32(ImGuiCol_Text), env.anim.selected);
							ImColor icon_col = col_anim(col_anim(GetColorU32(ImGuiCol_TextDisabled), GetColorU32(ImGuiCol_TextHovered), env.anim.hover), GetColorU32(ImGuiCol_Scheme), env.anim.selected);

							GetWindowDrawList()->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, env.anim.selected), GImGui->Style.FrameRounding);
							GetWindowDrawList()->AddText(fonts[icons].get(14), 14, bb.Min + GImGui->Style.FramePadding, icon_col, i_file_04);
							GetWindowDrawList()->AddText(bb.Min + GImGui->Style.FramePadding + ImVec2{ 24, 0 }, col, label);
							})) {
							cfg = i;
							strncpy_s(cfg_name, configs[i].c_str(), _TRUNCATE);
						}
					}
				}
				EndChild();
				PopStyleVar();
			}
			EndChild();
			PopStyleVar();
		}
		EndGroup();

		SameLine();

		BeginGroup();
		{
			cm.beginchild(LangManager::get().translate("Config actions"));
			{
				const char* selected_name = configs.empty() ? cfg_name : configs[cfg].c_str();

				TextDisabled(LangManager::get().translate("Name:"));
				SameLine(GetWindowWidth() - CalcTextSize(selected_name).x - GImGui->Style.WindowPadding.x);
				TextUnformatted(selected_name);

				widgets.Separator();

				if (widgets.Button(LangManager::get().translate("Save"))) {
					if (configs.empty() && strlen(cfg_name) == 0) {
						NotifyManager::get().add(LangManager::get().translate("No config selected!"), NotifyManager::notify_error);
					}
					else {
						std::string target_name = configs.empty() ? cfg_name : configs[cfg];
						setting::config::save(target_name);
						configs = list_configs();
						cfg = configs.empty() ? 0 : std::clamp(cfg, 0, static_cast<int>(configs.size()) - 1);
						NotifyManager::get().add(LangManager::get().translate("Config saved"), NotifyManager::notify_success);
					}
				}

				if (widgets.Button(LangManager::get().translate("Load"))) {
					if (configs.empty() && strlen(cfg_name) == 0) {
						NotifyManager::get().add(LangManager::get().translate("No config selected!"), NotifyManager::notify_error);
					}
					else {
						setting::config::load(configs.empty() ? cfg_name : configs[cfg]);
						NotifyManager::get().add(LangManager::get().translate("Config loaded"), NotifyManager::notify_info);
					}
				}

				ImColor red;
				float h, s, v;
				ColorConvertRGBtoHSV(GetStyleColorVec4(ImGuiCol_Scheme).x, GetStyleColorVec4(ImGuiCol_Scheme).y, GetStyleColorVec4(ImGuiCol_Scheme).z, h, s, v);
				ColorConvertHSVtoRGB(0.f, s, v, red.Value.x, red.Value.y, red.Value.z);
				red.Value.w = GImGui->Style.Alpha;

				PushStyleColor(ImGuiCol_Button, red.Value);
				PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ red.Value.x / 1.15f, red.Value.y / 1.15f, red.Value.z / 1.15f, GImGui->Style.Alpha });
				PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ red.Value.x / 1.35f, red.Value.y / 1.35f, red.Value.z / 1.35f, GImGui->Style.Alpha });
				if (widgets.Button(LangManager::get().translate("Delete"))) {
					if (configs.empty()) {
						NotifyManager::get().add(LangManager::get().translate("No config to delete!"), NotifyManager::notify_error);
					}
					else {
						std::filesystem::remove(setting::config::get_path(configs[cfg]));
						configs = list_configs();
						cfg = configs.empty() ? 0 : std::clamp(cfg, 0, static_cast<int>(configs.size()) - 1);
						cfg = 0;
						if (!configs.empty())
							strncpy_s(cfg_name, configs[0].c_str(), _TRUNCATE);
						NotifyManager::get().add(LangManager::get().translate("Config deleted"), NotifyManager::notify_info);
					}
				}
				PopStyleColor(3);

				widgets.Separator();

				if (widgets.Button(LangManager::get().translate("Refresh list"))) {
					configs = list_configs();
						cfg = configs.empty() ? 0 : std::clamp(cfg, 0, static_cast<int>(configs.size()) - 1);
					NotifyManager::get().add(LangManager::get().translate("List refreshed"), NotifyManager::notify_info);
				}

				if (widgets.Button(LangManager::get().translate("Open Dir"))) {
					const auto dir = std::filesystem::temp_directory_path() / L"data";
					std::filesystem::create_directories(dir);
					ShellExecuteW(NULL, L"open", dir.wstring().c_str(), NULL, NULL, SW_SHOW);
					NotifyManager::get().add(LangManager::get().translate("Directory opened"), NotifyManager::notify_info);
				}
			}
			cm.endchild();
		}
		EndGroup();
		});

	initialized = true;
}

void GUI::draw() {
	SetNextWindowSize(size);
	Begin("GUI", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus); {
		BeginChild("navbar", { 180, 0 }, 0, ImGuiWindowFlags_NoBackground);
		{
			// Vector emblem: no image downloads or extra font glyphs required.
			auto* brand = GetWindowDrawList();
			const ImVec2 origin = GetWindowPos();
			const ImU32 blue = GetColorU32(ImGuiCol_Scheme);
			brand->AddRectFilled(origin + ImVec2{ 14, 12 }, origin + ImVec2{ 166, 15 }, blue);
			brand->AddRectFilled(origin + ImVec2{ 14, 70 }, origin + ImVec2{ 166, 73 }, blue);
			const ImVec2 center = origin + ImVec2{ 35, 43 };
			brand->AddTriangle(center + ImVec2{ 0, -17 }, center + ImVec2{ 14.722f, 8.5f }, center + ImVec2{ -14.722f, 8.5f }, blue, 2.0f);
			brand->AddTriangle(center + ImVec2{ 0, 17 }, center + ImVec2{ -14.722f, -8.5f }, center + ImVec2{ 14.722f, -8.5f }, blue, 2.0f);
			brand->AddText(origin + ImVec2{ 60, 28 }, blue, "ISRAEL");
			brand->AddText(origin + ImVec2{ 60, 47 }, GetColorU32(ImGuiCol_TextDisabled), "BLUE / WHITE");
			SetCursorPos({ 14, 88 });
			PushStyleColor(ImGuiCol_FrameBg, GetColorU32(ImGuiCol_FrameBg, 0.f));
			PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
			PushItemFlag(ImGuiItemFlags_NoNav, true);
			if (WidgetsManager::get().TextField("##search", SearchManager::get().search_buf, sizeof(SearchManager::get().search_buf), { GetWindowWidth() - 28, 0 }, LangManager::get().translate("Search..."), i_search_md)) {
				SearchManager::get().update();
			}
			PopItemFlag();
			PopStyleVar();
			PopStyleColor();
			WidgetsManager::get().Separator();
			SetCursorPosX(14);
			TabsManager::get().render_tabs(8);

			GetWindowDrawList()->AddRectFilled(GetWindowPos() + ImVec2{ GetWindowWidth() - 1, 0 }, GetWindowPos() + GetWindowSize(), GetColorU32(ImGuiCol_Border));

			SetCursorPos({ 14, GetWindowHeight() - 38 });
			CompBuilder::get().OpenButton("lang selector", { 32 + fonts[font].get(12)->CalcTextSizeA(12, FLT_MAX, -1, LangManager::get().get_lang_name()).x, 24 }, [&](CompBuilder::OpenButtonEnv env) {
				GetWindowDrawList()->AddRectFilled(env.bb.Min, env.bb.Max, GetColorU32(ImGuiCol_ChildBg), GImGui->Style.FrameRounding);
				GetWindowDrawList()->AddText(fonts[icons].get(12), 12, { env.bb.Min.x + 6, env.bb.GetCenter().y - 6 }, GetColorU32(ImGuiCol_Scheme), i_translate_02);
				GetWindowDrawList()->AddText(fonts[font].get(12), 12, { env.bb.Min.x + 26, env.bb.GetCenter().y - 6 }, GetColorU32(ImGuiCol_Text), LangManager::get().get_lang_name());

				// OpenButton already toggles the open state on click.

				if (env.hovered) env.open = true;

				if (env.anim.open > 0.05f) {
					PushStyleVar(ImGuiStyleVar_Alpha, env.anim.open);
					PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
					PushStyleVar(ImGuiStyleVar_WindowRounding, GImGui->Style.FrameRounding);
					PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 8 });
					PushStyleVar(ImGuiStyleVar_FramePadding, { 10, 8 });
					PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
					PushStyleColor(ImGuiCol_WindowBg, GetColorU32(ImGuiCol_FrameBg));
					Begin("lang selector", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
					{
						SetWindowPos({ env.bb.Min.x, env.bb.GetCenter().y - GetWindowHeight() / 2 });
						SetWindowSize({ 100, (GetCurrentWindow()->ContentSize.y + GImGui->Style.WindowPadding.y * 2) * env.anim.open });

						BringWindowToDisplayFront(GetCurrentWindow());
						BringWindowToFocusFront(GetCurrentWindow());

						if (!IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && !env.hovered) {
							env.open = false;
						}

						PushStyleColor(ImGuiCol_FrameBg, GetColorU32(ImGuiCol_FrameBgHovered));

						for (int i = 0; i < LangManager::get().get_langs().size(); ++i) {
							if (WidgetsManager::get().Selectable(LangManager::get().get_langs()[i].label, LangManager::get().get_lang() == i)) {
								LangManager::get().set_lang(i);
							}
						}

						PopStyleColor();
					}
					End();
					PopStyleColor();
					PopStyleVar(6);
				}
				});
		}
		EndChild();

		SameLine(0, 0);

		if (strlen(SearchManager::get().search_buf) > 0 || SearchManager::get().get_anim() > 0.05f) PushStyleVar(ImGuiStyleVar_WindowPadding, { 14, 14 });
		BeginChild("main", { 0, 0 }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			if (strlen(SearchManager::get().search_buf) > 0 || SearchManager::get().get_anim() > 0.05f) PopStyleVar();

			ChildManager::get().smoothscroll();

			if (strlen(SearchManager::get().search_buf) == 0 && SearchManager::get().get_anim() < 0.05f) {
				TabsManager::get().render_subtabs(20);

				auto window = GetCurrentWindow();

				PushStyleVar(ImGuiStyleVar_Alpha, GImGui->Style.Alpha * TabsManager::get().get_anim() * GImGui->Style.Alpha);
				PushStyleVar(ImGuiStyleVar_WindowPadding, { 14, 14 });
				BeginChild("content", { 0, 0 }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					TabsManager::get().draw_page(window);
				}
				EndChild();
				PopStyleVar(2);
			}
			else {
				SearchManager::get().draw();
			}
		}
		EndChild();

		PopupManager::get().handle();
	}
	End();

	NotifyManager::get().draw();
}
