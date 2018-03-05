#include "Engine/Core/directory.h"
#include "Engine/Core/log.h"
#include "Engine/Core/StringUtils.hpp"
#include <vector>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool create_directory(const char* directory_path)
{
	std::vector<std::string> dir_components = tokenize_string_by_delimeter(directory_path, '/');

	std::string current_folder;

	while(!dir_components.empty()){
		current_folder += dir_components[0] + "/";
		dir_components.erase(dir_components.begin());

	    BOOL created = CreateDirectoryA(current_folder.c_str(), NULL);
	    if(created == 0){
	        if(GetLastError() != ERROR_ALREADY_EXISTS){
	            log_warningf("ERROR: Failed to create directory structure [%s]\n", directory_path);
				return false;
	        }
	    }
	}

	return true;
}