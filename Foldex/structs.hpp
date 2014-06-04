//
//  structs.hpp
//  Foldex
//
//  Created by Stan Potemkin on 6/1/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#ifndef Foldex_structs_hpp
#define Foldex_structs_hpp

#include <string>
#include <vector>
#include <map>
using namespace std;


class object_t {
public:
	enum {
		ot_int,
		ot_string,
		ot_array,
		ot_map
	} type;
	
	object_t() {
		type = ot_string;
	}
	
	virtual ~object_t() {
	}
};


class object_map_t : public object_t {
public:
	object_map_t() {
		type = ot_map;
	}
	
	map<string, string> fields;
	map<string, object_t*> children;
};


class object_array_t : public object_t {
public:
	object_array_t() {
		type = ot_array;
	}
	
	vector<string> fields;
	vector<object_t*> children;
};


class project_t {
public:
	project_t() {
	}
	
	object_map_t* objects() {
		object_map_t *objects = dynamic_cast<object_map_t*>(child);
		return objects;
	}
	
	object_t* object_by_id(string id) {
		return objects()->children[id];
	}
	
	vector<object_t*> objects_by_isa(string isa) {
		vector<object_t*> ret;
		
		for (auto it : objects()->children) {
			object_map_t *info = dynamic_cast<object_map_t*>(it.second);
			if (info->fields["isa"] == isa) {
				ret.push_back(info);
			}
		}
		
		return ret;
	}
	
public:
	string comment;
	string archive_version;
	string object_version;
	object_map_t classes;
	object_t *child;
	string rootRef;
};

#endif
