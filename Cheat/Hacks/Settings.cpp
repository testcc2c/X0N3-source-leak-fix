#include "Settings.h"
#include "Setup.h"
#include "../GUI/Gui.h"

//bool isDirectoryExists(const char *filename)
//{
//	DWORD dwFileAttributes = FastCall::G().t_GetFileAttributesA(filename);
//	if (dwFileAttributes == 0xFFFFFFFF)
//		return false;
//	return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
//}

void CGSettings::SaveEx(nlohmann::json &j)
{
	if (GP_Esp)
	{
		GP_Esp->Config.Save(j);
		GP_Esp->SaveVisuals(j);
	}

	if (GP_GHelper)
		GP_GHelper->Config.Save(j);
#ifdef ENABLE_INVENTORY
	if (GP_Inventory)
	{
		GP_Inventory->Config.Save(j);
		GP_Inventory->SaveInventory(j);
	}
#endif
	if (GP_LegitAim)
	{
		GP_LegitAim->Config.Save(j);
		GP_LegitAim->SaveWeapons(j);
	}

	if (GP_Misc)
		GP_Misc->Config.Save(j);

	if (GP_Radar)
		GP_Radar->Config.Save(j);

	if (GP_Skins)
	{
		GP_Skins->Config.Save(j);
		GP_Skins->SaveSkins(j);
	}

	if (GP_Main)
		GP_Main->Config.Save(j);
}

void CGSettings::LoadEx(nlohmann::json &j)
{
	if (GP_Esp)
	{
		GP_Esp->Config.Load(j);
		GP_Esp->LoadVisuals(j);
	}

	if (GP_GHelper)
		GP_GHelper->Config.Load(j);
#ifdef ENABLE_INVENTORY
	if (GP_Inventory)
	{
		GP_Inventory->Config.Load(j);
		GP_Inventory->LoadInventory(j);
	}
#endif
	if (GP_LegitAim)
	{
		GP_LegitAim->Config.Load(j);
		GP_LegitAim->LoadWeapons(j);
	}

	if (GP_Misc)
		GP_Misc->Config.Load(j);

	if (GP_Radar)
		GP_Radar->Config.Load(j);

	if (GP_Skins)
	{
		GP_Skins->Config.Load(j);
		GP_Skins->LoadSkins(j);
	}

	if (GP_Main)
		GP_Main->Config.Load(j);
}

bool CGSettings::Save(string name)
{
	nlohmann::json JFile;

	SaveEx(JFile);

	std::ofstream o(config_name);

	if (!o)
	{
		o.clear();
		JFile.clear();
		return false;
	}

	o << std::setw(4) << JFile << std::endl;

	JFile.clear();
	return true;
}

bool CGSettings::Load()
{
	string JCont = readFile(config_name);

	if (JCont == string(XorStr("Read Error")))
		return false;

	nlohmann::json JFile = nlohmann::json::parse(JCont);

	LoadEx(JFile);

	JFile.clear();
#ifdef ENABLE_INVENTORY
	GP_Inventory->SendClientHello();
#endif
	GP_Skins->UpdateSkins();

	return true;
}

void CGSettings::UpdateColors()
{
	if (GP_Esp)
		GP_Esp->Config.UpdateColors();

	if (GP_LegitAim)
		GP_LegitAim->Config.UpdateColors();

	if (GP_Misc)
		GP_Misc->Config.UpdateColors();

	if (GP_Radar)
		GP_Radar->Config.UpdateColors();

	if (GP_Main)
		GP_Main->Config.UpdateColors();
}

void CGSettings::SetName(const string _name)
{
	config_name = _name;
}

void CGSettings::UpdateList()
{
	AllSettings.clear();
	CGlobal::CreateFolderCFG();
	CGlobal::RefreshConfigs();

	if (CGlobal::ConfigList.size())
	{
		for (auto &v : CGlobal::ConfigList)
		{
			CfgInfo_s cfg_entry;
			cfg_entry.name = v;
			cfg_entry.id = "";
			AllSettings.push_back(cfg_entry);
		}
	}

	if (!AllSettings.empty())
	{
		vector<CfgInfo_s> TempSettings = AllSettings;
		vector<int> skip_idx;
		AllSettings.clear();

		for (int i(0); i < (int)TempSettings.size(); i++)
		{
			bool cont = false;
			for (int j(0); j < (int)skip_idx.size(); j++)
			{
				if (skip_idx[j] == i)
				{
					cont = true;
					break;
				}
			}
			if (cont)
				continue;
			AllSettings.push_back(TempSettings[i]);
		}
	}
}

