//
//  pbx_processer.cpp
//  Foldex
//
//  Created by Stan Potemkin on 6/2/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include <sstream>
#include <unistd.h>
#include "pbx_processer.h"


pbx_processer::pbx_processer(project_t *project)
: _project(project)
{
}

void pbx_processer::process(string old_base, string new_base)
{
	vector<object_t *> project_list = _project->objects_by_isa("PBXProject");
	for (auto project : project_list) {
		string main_group_id = dynamic_cast<object_map_t*>(project)->fields["mainGroup"];
		recursive_build(main_group_id, true);
	}
	
	_old_project_path = old_base;
	_old_base = path_without_last_component(_old_project_path, false);
	_new_base = new_base;
	
	rename_files();
	move_project();
}

void pbx_processer::recursive_build(string id, bool is_main)
{
	object_t *object = _project->object_by_id(id);
	
	object_map_t *map = dynamic_cast<object_map_t*>(object);
	if (map) {
		if (is_isa(map, "PBXGroup")) {
			process_group(map, is_main);
		}
		else if (is_isa(map, "PBXFileReference")) {
			process_file(map);
		}
	}
}

void pbx_processer::process_group(object_map_t *group, bool is_main)
{
	string path = group->fields["path"];
	string name = group->fields["name"];
	
	if (group->fields["sourceTree"] == "SOURCE_ROOT") {
		path.insert(0, "/");
	}
	
	if (is_main == false) {
		_old_path.push_back(path);
		_new_path.push_back(name);
	}
	
	object_array_t *children = dynamic_cast<object_array_t*>(group->children["children"]);
	for (auto child_id : children->fields) {
		recursive_build(child_id, false);
	}
	
	if (is_main == false) {
		_old_path.pop_back();
		_new_path.pop_back();
	}
}

void pbx_processer::process_file(object_map_t *file)
{
	string path = file->fields["path"];
	string name = file->fields["name"];
	
	_old_path.push_back(path.length() ? path : name);
	_new_path.push_back(name.length() ? name : path);
	
	string old_path;
	if (file->fields["sourceTree"] == "<group>") {
		old_path = build_path(_old_path);
	}
	else if (file->fields["sourceTree"] == "SOURCE_ROOT") {
		old_path = path;
	}
	
	if (old_path.length()) {
		string new_path = build_path(_new_path);
		_renames[old_path] = new_path;
		
//		printf("move %s to %s\n", old_path.c_str(), new_path.c_str());
	}
	
	_old_path.pop_back();
	_new_path.pop_back();
}

bool pbx_processer::is_isa(object_t *object, string isa)
{
	if (object == NULL) {
		return false;
	}
	
	object_map_t *info = dynamic_cast<object_map_t*>(object);
	if (info) {
		return (info->fields["isa"] == isa);
	}
	
	return false;
}

string pbx_processer::path_without_last_component(string path, bool with_delimiter)
{
	if (path.length() < 2) {
		return string();
	}
	
	string::size_type idx = path.rfind("/", path.length() - 2);
	if (idx == string::npos) {
		return "";
	}
	else {
		if (with_delimiter) {
			idx++;
		}
		
		return path.replace(idx, path.length() - idx, "");
	}
}

string pbx_processer::build_path(vector<string> &path)
{
	string ret;
	for (auto it : path) {
		if (it == "") {
			continue;
		}
		else if (it == "..") {
			ret = path_without_last_component(ret, true);
		}
		else {
			ret = (it[0] == '/' ? it.substr(1) : ret + it) + "/";
		}
	}
	
	if (ret.length()) {
		ret.replace(ret.length() - 1, 1, "");
	}
	
	return ret;
}

void pbx_processer::rename_files()
{
	vector<string> not_exist;
	
	for (auto it : _renames) {
		string old_path = _old_base + "/" + it.first;
		string new_path = _new_base + "/" + it.second;
		
		if (access(old_path.c_str(), F_OK) != 0) {
			not_exist.push_back(old_path);
		}
		else {
			sprintf(_cmd, "mkdir -p \"$(dirname \"%s\")\"", new_path.c_str());
			system(_cmd);
			
			sprintf(_cmd, "cp -af \"%s\" \"%s\"", old_path.c_str(), new_path.c_str());
			system(_cmd);
		}
	}
	
	if (not_exist.size()) {
		printf("%s\n", "Looks like these files dont exist, check it out:");
		for (auto it : not_exist) {
			printf("\t%s\n", it.c_str());
		}
	}
}

void pbx_processer::move_project()
{
	sprintf(_cmd, "cp -af \"%s\" \"%s\"", _old_project_path.c_str(), _new_base.c_str());
	system(_cmd);
}
