//---------------------------------------------------------------------------
// dmk_modman.cpp
//
// model manager for dmk
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "dmk_modman.h"
#include "dmk_strings.h"
#include "dmk_xmlutil.h"
#include "a_base.h"
#include "a_str.h"
#include "a_xmlparser.h"
#include <iostream>
#include <fstream>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char NAMESEP_CHAR 		= '.';		// model - generator sparator
const char * const INCLUDE_TAG = "include";
const char * const FILE_ATTR 	= "file";
const int MAX_INC 				= 50;

//----------------------------------------------------------------------------
// only needed so we can make it private
//----------------------------------------------------------------------------

ModelManager :: ModelManager() {
}

//----------------------------------------------------------------------------
// free owned models
//----------------------------------------------------------------------------

ModelManager :: ~ModelManager() {
	Clear();
}

//----------------------------------------------------------------------------
// Replaces the element e with a new XML tree created from file fname.
//----------------------------------------------------------------------------

void ModelManager :: ReplaceFromFile( ALib::XMLElement * e,
										const string & fname ) {
//	ALib::XMLElement * p = new ALib::XMLElement( "counter" );
//	e->Replace( p );
	ALib::XMLTreeParser parser;
	ALib::XMLElement * p = parser.ParseFile( fname );
	if ( p == 0 ) {
		throw Exception( "XML parser error: during include " + parser.ErrorMsg() );
	}
	e->Replace( p );
}

//----------------------------------------------------------------------------
// Process a single include tag in the tree, returning true if it finds one.
// This is called repeatedly withsame root until all includes are processed.
//----------------------------------------------------------------------------

bool ModelManager :: ProcessInclude( ALib::XMLElement * e ) {
	if ( e->Name() == INCLUDE_TAG ) {
		if ( e->ChildCount() != 0 ) {
			XMLERR( e, "include cannot have content" );
		}
		string incfile = e->AttrValue( FILE_ATTR, "" );
		if ( incfile == ""  ) {
			XMLERR( e, "No include file specified" );
		}
		ReplaceFromFile( e, incfile );
		return true;
	}
	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		ALib::XMLElement * ce = const_cast <ALib::XMLElement *>(e->ChildElement(i) );
		if ( ce  && ProcessInclude( ce ) ) {
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// Add uniquely named model, taking ownsership
//----------------------------------------------------------------------------

void ModelManager :: AddModel( Model * m, ModelMode mm ) {
	if ( FindModel( m->Name() ) ) {
		throw Exception( "Duplicate model name " + ALib::SQuote( m->Name() ));
	}
	mModels.push_back( MME( m, mm ) );
}

//----------------------------------------------------------------------------
// Create model from XML file and add it to manager
//----------------------------------------------------------------------------

void ModelManager :: AddModelFromFile( const string & fname, ModelMode mm ) {
	ALib::XMLTreeParser parser;
	std::auto_ptr <ALib::XMLElement> e ( parser.ParseFile( fname ) );
	if ( e.get() == 0 ) {
		throw Exception( "XML parser error: " + parser.ErrorMsg()
							+ " at line " + ALib::Str( parser.ErrorLine()) );
	}

	if ( e.get()->Name() != DMKROOT_TAG ) {
		throw Exception( "Expected tag " + ALib::SQuote( DMKROOT_TAG )
							+ " but found "
							+ ALib::SQuote( e.get()->Name()));
	}

	int incs = 0;
	while( ProcessInclude( e.get() ) ) {
		if ( ++incs > MAX_INC ) {
			throw Exception( "Too many includes" );
		}
	}

	ModelBuilder builder;
	std::auto_ptr <Model> m( builder.Build( e.get() ) );

	AddModel( m.get(), mm );
	m.release();
}

//----------------------------------------------------------------------------
// free owned models
//----------------------------------------------------------------------------

void ModelManager :: Clear() {
	for ( unsigned int i = 0; i < mModels.size(); i++ ) {
		delete mModels[i].mModel;
	}
	mModels.clear();
}

//----------------------------------------------------------------------------
// find model by name, returning 0 if not there
//----------------------------------------------------------------------------

Model * ModelManager :: FindModel( const std::string & name ) const {
	for ( unsigned int i = 0; i < mModels.size(); i++ ) {
		if ( mModels[i].mModel->Name() == name ) {
			return mModels[i].mModel;
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// Find generator by name, which  may be in the form "model.gen" or "gen"
//.returns pointer to generator, or NULL if not foyund
//----------------------------------------------------------------------------

Generator * ModelManager :: FindGen( const std::string & name ) const {
	std::vector <string> tmp;
	int n = ALib::Split( name,  NAMESEP_CHAR, tmp );
	if ( n < 1 || n > 2 ) {
		std::cerr << "Split " << name << " count is " << n << std::endl;
		throw Exception( "Invalid generator name " + ALib::SQuote( name ) );
	}
	string mname = n == 1 ? "" : tmp[0];
	string gname = n == 1 ?  tmp[0] : tmp[1];

	Model * mp = FindModel( mname );
	if ( mp == 0 ) {
		return 0;
	}

	Generator * gp = mp->FindGen( gname );
	return gp;
}

//----------------------------------------------------------------------------
// Run models giving them stream as default output
//----------------------------------------------------------------------------

void ModelManager :: RunModels( std::ostream & ) {
	for ( unsigned int i = 0; i < mModels.size(); i++ ) {
		if ( mModels[i].mMode != mmCheckOnly ) {
			mModels[i].mModel->Generate();
		}
	}
}

//----------------------------------------------------------------------------
// Access count specified on command line
//----------------------------------------------------------------------------

int ModelManager :: CommandLineCount() const {
	return mCmdLineCount;
}

int & ModelManager :: CommandLineCount() {
	return mCmdLineCount;
}

//----------------------------------------------------------------------------
// single model manager instance
//----------------------------------------------------------------------------

ModelManager * ModelManager :: Instance() {
	static ModelManager man;
	return & man;
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

// Testing

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "ModMan" );


#endif

//----------------------------------------------------------------------------


// end

