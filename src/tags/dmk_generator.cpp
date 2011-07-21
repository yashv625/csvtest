//---------------------------------------------------------------------------
// dmk_generator.cpp
//
// Tag wrapper for the base generator class.
// Most stuff is done in the base - probably everything should be.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "dmk_model.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include "dmk_fileman.h"
#include <set>
#include <memory>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const GEN_TAG 	= "gen";
const char * const NAME_ATTR 	= "name";
const char * const HIDE_ATTR 	= "hide";
const char * const FNAMES_ATTR = "fields";
const char * const GROUP_ATTR  = "group";


//----------------------------------------------------------------------------

class GeneratorTag : public Generator {

	public:

		GeneratorTag( const std::string & name,
						int count, bool debug,
						const std::string & ofn,
						const std::string & fields,
						const FieldList & grp );

		void Generate( Model * model );
		bool Hide() const;

		static Generator * FromXML( const ALib::XMLElement * e );

	private:

		int mCount;
		bool mHide;
		std::string mOutFile;
		ALib::CommaList mFields;

};

//----------------------------------------------------------------------------
// Register tag
//----------------------------------------------------------------------------

static RegisterGen <GeneratorTag> regrs1_( GEN_TAG );

//----------------------------------------------------------------------------
// construct from unique name & count of rows to generate
//----------------------------------------------------------------------------

GeneratorTag :: GeneratorTag( const string & name,
								int count, bool debug,
								const string &  ofn,
								const string & fields,
								const FieldList & grp  )
	: Generator( name, debug, grp ),
		mCount( count ),  mOutFile( ofn ), mFields( fields ) {
}

//----------------------------------------------------------------------------
// generate the data. if the user wants "all rows" we need to]
// send a "size" message to all children to find how many rows to produce.
//----------------------------------------------------------------------------

void GeneratorTag :: Generate( Model * model ) {

	std::ostream & os = FileManager::Instance().GetStream( mOutFile );
	int nrows = mCount < 0 ? GetSize() : mCount;
	bool debug = false; // model->Debug() || Debug();

	if ( debug ) {
		std::cerr << "----- begin " << Name() << "\n";
	}

	if ( mFields.Size() ) {
		Row r;
		for ( unsigned int i = 0; i < mFields.Size(); i++ ) {
			r.AppendValue( mFields.At( i ) );
		}
		if ( debug ) {
			DebugRow( r, std::cerr );
		}
		os << r.AsCSV() << "\n";
	}

	while( nrows-- ) {
		Row r = Get();
		if ( debug ) {
			DebugRow( r, std::cerr );
		}
		if ( ! HasGroup() ) {
			os << r.AsCSV() << "\n";
		}
		AddRow( r );
	}

	if ( HasGroup() ) {
		DoGroup();
		for ( int i = 0; i < Size(); i++ ) {
			os << RowAt(i).AsCSV() << "\n";
		}
	}

	if ( debug ) {
		std::cerr << "----- end   " << Name() << "\n";
	}
}

//----------------------------------------------------------------------------
// Should the output from this generator be displayed?
//----------------------------------------------------------------------------

bool GeneratorTag :: Hide() const {
	return mHide;
}
// build generator - needs uniqe name (checked by ModelBuilder)
// and optional count, which defaults to special "all rows value"
Generator * GeneratorTag :: FromXML( const ALib::XMLElement * e ) {

	RequireChildren( e );
	AllowAttrs( e, AttrList( NAME_ATTR, COUNT_ATTRIB, GROUP_ATTR,
								DEBUG_ATTRIB, HIDE_ATTR,
								OUT_ATTRIB, FNAMES_ATTR, 0 ) );
	string name = e->HasAttr( NAME_ATTR) ? e->AttrValue( NAME_ATTR ) : "";

	int count = GetCount( e );
	bool debug = GetBool( e, DEBUG_ATTRIB, NO_STR );
	FieldList grp( e->AttrValue( GROUP_ATTR, "" ));
	string ofn = GetOutputFile( e );
	string fields = e->AttrValue( FNAMES_ATTR, "" );
	std::auto_ptr <GeneratorTag> g(
		new GeneratorTag( name, count, debug, ofn, fields, grp )
	);
	g->AddSources( e );
	return g.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Generator" );

#endif

//----------------------------------------------------------------------------

// end

