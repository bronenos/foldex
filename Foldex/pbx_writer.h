//
//  pbx_writer.h
//  Foldex
//
//  Created by Stan Potemkin on 6/3/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//


#include <string>
#include <stack>
#include "structs.hpp"
using namespace std;


class pbx_writer {
public:
	pbx_writer(project_t *project);
	void write(string filepath);
	
	string data_for_object(object_t *object);
	string data_for_array_object(object_array_t *object);
	string data_for_map_object(object_map_t *object);
	string data_for_string(string str);
	string data_for_offset();
	string data_for_newline();
	
private:
	project_t *_project;
	uint8_t _depth;
};