void CGSettings::Menu()
{
	static int ConfigSelect = 0;
	static char NewConfigName[64] = { 0 };

	float long_item_w = X1Gui().GetThis()->Info.Size.x - (X1Gui().GetStyle().wndPadding.x * 2);

	if (X1Gui().ListBoxHeader(XorStr("##0"), Vec2(long_item_w, 250)))
	{
		X1Gui().Spacing();
		X1Gui().Separator();
		X1Gui().Text(XorStr("Name"));
		X1Gui().Separator();

		for (int i = 0; i < (int)AllSettings.size(); i++)
		{
			bool selected = i == ConfigSelect;
			if (X1Gui().SelectLabel(AllSettings[i].name.c_str(), selected, Vec2(225, 19)))
				ConfigSelect = i;
			X1Gui().Separator();
		}
		X1Gui().VerticalSeparator(230.f);
	}
	X1Gui().ListBoxFooter();

	X1Gui().Spacing();
	X1Gui().Separator();
	X1Gui().Spacing();

	X1Gui().PushItemWidth(300.f);
	X1Gui().InputText(XorStr("##Name"), NewConfigName, 28);
	X1Gui().SameLine();

	string SettignBaseDir = CGlobal::SystemDisk + XorStr("X1N3\\Configurations\\");
	string SettingsFormat = XorStr(".json");

	auto CheckRus = [&](string& check)-> bool
	{
		string alph_rus = XorStr("АаБбВвГгДдЕеЁёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯя");

		for (auto& v_check : check)
			for (auto& v_rus : alph_rus)
				if (v_check == v_rus)
					return true;

		return false;
	};

	if (X1Gui().Button(XorStr("Create & save new config")))
	{
		if (!CheckRus(string(NewConfigName)))
		{
			string ConfigFileName = NewConfigName;

			if (ConfigFileName.size() < 1)
			{
				ConfigFileName = XorStr("Settings");
			}

			SetName(SettignBaseDir + ConfigFileName + SettingsFormat);

			if (Save(ConfigFileName + SettingsFormat))
				Message::Get().Start(XorStr("Config saved"));
			else
				Message::Get().Start(XorStr("Save error"));

			UpdateList();
			strcpy(NewConfigName, "");
		}
	}
	X1Gui().Spacing();
	X1Gui().Separator();
	X1Gui().Spacing();

	if (X1Gui().Button(XorStr("Update configs list"), Vec2(long_item_w, 0)))
	{
		UpdateList();
		Message::Get().Start(XorStr("List updated"));
	}

	X1Gui().Spacing();
	X1Gui().Separator();
	X1Gui().Spacing();

	if (X1Gui().Button(XorStr("Load config"), Vec2(long_item_w, 0)))
	{
		if (ConfigSelect >= 0 && ConfigSelect < (int)AllSettings.size())
		{
			SetName(SettignBaseDir + AllSettings[ConfigSelect].name);
			if (Load())
				Message::Get().Start(XorStr("Config loaded"));
			else
				Message::Get().Start(XorStr("Load error"));
		}
	}
	X1Gui().Spacing();
	if (X1Gui().Button(XorStr("Save config"), Vec2(long_item_w, 0)))
	{
		if (ConfigSelect >= 0 && ConfigSelect < (int)AllSettings.size())
		{
			SetName(SettignBaseDir + AllSettings[ConfigSelect].name);
			if (Save(AllSettings[ConfigSelect].name))
			{
				UpdateList();
				Message::Get().Start(XorStr("Config saved"));
			}
			else
				Message::Get().Start(XorStr("Save error"));
		}
	}
	X1Gui().Spacing();
	X1Gui().Separator();
	X1Gui().Spacing();
	if (X1Gui().Button(XorStr("Delete config"), Vec2(long_item_w, 0)))
	{
		if (ConfigSelect >= 0 && ConfigSelect < (int)AllSettings.size())
		{
			string FulDel = SettignBaseDir + AllSettings[ConfigSelect].name;

			if (AllSettings.size() > 0)
			{
				FastCall::G().t_DeleteFileA(FulDel.c_str());
			}

			UpdateList();
			Message::Get().Start(XorStr("Config deleted"));
		}
	}

	X1Gui().Spacing();
	X1Gui().Separator();
	X1Gui().Spacing();
}

string CGSettings::readFile(const string& fileName)
{
	ifstream f(fileName);
	if (!f)
		return (XorStr("Read error"));
	stringstream ss;
	ss << f.rdbuf();
	return ss.str();
}

void CMain::SetMenuColors()
{
	GuiStyle& style = X1Gui().GetStyle();

	auto AutoChangeColor = [&](Color col, float ch) -> Color
	{
		Color entry;

		entry.SetR(((col.r() + ch < 0) ? 0 : ((col.r() + ch > 255) ? 255 : (col.r() + ch))));
		entry.SetG(((col.g() + ch < 0) ? 0 : ((col.g() + ch > 255) ? 255 : (col.g() + ch))));
		entry.SetB(((col.b() + ch < 0) ? 0 : ((col.b() + ch > 255) ? 255 : (col.b() + ch))));
		entry.SetA(col.a());

		return entry;
	};

	style.clrLine = LineColor;
	style.clrBackground = BackgroundColor;
	style.clrText = TextColor;

	style.clrFrame = FrameColor;
	style.clrFrameHover = AutoChangeColor(FrameColor, -5);
	style.clrFrameHold = AutoChangeColor(FrameColor, -9);

	style.clrTabLabel = color_t(ButtonColor.r(), ButtonColor.g(), ButtonColor.b(), 120);
	style.clrTabLabelText = color_t(TextColor.r(), TextColor.g(), TextColor.b(), 160);

	style.clrButton = ButtonColor;
	style.clrButtonHover = AutoChangeColor(ButtonColor, -10);
	style.clrButtonHold = AutoChangeColor(ButtonColor, -20);

	style.clrDisButton = DisableButtonColor;
	style.clrDisButtonHover = AutoChangeColor(DisableButtonColor, -10);
	style.clrDisButtonHold = AutoChangeColor(DisableButtonColor, -20);

	style.clrScroll = AutoChangeColor(ButtonColor, -20);
	style.clrScrollHover = AutoChangeColor(ButtonColor, -20);
	style.clrScrollHold = AutoChangeColor(ButtonColor, -20);
	style.clrTitle = TitleColor;
}

