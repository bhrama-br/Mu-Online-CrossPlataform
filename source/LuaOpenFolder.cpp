#include "stdafx.h"
#include "LuaOpenFolder.h"
#include "LuaDecrypt.h"

static const int sentinel_ = 0;
#define sentinel ((void *)&sentinel_)

CLuaOpenFolder gLuaOpenFolder;

CLuaOpenFolder::CLuaOpenFolder()
{
}

CLuaOpenFolder::~CLuaOpenFolder()
{
}

void CLuaOpenFolder::LoadFolder(lua_State* lua, char* folder)
{
    char name[MAX_PATH] = { 0 };

#if defined(__ANDROID__)
    // Normalize Windows backslashes to forward slashes for Android
    char normalized_folder[MAX_PATH];
    strncpy(normalized_folder, folder, MAX_PATH - 1);
    normalized_folder[MAX_PATH - 1] = '\0';
    for (char* p = normalized_folder; *p; ++p) { if (*p == '\\') *p = '/'; }

    // Android: use POSIX opendir/readdir
    DIR* dir = opendir(normalized_folder);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Filter for .lua files
        const char* fname = entry->d_name;
        size_t len = strlen(fname);
        if (len < 5 || strcmp(fname + len - 4, ".lua") != 0) continue;
        if (entry->d_type == DT_DIR) continue;

        wsprintf(name, "%s%s", normalized_folder, fname);
#else
    char wildcard_path[MAX_PATH];
    wsprintf(wildcard_path, "%s*.lua", folder);

    WIN32_FIND_DATA data;

    HANDLE file = FindFirstFile(wildcard_path, &data);

    if (file == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {

            wsprintf(name, "%s%s", folder, data.cFileName);
#endif

            std::string script = gFileProtectLua.ConvertMainFilePath((char*)name);

            lua_getfield(lua, LUA_REGISTRYINDEX, "_LOADED");
            lua_getfield(lua, 2, name);

            if (lua_toboolean(lua, -1))
            {
                gFileProtectLua.DeleteTemporaryFile();

                if (lua_touserdata(lua, -1) == sentinel)
                {
                    luaL_error(lua, "loop or previous error loading module " LUA_QS, name);
                }
                return;
            }


            if (luaL_loadbuffer(lua, script.c_str(), script.length(), name) != 0)
            {
                gFileProtectLua.DeleteTemporaryFile();
                if (!gProtect->m_MainInfo.LuaCrypt)
                {
                    luaL_error(lua, "error loading module " LUA_QS " from file " LUA_QS ":\n\t%s", lua_tostring(lua, 1), name, lua_tostring(lua, -1));
                    return;
                }
                else
                {
                    exit(0);
                    return;
                }
            }


            lua_pushlightuserdata(lua, sentinel);
            lua_setfield(lua, 2, name);
            lua_pushstring(lua, name);

            lua_call(lua, 1, 1);

            if (!lua_isnil(lua, -1))
            {
                lua_setfield(lua, 2, name);
            }

            lua_getfield(lua, 2, name);

            if (lua_touserdata(lua, -1) == sentinel)
            {
                lua_pushboolean(lua, 1);
                lua_pushvalue(lua, -1);
                lua_setfield(lua, 2, name);
            }

            gFileProtectLua.DeleteTemporaryFile();
#if defined(__ANDROID__)
    }
    closedir(dir);
}
#else
        }
    } while (FindNextFile(file, &data) != 0);
}
#endif

void CLuaOpenFolder::LoadFile(lua_State* L, char* name)
{
    std::string result = gFileProtectLua.ConvertMainFilePath((char*)name);

    lua_settop(L, 1);

    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_getfield(L, 2, name);

    if (lua_toboolean(L, -1))
    {
        gFileProtectLua.DeleteTemporaryFile();
        if (lua_touserdata(L, -1) == sentinel)
        {
            luaL_error(L, "loop or previous error loading module " LUA_QS, name);
        }
        return;
    }

    if (luaL_loadbuffer(L, result.c_str(), result.length(), name) != 0)
    {
        gFileProtectLua.DeleteTemporaryFile();

        if (gProtect->m_MainInfo.LuaCrypt)
        {
            luaL_error(L, "error loading module " LUA_QS " from string:\n\t%s", lua_tostring(L, 1), lua_tostring(L, -1));
            return;
        }
        else
        {
            exit(0);
            return;
        }
    }

    lua_pushlightuserdata(L, sentinel);
    lua_setfield(L, 2, name);
    lua_pushstring(L, name);

    lua_call(L, 1, 1);

    if (!lua_isnil(L, -1))
    {
        lua_setfield(L, 2, name);
    }

    lua_getfield(L, 2, name);

    if (lua_touserdata(L, -1) == sentinel)
    {
        lua_pushboolean(L, 1);
        lua_pushvalue(L, -1);
        lua_setfield(L, 2, name);
    }

    gFileProtectLua.DeleteTemporaryFile();
}

