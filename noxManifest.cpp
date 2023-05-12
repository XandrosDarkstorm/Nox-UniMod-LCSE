#include "stdafx.h"
#include <vector>

// This list tracks our Unimod modules. Use announceCapability() function to add to this list
std::vector<std::string> unimodCapabilities;
// This list tracks user loaded modules.
std::vector<std::string> userLuaCapabilities;
/// <summary>
/// This function returns a table, which acts as a client manifest.
/// This table can be used by scripts to differentiate various Nox mods and its versions.
/// </summary>
int noxClientInfoL(lua_State* L)
{
	lua_createtable(L, 3, 0);
	lua_pushstring(L, MODNAME);
	lua_setfield(L, -2, "name");
	lua_pushstring(L, MODVERSION);
	lua_setfield(L, -2, "version");
	lua_createtable(L, 0, unimodCapabilities.size());
	for (int i = 0; i < unimodCapabilities.size(); ++i)
	{
		conPrintI(unimodCapabilities[i].c_str());
		lua_pushnumber(L, i + 1);
		lua_pushstring(L, unimodCapabilities[i].c_str());
		lua_rawset(L, -3);
	}
	lua_setfield(L, -2, "base_capabilities");
	return 1;
}

/// <summary>
/// Appends capability / module to the client manifest.
/// </summary>
/// <param name="capability_name">Capability name. Please, prefer concise and meaningfull names.</param>
void announceCapability(const char* capability_name)
{
	unimodCapabilities.push_back(capability_name);
}

int declareLuaCapabilityL(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TSTRING)
	{
		lua_pushstring(L, "[announceLuaCapability,type_arg1] -> argument 1 requires a string.");
		lua_error(L);
		return 0;
	}
	std::string cap_name = lua_tostring(L, 1);
	for (int i = 0; i < userLuaCapabilities.size(); ++i)
	{
		if (userLuaCapabilities[i] == cap_name)
		{
			lua_pushstring(L, "[announceLuaCapability,already_exist] -> this capability already announced.");
			lua_error(L);
			lua_pushboolean(L, 0);
			return 1;
		}
	}
	userLuaCapabilities.push_back(cap_name.c_str());
	lua_pushboolean(L, 1);
	return 1;
}

/// <summary>
/// Return a table with capabilities announced by the Lua environment.
/// </summary>
int listLuaCapabilitiesL(lua_State* L)
{
	lua_createtable(L, 0, userLuaCapabilities.size());
	for (int i = 0; i < userLuaCapabilities.size(); ++i)
	{
		lua_pushstring(L, userLuaCapabilities[i].c_str());
		lua_settable(L, -2);
	}
	return 1;
}

void initManifest()
{
	registerclient("noxClientInfo", &noxClientInfoL);
	registerserver("declareLuaCapability", &declareLuaCapabilityL);
	registerserver("listLuaCapabilities", &listLuaCapabilitiesL);
	announceCapability("manifest");
}

