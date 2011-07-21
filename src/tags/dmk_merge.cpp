//---------------------------------------------------------------------------
// dmk_merge.cpp
//
// Merge all fields into single field
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const MERGE_TAG 			= "merge";
const char * const SEP_ATTRIB 			= "sep";

//----------------------------------------------------------------------------

class DSMerge : public CompositeDataSource {

	public:

		DSMerge( const FieldList & order, const std::string & sep  );

		Row Get();

		static DataSource * FromXML( const ALib::XMLElement * e );


	private:

		std::string mSep;

};

//----------------------------------------------------------------------------
// Register tag
//----------------------------------------------------------------------------

static RegisterDS <DSMerge> regrs1_( MERGE_TAG );

//----------------------------------------------------------------------------
// sep is separator between fields
//----------------------------------------------------------------------------

DSMerge :: DSMerge( const FieldList & order, const string & sep  )
	: CompositeDataSource( order ), mSep( sep ) {
}

//----------------------------------------------------------------------------
// get a row and merge all fields
//----------------------------------------------------------------------------

Row DSMerge :: Get() {
	Row r = CompositeDataSource::Get();
	string s;
	for ( unsigned int i = 0; i < r.Size() ; i++ ) {
		if ( s != "" && r.At(i) != "" ) {
			s += mSep;
		}
		if ( r.At(i) != "" ) {
			s += r.At( i );
		}
	}
	Row rv;		// remember Row(s) will treat s as csv!
	rv.AppendValue( s );
	return Order( rv );
}

//----------------------------------------------------------------------------
// create from xml
//----------------------------------------------------------------------------

DataSource * DSMerge :: FromXML( const ALib::XMLElement * e ) {
	RequireChildren( e );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, SEP_ATTRIB, 0 ) );
	string sep = e->AttrValue( SEP_ATTRIB, " " );
	std::auto_ptr <DSMerge> c( new DSMerge( GetOrder( e ), sep ) );
	c->AddChildSources( e );
	return c.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Merge" );

const char * const XML1 =
	"<merge>\n"
		"<row values='1,2' />\n"
		"<row values='three' />\n"
	"</merge>\n";

DEFTEST( Simple ) {
	XMLPtr xml( XML1 );
	DSMerge * p = (DSMerge *) DSMerge::FromXML( xml );
	Row r = p->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "1 2 three" );
}


#endif

//----------------------------------------------------------------------------

// end

