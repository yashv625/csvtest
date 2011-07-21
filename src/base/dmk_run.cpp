//---------------------------------------------------------------------------
// dmk_run.cpp
//
// this is the "main" for DMK
// ??? this has been hacked  about too much
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "dmk_run.h"
#include "dmk_modman.h"
#include "dmk_strings.h"
#include "dmk_random.h"
#include "dmk_fileman.h"

#include <time.h>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const CHECK_FLAG 		= "-c";
const char * const FORMAT_FLAG 	= "-f";
const char * const GEN_FLAG 		= "-g";
const char * const RANDVAL_FLAG 	= "-rn";
const char * const TIME_SEED		= "time";

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// argc & argv are from the executables main()
//----------------------------------------------------------------------------

DMKRun :: DMKRun( int argc, char *argv[] )
	: mCmdLine( argc, argv ) {

}

//----------------------------------------------------------------------------
// Nothing to do
//----------------------------------------------------------------------------

DMKRun :: ~DMKRun() {
}

//----------------------------------------------------------------------------
// helper to translate flag to model tun mode
//----------------------------------------------------------------------------
/*
static ModelManager::ModelMode GetMode( const std::string & flag ) {
	if ( flag == CHECK_FLAG ) {
		return ModelManager::mmCheckOnly;
	}
	else if ( flag == GEN_FLAG ) {
		return ModelManager::mmGenOnly;
	}
	else if ( flag == FORMAT_FLAG ) {
		return ModelManager::mmGenForm;
	}
	else {
		throw Exception( "Bad flag " + flag );
	}
}
*/
//----------------------------------------------------------------------------
// Process command line flags, passing results to then model manager
//----------------------------------------------------------------------------

int DMKRun :: Run() {
	try {

		FileManager fm( std::cout );	// create singleton
		mCmdLine.AddFlag( ALib::CommandLineFlag( RANDVAL_FLAG, false, 1, true ) );
		mCmdLine.CheckFlags(1);
/*
		mCmdLine.AddFlag( ALib::CommandLineFlag( GEN_FLAG, false, 1, true ) );
		mCmdLine.AddFlag( ALib::CommandLineFlag( FORMAT_FLAG, false, 1, true ) );
		mCmdLine.AddFlag( ALib::CommandLineFlag( CHECK_FLAG, false, 1, true ) );

		mCmdLine.CheckFlags(1);
*/
		SeedRNG();
/*
		int pos = 1;
		while( pos < mCmdLine.Argc() ) {
			string flag = mCmdLine.Argv( pos++ );
			if ( flag == CHECK_FLAG || flag == FORMAT_FLAG || flag == GEN_FLAG ) {
				if ( pos == mCmdLine.Argc() ) {
					throw Exception( "No filename following " + flag );
				}
				string filename = mCmdLine.Argv( pos++ );
				ModelManager::Instance()->AddModelFromFile( filename, GetMode( flag ) );
			}
		}
*/
		if ( mCmdLine.FileCount() != 1 ) {
			//std::cerr << "File count:" << mCmdLine.FileCount() << std::endl;
			std::cerr << "CSVTest version Alpha 0.1" << std::endl;
			std::cerr << "Copyright (C) 2009 Neil Butterworth" << std::endl;
			std::cerr << "usage: csvtest  [flags] script.xml" << std::endl;
			return -1;
		}

		string filename = mCmdLine.File( 0 );
		ModelManager::Instance()->AddModelFromFile( filename, ModelManager::mmGenForm );
		ModelManager::Instance()->RunModels( std::cout );

		return 0;
	}
	catch( const std::exception & ex ) {
		std::cerr << "Error: " << ex.what() << std::endl;
		return -1;
	}
	catch( ... ) {
		std::cerr << "Error: " << "Unknown exception" << std::endl;
		return -1;
	}
	return 0;
}

//----------------------------------------------------------------------------
// Seed the random number generator. Default is to use current time as seed.
//----------------------------------------------------------------------------

void DMKRun :: SeedRNG() {
	if ( mCmdLine.HasFlag( RANDVAL_FLAG ) ) {
		string s = mCmdLine.GetValue( RANDVAL_FLAG, "" );
		if ( ALib::IsInteger( s ) ) {
			RNG::Randomise( ALib::ToInteger( s ) );
		}
		else if ( s == TIME_SEED ) {
			RNG::Randomise( time(0) );
		}
		else {
			throw Exception( "Invalid value for random seed: " + s );
		}

	}
	else {
		RNG::Randomise( time(0) );
	}
}


//----------------------------------------------------------------------------

} // namespace


// end

