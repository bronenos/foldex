//
//  parser.cpp
//  Foldex
//
//  Created by Stan Potemkin on 6/1/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include <stdlib.h>
#include <cstdio>
#include "pbx_reader.h"


pbx_reader::pbx_reader(string filepath)
{
	FILE *fp = fopen(filepath.c_str(), "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		long filesize = ftell(fp);
		
		fseek(fp, 0, SEEK_SET);
		char *buff = new char[filesize];
		
		fread(buff, filesize, 1, fp);
		_data.append(buff, filesize);
		
		delete[] buff;
		fclose(fp);
		
		step_set(_data.data());
	}
	else {
		step_set(nullptr);
	}
}

project_t* pbx_reader::parse()
{
	if (step_get()) {
		string comment = parse_project_comment();
		object_map_t *obj = dynamic_cast<object_map_t *>(parse_child());
		
		project_t *proj = new project_t;
		proj->comment = comment;
		proj->root = obj;
		
		return proj;
	}
	else {
		return nullptr;
	}
}

string pbx_reader::parse_project_comment()
{
	const char *key_begin = step_get();
	const char *key_end = strstr(step_get(), "{");
	
	string ret = string(key_begin, key_end);
	step_set(key_end);
	
	return ret;
}

object_t* pbx_reader::parse_child()
{
	if (*step_get() == '{') {
		step_forward();
		
		object_map_t *obj = new object_map_t;
		_container_type.push(kPBXContainerTypeDictionary);
		
		while (true) {
			skip_space_and_comment();
			
			string key;
			if (step_get_char() == '"') {
				key = parse_string();
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (is_valid_path(step_get())) {
				key = parse_string();
				
				skip_space_and_comment();
				find_value();
			}
			
			if (step_get_char() == '"') {
				obj->fields[key].value = parse_string();
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (is_valid_path(step_get())) {
				obj->fields[key].value = parse_string();
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (step_get_char() == '{' || step_get_char() == '(') {
				obj->fields[key].object = parse_child();
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (step_get_char() == '}') {
				step_forward();
				_container_type.pop();
				return obj;
			}
			
			skip_space_and_comment();
			skip_optional_delimiter();
			skip_space_and_comment();
		}
	}
	else if (*step_get() == '(') {
		step_forward();
		
		object_array_t *obj = new object_array_t;
		_container_type.push(kPBXContainerTypeArray);
		
		while (true) {
			skip_space_and_comment();
			
			if (step_get_char() == '"') {
				field_t field = { parse_string(), nullptr };
				obj->fields.push_back(field);
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (is_valid_path(step_get())) {
				field_t field = { parse_string(), nullptr };
				obj->fields.push_back(field);
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (step_get_char() == '{' || step_get_char() == '(') {
				field_t field = { parse_string(), nullptr };
				obj->fields.push_back(field);
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (step_get_char() == ')') {
				step_forward();
				_container_type.pop();
				return obj;
			}
			
			skip_space_and_comment();
			skip_optional_delimiter();
			skip_space_and_comment();
		}
	}
	
	return nullptr;
}

const char* pbx_reader::find_equal()
{
	while (*step_get() != '=') {
		step_forward();
	}
	
	return step_get();
}

const char* pbx_reader::find_value()
{
	step_set(strchr(step_get(), '='));
	step_forward();
	skip_space();
	return step_get();
}

const char* pbx_reader::find_quote()
{
	while (true) {
		if (*step_get() != '"') {
			step_forward();
			continue;
		}
		
		if (*(step_get() - 1) == '\\') {
			step_forward();
			continue;
		}
		
		break;
	}
	
	return step_get();
}

const char* pbx_reader::find_non_alphadigit()
{
	while (is_valid_path(step_get())) {
		step_forward();
	}
	
	return step_get();
}

const char* pbx_reader::find_delimiter()
{
	const char *end = _data.data() + _data.length();
	const char *delimiters = delimiters_for_current_container();
	
	while (true) {
		if (step_get() == end) {
			break;
		}
		
		if (strchr(delimiters, step_get_char())) {
			break;
		}
		
		step_forward();
	}
	
	return step_get();
}

void pbx_reader::step_set(const char *it)
{
	_it = it;
}

const char* pbx_reader::step_get()
{
	return _it;
}

const char pbx_reader::step_get_char()
{
	return *step_get();
}

void pbx_reader::step_forward()
{
	_it++;
}

void pbx_reader::skip_space()
{
	while (isspace(step_get_char())) {
		step_forward();
	}
}

void pbx_reader::skip_comment()
{
	static const char *comment_begin	= "/*";
	static const char *comment_end		= "*/";
	
	if (strncmp(step_get(), comment_begin, strlen(comment_begin)) == 0) {
		const char *end = strstr(step_get(), comment_end);
		step_set(end + strlen(comment_end));
	}
}

void pbx_reader::skip_space_and_comment()
{
	skip_space();
	skip_comment();
	skip_space();
}

void pbx_reader::skip_optional_delimiter()
{
	if (step_get_char() == delimiters_for_current_container()[0]) {
		step_forward();
	}
}

string pbx_reader::parse_int()
{
	int32_t number = atoi(step_get());
	return to_string(number);
}

string pbx_reader::parse_string()
{
	const char c = *step_get();
	if (c == '"') {
		step_forward();
		const char *value_begin = step_get();
		const char *value_end = find_quote();
		step_forward();
		
		return string(value_begin, value_end - value_begin);
	}
	else {
		const char *value_begin = step_get();
		const char *value_end = find_non_alphadigit();
		
		return string(value_begin, value_end - value_begin);
	}
	
	return string();
}

const char* pbx_reader::delimiters_for_current_container()
{
	switch (_container_type.top()) {
		case kPBXContainerTypeArray:		return ",)";
		case kPBXContainerTypeDictionary:	return ";}";
	}
	
	return nullptr;
}

bool pbx_reader::is_valid_path(const char *c)
{
	if (*c == 0x00) {
		return false;
	}
	
	if (isalpha(*c) || isdigit(*c) || strchr("._", *c)) {
		return true;
	}
	
	if (*(c + 0) == '/' && *(c + 1) != '*') {
		return true;
	}
	
	return false;
}