void CConfig::Save(nlohmann::json &j)
{
	for (auto &v : VecBool)
		j[SectionName][v.Key] = v.Var;

	for (auto &v : VecInt)
		j[SectionName][v.Key] = v.Var;

	for (auto &v : VecFloat)
		j[SectionName][v.Key] = v.Var;

	for (auto &v : VecDouble)
		j[SectionName][v.Key] = v.Var;

	for (auto &v : VecString)
		j[SectionName][v.Key] = v.Var;

	for (auto &v : VecColor)
	{
		j[SectionName][v.Key][XorStr("R")] = v.Var.r();
		j[SectionName][v.Key][XorStr("G")] = v.Var.g();
		j[SectionName][v.Key][XorStr("B")] = v.Var.b();
		j[SectionName][v.Key][XorStr("A")] = v.Var.a();
		j[SectionName][v.Key][XorStr("Rainbow")] = v.Var.brainbow;
		j[SectionName][v.Key][XorStr("flRainbow")] = v.Var.rainbow;
	}

	for (auto &v : VecBind)
	{
		j[SectionName][v.Key][XorStr("Enable")] = v.Var.Enable;
		j[SectionName][v.Key][XorStr("Button")] = v.Var.Button;
		j[SectionName][v.Key][XorStr("Hold")] = v.Var.Hold;
	}

	ADD_LOG("Settings: %s Saved\n", SectionName.c_str());
}

void CConfig::Load(nlohmann::json j)
{
	if (!j[SectionName].is_null())
	{
		for (auto &v : VecBool)
			if (!j[SectionName][v.Key].is_null())
				v.Var = j[SectionName][v.Key];

		for (auto &v : VecInt)
			if (!j[SectionName][v.Key].is_null())
				v.Var = j[SectionName][v.Key];

		for (auto &v : VecFloat)
			if (!j[SectionName][v.Key].is_null())
				v.Var = j[SectionName][v.Key];

		for (auto &v : VecDouble)
			if (!j[SectionName][v.Key].is_null())
				v.Var = j[SectionName][v.Key];

		for (auto &v : VecString)
			if (!j[SectionName][v.Key].is_null())
				v.Var = j[SectionName][v.Key].get<string>();

		for (auto &v : VecColor)
		{
			if (!j[SectionName][v.Key].is_null())
			{
				if (!j[SectionName][v.Key][XorStr("R")].is_null())
					v.Var[0] = j[SectionName][v.Key][XorStr("R")];

				if (!j[SectionName][v.Key][XorStr("G")].is_null())
					v.Var[1] = j[SectionName][v.Key][XorStr("G")];

				if (!j[SectionName][v.Key][XorStr("B")].is_null())
					v.Var[2] = j[SectionName][v.Key][XorStr("B")];

				if (!j[SectionName][v.Key][XorStr("A")].is_null())
					v.Var[3] = j[SectionName][v.Key][XorStr("A")];

				if (!j[SectionName][v.Key][XorStr("Rainbow")].is_null())
					v.Var.brainbow = j[SectionName][v.Key][XorStr("Rainbow")];

				if (!j[SectionName][v.Key][XorStr("flRainbow")].is_null())
					v.Var.rainbow = j[SectionName][v.Key][XorStr("flRainbow")];
			}
		}

		for (auto &v : VecBind)
		{
			if (!j[SectionName][v.Key].is_null())
			{
				if (!j[SectionName][v.Key][XorStr("Enable")].is_null())
					v.Var.Enable = j[SectionName][v.Key][XorStr("Enable")];

				if (!j[SectionName][v.Key][XorStr("Button")].is_null())
					v.Var.Button = j[SectionName][v.Key][XorStr("Button")];

				if (!j[SectionName][v.Key][XorStr("Hold")].is_null())
					v.Var.Hold = j[SectionName][v.Key][XorStr("Hold")];
			}
		}
		ADD_LOG("Settings: %s Loaded\n", SectionName.c_str());
	}
}

void CConfig::UpdateColors()
{
	for (auto &v : VecColor)
		if (v.Var.brainbow)
			v.Var.UpdateRainbow(1.f, 1.f);
}



























































