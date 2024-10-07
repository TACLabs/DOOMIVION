#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"

#include "obse/GameObjects.h"

#include "obse/Utilities.h"

#include <Mmsystem.h>
#include "Windows.h"
#include <TlHelp32.h>

#include "GameplayTweaks.h"
#include "MemoryFunctions.h"
#include "Tambouille.h"

#include "Dash.h"
#include "DoubleJump.h"

extern "C" {

	bool OBSEPlugin_Query(const OBSEInterface* obse, PluginInfo* info)
	{
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Doomivion";
		info->version = 1;

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface* pluginInterface)
	{
		if (!pluginInterface->isEditor)
		{
			Tambouille::Init();
			SoundsManager::Init(pluginInterface->GetOblivionDirectory());
			GameplayTweaks::Init();
			DoubleJump::Init();
			Dash::Init();
		}

		return true;
	}


}