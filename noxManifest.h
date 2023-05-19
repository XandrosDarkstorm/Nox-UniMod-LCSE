#pragma once
#include <vector>
// This list tracks our Unimod modules. Use announceCapability() function to add to this list
std::vector<std::string> modCapabilities;
// This list tracks user loaded modules.
std::vector<std::string> userLuaCapabilities;
/// <summary>
/// This function returns a table, which acts as a client manifest.
/// This table can be used by scripts to differentiate various Nox mods and its versions.
/// </summary>
int noxClientInfoL(lua_State* L);
/// <summary>
/// Appends capability / module to the client manifest.
/// </summary>
/// <param name="capability_name">Capability name. Please, prefer concise and meaningfull names.</param>
void announceCapability(const char* capability_name);
int declareLuaCapabilityL(lua_State* L);
/// <summary>
/// Return a table with capabilities announced by the Lua environment.
/// </summary>
int listLuaCapabilitiesL(lua_State* L);

/// <summary>
/// Initializatize manifest module.
/// </summary>
void initManifest();