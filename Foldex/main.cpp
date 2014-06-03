//
//  main.cpp
//  Foldex
//
//  Created by Stan Potemkin on 6/1/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include <iostream>
#include "pbx_processer.h"


enum {
	kFoldexArgSelfPath,
	kFoldexArgProjectPath,
	kFoldexArgNewFolderPath,
	kFoldexArg_Count,
};


int main(int argc, const char * argv[])
{
	if (argc < kFoldexArg_Count) {
		std::cout << "Usage:" << endl;
		std::cout << "foldex (path_to_xcodeproj_file) (path_to_new_project_folder)" << endl;
		return 0;
	}
	
	const char *project_path = argv[kFoldexArgProjectPath];
	const char *internal_path = "/project.pbxproj";
	const char *new_folder_path = argv[kFoldexArgNewFolderPath];
	
	char *path = new char[strlen(project_path) + strlen(internal_path) + 1];
	if (path) {
		strcpy(path, project_path);
		strcat(path, internal_path);
		
		project_t *parser = pbx_reader(path).parse();
		if (parser) {
			pbx_processer proc(parser);
			proc.process(project_path, new_folder_path);
		}
		
		delete[] path;
	}
	
    return 0;
}

