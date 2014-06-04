//
//  pbx_processer.h
//  Foldex
//
//  Created by Stan Potemkin on 6/2/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include "pbx_reader.h"


class pbx_processer {
public:
	pbx_processer(project_t *project);
	void process(string old_base, string new_base);
	
private:
	void process_recursive(string id, bool is_main);
	void process_group(object_map_t *group, bool is_main);
	void process_file(object_map_t *file);
	void process_root_object(object_map_t *root);
	void process_build_configs();
	bool is_isa(object_t *object, string isa);
	
	string path_without_last_component(string path, bool with_delimiter);
	string build_path(vector<string> &path);
	
	void rename_files();
	void move_project_file();
	
private:
	project_t *_project;
	
	string _old_project_path;
	string _old_base;
	string _new_base;
	
	vector<string> _old_path;
	vector<string> _new_path;
	map<string, string> _renames;
	
	char _cmd[1024];
};
