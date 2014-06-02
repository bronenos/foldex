//
//  parser.cpp
//  Foldex
//
//  Created by Stan Potemkin on 6/1/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include <stdlib.h>
#include <cstdio>
#include "pbx_parser.h"


pbx_parser::pbx_parser(string filepath)
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
}

project_t* pbx_parser::parse()
{
	string comment = parse_project_comment();
	object_map_t *obj = dynamic_cast<object_map_t *>(parse_child());
	
	project_t *proj = new project_t;
	proj->comment = comment;
	proj->archive_version = atoi(obj->fields["archiveVersion"].c_str());
	proj->object_version = atoi(obj->fields["objectVersion"].c_str());
	proj->child = obj;
	proj->rootRef = obj->fields["rootObject"];
	
	return proj;
}

string pbx_parser::parse_project_comment()
{
	const char *key_begin = step_get();
	const char *key_end = strstr(step_get(), "{");
	
	string ret = string(key_begin, key_end);
	step_set(key_end);
	
	return ret;
}

object_t* pbx_parser::parse_child()
{
	if (*step_get() == '{') {
		step_forward();
		
		object_map_t *obj = new object_map_t;
		_container_type.push(true);
		
		while (true) {
			skip_space_and_comment();
			
			string key;
			if (isalpha(step_get_char()) || isdigit(step_get_char())) {
				key = parse_key();
				
				skip_space_and_comment();
				find_value();
			}
			
			if (step_get_char() == '"') {
				string value = parse_string();
				obj->fields[key] = value;
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (is_valid_path(step_get())) {
				string value = parse_string();
				obj->fields[key] = value;
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (step_get_char() == '{' || step_get_char() == '(') {
				obj->children[key] = parse_child();
				
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
		_container_type.push(false);
		
		while (true) {
			skip_space_and_comment();
			
			if (step_get_char() == '"') {
				string value = parse_string();
				obj->fields.push_back(value);
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (is_valid_path(step_get())) {
				string value = parse_string();
				obj->fields.push_back(value);
				
				skip_space_and_comment();
				find_delimiter();
			}
			else if (step_get_char() == '{' || step_get_char() == '(') {
				obj->children.push_back(parse_child());
				
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
	
	return NULL;
}

//string pbx_parser::remove_comments(string data)
//{
//	string ret;
//	
//	const char *c_str = data.c_str();
//	const char *begin = c_str;
//	string::size_type idx_begin = 0;
//	
//	while (true) {
//		idx_begin = data.find("/*", idx_begin);
//		if (idx_begin != string::npos) {
//			ret += string(begin, idx_begin - (begin - c_str));
//		}
//		else {
//			ret += string(begin, c_str + data.length() - begin);
//			break;
//		}
//		
//		string::size_type idx_end = data.find("*/", idx_begin);
//		if (idx_end != string::npos) {
//			begin = c_str + idx_end + 2;
//			idx_begin = begin - c_str;
//		}
//	}
//	
//	return ret;
//}

const char* pbx_parser::find_equal()
{
	while (*step_get() != '=') {
		step_forward();
	}
	
	return step_get();
}

const char* pbx_parser::find_value()
{
	step_set(strchr(step_get(), '='));
	step_forward();
	skip_space();
	return step_get();
}

const char* pbx_parser::find_quote()
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

const char* pbx_parser::find_non_alphadigit()
{
	while (is_valid_path(step_get())) {
		step_forward();
	}
	
	return step_get();
}

const char* pbx_parser::find_delimiter()
{
	const char *end = _data.data() + _data.length();
	const char *delimiters = _container_type.top() ? ";}" : ",)";
	
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

void pbx_parser::step_set(const char *it)
{
	_it = it;
}

const char* pbx_parser::step_get()
{
	return _it;
}

const char pbx_parser::step_get_char()
{
	return *step_get();
}

void pbx_parser::step_forward()
{
	_it++;
}

void pbx_parser::skip_space()
{
	while (isspace(step_get_char())) {
		step_forward();
	}
}

void pbx_parser::skip_comment()
{
	static const char *comment_begin	= "/*";
	static const char *comment_end		= "*/";
	
	if (strncmp(step_get(), comment_begin, strlen(comment_begin)) == 0) {
		const char *end = strstr(step_get(), comment_end);
		step_set(end + strlen(comment_end));
	}
}

void pbx_parser::skip_space_and_comment()
{
	skip_space();
	skip_comment();
	skip_space();
}

void pbx_parser::skip_optional_delimiter()
{
	const char c = _container_type.top() ? ';' : ',';
	
	if (step_get_char() == c) {
		step_forward();
	}
}

string pbx_parser::parse_key()
{
	const char *key_begin = step_get();
	const char *key_end = find_non_alphadigit();
	
	string key = string(key_begin, key_end - key_begin);
	return key;
}

string pbx_parser::parse_int()
{
	int32_t number = atoi(step_get());
	return to_string(number);
}

string pbx_parser::parse_string()
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

bool pbx_parser::is_valid_path(const char *c)
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
