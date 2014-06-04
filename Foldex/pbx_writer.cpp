//
//  pbx_writer.cpp
//  Foldex
//
//  Created by Stan Potemkin on 6/3/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include "pbx_writer.h"


pbx_writer::pbx_writer(project_t *project)
: _project(project)
{
}


void pbx_writer::write(string filepath)
{
	object_map_t map;
	map.fields["archiveVersion"] = _project->archive_version;
	map.fields["objectVersion"] = _project->object_version;
	map.fields["rootObject"] = _project->rootRef;
	map.fields["archiveVersion"] = _project->archive_version;
	map.children["classes"] = &_project->classes;
	map.children["objects"] = _project->child;
	
	FILE *fp = fopen(filepath.c_str(), "w");
	if (fp) {
		_depth = 0;
		string data = _project->comment + data_for_offset() + data_for_object(&map);
		
		fwrite(data.c_str(), data.length(), 1, fp);
		fclose(fp);
	}
}


string pbx_writer::data_for_object(object_t *object)
{
	object_array_t *array = dynamic_cast<object_array_t*>(object);
	if (array) {
		return data_for_array_object(array);
	}
	
	object_map_t *map = dynamic_cast<object_map_t*>(object);
	if (map) {
		return data_for_map_object(map);
	}
	
	return string();
}


string pbx_writer::data_for_array_object(object_array_t *object)
{
	string ret;
	
	_depth++;
	ret += string() + "(" + data_for_newline();
	
	for (auto it : object->fields) {
		if (it.length() == 0) {
			continue;
		}
		
		ret += data_for_string(it) + "," + data_for_newline();
	}
	
	for (auto it : object->children) {
		ret += data_for_object(it) + "," + data_for_newline();
	}
	
	ret.replace(ret.length() - 1, 1, "");
	ret += string() + ")";
	_depth--;
	
	return ret;
}


string pbx_writer::data_for_map_object(object_map_t *object)
{
	string ret;
	
	_depth++;
	ret += string() + "{" + data_for_newline();
	
	for (auto it : object->fields) {
		if (it.second.length() == 0) {
			continue;
		}
		
		ret += it.first + " = " + data_for_string(it.second) + ";" + data_for_newline();
	}
	
	for (auto it : object->children) {
		ret += it.first + " = " + data_for_object(it.second) + ";" + data_for_newline();
	}
	
	ret.replace(ret.length() - 1, 1, "");
	ret += string() + "}";
	_depth--;
	
	return ret;
}


string pbx_writer::data_for_string(string str)
{
	return "\"" + str + "\"";
}


string pbx_writer::data_for_offset()
{
	static map<uint8_t, string> __offsets;
	
	string ret = __offsets[_depth];
	if (ret.length() == 0) {
		for (uint8_t i=0; i<_depth; i++) {
			ret += "\t";
		}
		
		__offsets[_depth] = ret;
	}
	
	return ret;
}

string pbx_writer::data_for_newline()
{
	return string("\n") + data_for_offset();
}
