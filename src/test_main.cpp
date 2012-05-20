//----------------------------------------------------------------------------
// test_main.cpp
//
// main for the dmk3 test build
//
// Copyright (C) 2012 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_myth.h"
#include "a_file.h"
#include "dmk_run.h"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;
using namespace ALib;


int main( int argc, char * argv[] ) {

	if ( std::getenv( ALib::MYTH_TESTS ) ) {
		return ALib::TestMain( argc, argv );
	}
	else {
		DMK::DMKRun runner( argc, argv );
		return runner.Run();
	}
}
