#include "stdafx.h"
#include "noxManifest.h"

int noxClientInfoL(lua_State* L)
{
	lua_createtable(L, 3, 0);
	lua_pushstring(L, MODNAME);
	lua_setfield(L, -2, "name");
	lua_pushstring(L, MODVERSION);
	lua_setfield(L, -2, "version");
	lua_createtable(L, 0, modCapabilities.size());
	for (int i = 0; i < modCapabilities.size(); ++i)
	{
		lua_pushnumber(L, i + 1);
		lua_pushstring(L, modCapabilities[i].c_str());
		lua_rawset(L, -3);
	}
	lua_setfield(L, -2, "base_capabilities");
	return 1;
}

void announceCapability(const char* capability_name)
{
	modCapabilities.push_back(capability_name);
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

