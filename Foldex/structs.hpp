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
	virtual ~object_t() {
	}
};


class field_t {
public:
	std::string value;
	object_t *object;
};


class object_map_t : public object_t {
public:
	map<string, field_t> fields;
};


class object_array_t : public object_t {
public:
	vector<field_t> fields;
};


class project_t {
public:
	project_t() {
	}
	
	object_map_t* objects() {
		object_map_t *root_map = dynamic_cast<object_map_t*>(root);
		object_map_t *objects = dynamic_cast<object_map_t*>(root_map->fields["objects"].object);
		return objects;
	}
	
	object_t* object_by_id(string id) {
		return objects()->fields[id].object;
	}
	
	vector<object_t*> objects_by_isa(string isa) {
		vector<object_t*> ret;
		
		for (auto it : objects()->fields) {
			if (it.second.object == nullptr) {
				continue;
			}
			
			object_map_t *info = dynamic_cast<object_map_t*>(it.second.object);
			if (info->fields["isa"].value == isa) {
				ret.push_back(info);
			}
		}
		
		return ret;
	}
	
public:
	string comment;
	object_t *root;
};

#endif
