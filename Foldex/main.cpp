//
//  main.cpp
//  Foldex
//
//  Created by Stan Potemkin on 6/1/14.
//  Copyright (c) 2014 bronenos. All rights reserved.
//

#include <iostream>
#include "pbx_parser.h"


int main(int argc, const char * argv[])
{
	if (argc < 2) {
		std::cout << "You have to specify the project's filepath";
		return 0;
	}
	
	pbx_parser parser(argv[1]);
	parser.parse();
	
    return 0;
}

