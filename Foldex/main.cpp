//
//  main.cpp
//  Foldex
//
//  Created by Stan Potemkin on 6/1/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include <iostream>
#include "pbx_processer.h"
#include "pbx_writer.h"


enum {
	kFoldexArgSelfPath,
	kFoldexArgProjectPath,
	kFoldexArgNewFolderPath,
	kFoldexArg_Count,
};


void print_usage()
{
	std::cout << "Usage:" << endl;
	std::cout << "foldex (path_to_xcodeproj_file) (path_to_new_project_folder)" << endl;
}


int main(int argc, const char * argv[])
{
	if (argc < kFoldexArg_Count) {
		print_usage();
		return 0;
	}
	
	const char *project_path = argv[kFoldexArgProjectPath];
	const char *new_folder_path = argv[kFoldexArgNewFolderPath];
	const char *internal_path = "/project.pbxproj";
	
	if (strrchr(project_path, '/') == nullptr || strrchr(new_folder_path, '/') == nullptr) {
		print_usage();
		return 0;
	}
	
	char *old_path = new char[strlen(project_path) + strlen(internal_path) + 1];
	if (old_path) {
		// build the full path to the current project file
		strcpy(old_path, project_path);
		strcat(old_path, internal_path);
		
		char *new_path = new char[strlen(new_folder_path) + strlen(project_path) +  strlen(internal_path) + 1];
		if (new_path) {
			// build the full path to the new project file
			strcpy(new_path, new_folder_path);
			strcat(new_path, strrchr(project_path, '/'));
			strcat(new_path, internal_path);
			
			project_t *project = pbx_reader(old_path).parse();
			if (project) {
				pbx_processer(project).process(project_path, new_folder_path);
				pbx_writer(project).write(new_path);
			}
			
			delete[] new_path;
		}
		
		delete[] old_path;
	}
	
    return 0;
}

