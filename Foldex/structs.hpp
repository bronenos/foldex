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
		archive_version = 0;
		object_version = 0;
	}
	
	object_t* object_by_id(string id) {
		object_map_t *root = dynamic_cast<object_map_t*>(child);
		object_map_t *objects = dynamic_cast<object_map_t*>(root->children["objects"]);
		return objects->children[id];
	}
	
public:
	string comment;
	uint8_t archive_version;
	uint8_t object_version;
	object_t *child;
	string rootRef;
};

#endif
