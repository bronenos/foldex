//
//  parser.h
//  Foldex
//
//  Created by Stan Potemkin on 6/1/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include <string>
#include <stack>
#include "structs.hpp"
using namespace std;


class pbx_reader {
public:
	pbx_reader(string filepath);
	project_t* parse();
	
private:
	string parse_project_comment();
	object_t* parse_child();
	
	const char* find_equal();
	const char* find_value();
	const char* find_quote();
	const char* find_non_alphadigit();
	const char* find_delimiter();
	
	inline void step_set(const char *it);
	inline const char* step_get();
	inline const char step_get_char();
	inline void step_forward();
	
	void skip_space();
	void skip_comment();
	void skip_space_and_comment();
	void skip_optional_delimiter();
	
	string parse_key();
	string parse_int();
	string parse_string();
	
	bool is_valid_path(const char *c);
	
private:
	string _data;
	const char *_it;
	stack<bool> _container_type;
};
