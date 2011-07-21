//---------------------------------------------------------------------------
// dmk_fileman.cpp
//
// File stream management. FileManger provides mapping of file names to
// actual stream objects.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------


#include "dmk_fileman.h"
#include "a_base.h"
#include "a_nullstream.h"
#include <fstream>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------
// File manager is singleton
//----------------------------------------------------------------------------

FileManager * FileManager::mInstance = 0;

FileManager & FileManager :: Instance() {
	if ( mInstance == 0 ) {
		throw Exception( "No file manager instance" );
	}
	return * mInstance;
}

//----------------------------------------------------------------------------
// Create specifying default output stream, which will normally be stdout
// but can be changed via command line options.
//----------------------------------------------------------------------------

FileManager :: FileManager( std::ostream & defout ) : mDefOut( defout ) {
	if ( mInstance != 0 ) {
		throw Exception( "FileManage instance already exists" );
	}
	mInstance = this;
}

//----------------------------------------------------------------------------
// Reset singleton
//----------------------------------------------------------------------------

FileManager :: ~FileManager() {
	Clear();
	mInstance = 0;
}

//----------------------------------------------------------------------------
// Remove filename mappings
//----------------------------------------------------------------------------

void FileManager :: Clear() {
	NameMapType::iterator it = mNameMap.begin();
	while( it != mNameMap.end() ) {
		delete it->second;
		++it;
	}
	mNameMap.clear();
}

//----------------------------------------------------------------------------
// Special name for hidden output
//----------------------------------------------------------------------------

string FileManager :: HideName() const {
	return "hide";
}

//----------------------------------------------------------------------------
// special name for standard output
//----------------------------------------------------------------------------

string FileManager :: StdOutName() const {
	return "";
}

//----------------------------------------------------------------------------
// Givena filename, get the associated stream, possibly creating it, in
// which case add it to the ma.
//----------------------------------------------------------------------------

std::ostream & FileManager :: GetStream( const string & fname ) {
	NameMapType::const_iterator it = mNameMap.find( fname );
	if ( it != mNameMap.end() ) {
		return * it->second;
	}
	else if ( fname == HideName() ) {
		std::ostream * os = new ALib::NullStream;
		mNameMap.insert( std::make_pair( fname, os ));
		return * os;
	}
	else if ( fname == StdOutName() ) {
		return mDefOut;
	}
	else {
		std::ofstream * ofs = new std::ofstream( fname.c_str() );
		if ( ! ofs->is_open() ) {
			delete ofs;
			throw Exception( "Cannot open output file " + fname );
		}
		mNameMap.insert( std::make_pair( fname, ofs ));
		return * ofs;
	}
}


//----------------------------------------------------------------------------

}

// end

