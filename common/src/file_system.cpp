#include "file_system.h"
#if defined(_WIN32)
#include <corecrt_io.h>
#endif
#include <script_system.h>
#include "utils.h"

static String VFS_WORK_DIR="";
static String VFS_SCRIPT_DIR = "";
static String VFS_SHARE_SCRIPT_DIR = "";
FileSystem::FileSystem()
{

};

FileSystem::~FileSystem()
{
	
};

std::string FileSystem::GetPath()
{
	return VFS_WORK_DIR;
}

std::string FileSystem::MakePath(String rpath)
{
	return GetPath() + rpath;
}

std::string FileSystem::FormatPath(String path)
{
	return path.replace(path.begin(), path.end(), "\\", "/");
}


String FileSystem::GetTSVPath(String name)
{
	return GetPath() + "res/tables/" + name + ".tsv";
}

String FileSystem::GetTablePath(String name)
{
	return GetPath() + "res/tables/" + name;
}

std::string FileSystem::GetAbsPath(std::string localPath)
{
	return GetPath()  + localPath;
}

std::string FileSystem::GetResourcePath(std::string localPath)
{
	return GetPath() + "res/" + localPath;
}

std::string FileSystem::GetAssetsPath(std::string path)
{
	return GetPath() + "res/assets/" + path;
}

std::string FileSystem::GetShaderPath(std::string path)
{
	return GetPath() + "res/shader/" + path;
}
std::string FileSystem::GetLuaPath(std::string path)
{
	return VFS_SCRIPT_DIR + path;
}
std::string FileSystem::GetWDFPath(std::string path)
{
	return GetPath() + "res/data/" + path;
}
std::string FileSystem::GetMapPath(std::string path)
{
	return GetPath() + "res/data/scene/" + path + ".map";
}
std::string FileSystem::GetFontPath(std::string path)
{
	return GetPath() + "res/font/" + path;
}

std::string FileSystem::GetGameFontPath()
{
	return FileSystem::GetFontPath("simsun.ttc");
}

std::string FileSystem::GetIconPath(std::string path)
{
	return GetPath() + "res/icon/" + path;
}

void FileSystem::InitWorkPath()
{
	std::string cwd = script_system_get_config("cwd");
	if (cwd != "nil")
	{
		VFS_WORK_DIR = cwd;
	}
	else {
		std::string path = script_system_get_config("argv0");
		std::string PATH_SEP("");
		std::string CWD = "";
		if (path.find_last_of("\\") != std::string::npos) {
			PATH_SEP = "\\";
		}
		else if (path.find_last_of("/") != std::string::npos)
		{
			PATH_SEP = "/";
		}
		VFS_WORK_DIR = path.substr(0, path.find_last_of(PATH_SEP)) + PATH_SEP;
	}

	std::string script_path = script_system_get_config("script_path");
	if (script_path == "nil") {
#ifdef SIMPLE_SERVER
		VFS_SCRIPT_DIR = VFS_WORK_DIR + "scripts/server/";
#else
		VFS_SCRIPT_DIR = VFS_WORK_DIR + "scripts/client/"; 
#endif // SIMPLE_SERVER
	}
	else {
		VFS_SCRIPT_DIR = VFS_WORK_DIR+script_path;
	}

	std::string share_script_path = script_system_get_config("share_script_path");
	if (share_script_path == "nil") {
		VFS_SHARE_SCRIPT_DIR = VFS_WORK_DIR+"scripts/share/";
	}
	else {
		VFS_SHARE_SCRIPT_DIR = VFS_WORK_DIR+share_script_path;
	}	
}

std::vector<std::string> VFS_ListFiles(std::string path)
{
#if defined(_WIN32)
    std::vector<std::string> files;
	String filter_path(path);
	filter_path.append("\\*.*");
    intptr_t handle;
    _finddata_t findData;

    handle = _findfirst(filter_path.c_str(), &findData);
    if (handle == -1)
        return files;

    int limit = 1000;
    int found = 0;
    do
    {
		String filename = findData.name;
		if (filename != "." && filename != "..")
		{
			String p(path);
			p.append("/").append(findData.name);
			files.push_back(p);
			found++;
		}
    } while (_findnext(handle, &findData) == 0 || found >= limit);

    _findclose(handle);
    return files;
#else
    return {};
#endif
}

std::vector<std::string> VFS_ListAllFiles(std::string path)
{
#if defined(_WIN32)
    std::vector<std::string> files;
    path.append("\\*.*");
    intptr_t handle;
    _finddata_t findData;

    handle = _findfirst(path.c_str(), &findData);
    if (handle == -1)
        return files;

    int limit = 1000;
    int found = 0;
    do
    {
        if (findData.attrib & _A_SUBDIR)
        {
            if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
                continue;
            //cout << findData.name << "\t<dir>\n";
            auto nextFiles = VFS_ListFiles(path + "\\" + findData.name);
            for (auto f : nextFiles)
            {
                files.push_back(f);
                found++;
            }
        }
        else
        {
            files.push_back(findData.name);
            found++;
        }
    } while (_findnext(handle, &findData) == 0 || found >= limit);
    
    _findclose(handle);
    return files;
#else
    return {};
#endif
}


std::string fs_get_tsv_path(const char* name)
{
	return FileSystem::GetTSVPath(name);
}

int vfs_list_files(lua_State* L)
{
	auto path = luaL_checkstring(L, 1); 
	auto files = VFS_ListFiles(path);
	lua_newtable(L);
	int index = 1;
	for (auto& f : files) 
	{
		lua_pushstring(L, f.c_str());
		lua_rawseti(L, -2,index++);
	}
	return 1;
}

int vfs_set_workdir(lua_State* L)
{
	const char* path = luaL_checkstring(L, 1);
	VFS_WORK_DIR = path;
	return 0;
}


int vfs_get_workdir(lua_State* L)
{
	lua_pushstring(L, VFS_WORK_DIR.c_str());
	return 1;
}

void luaopen_filesystem(lua_State*L)
{
	script_system_register_function(L, fs_get_tsv_path);
	script_system_register_luac_function(L, vfs_list_files);
	script_system_register_luac_function(L, vfs_set_workdir);
	script_system_register_luac_function(L, vfs_get_workdir);
}