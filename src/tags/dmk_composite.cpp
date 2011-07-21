//---------------------------------------------------------------------------
// dmk_composite.cpp
//
// Composite data source. This is just a wrapper for the base class.
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

const char * const COMPOSITE_TAG 			= "compose";

//----------------------------------------------------------------------------

class DSComposite : public CompositeDataSource {

	public:

		DSComposite( const FieldList & order = FieldList() );
		static DataSource * FromXML( const ALib::XMLElement * e );

};

static RegisterDS <DSComposite> regrs1_( COMPOSITE_TAG );

DSComposite :: DSComposite( const FieldList & order )
	: CompositeDataSource( order ) {
}


DataSource * DSComposite :: FromXML( const ALib::XMLElement * e ) {

	RequireChildren( e );
	AllowAttrs( e, ORDER_ATTRIB );
	std::auto_ptr <DSComposite> c( new DSComposite( GetOrder( e ) ) );
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

DEFSUITE( "DSComposite" );

const char * const XML1 =
	"<compose>\n"
		"<row values='1,2' />\n"
		"<row values='three' />\n"
	"</compose>\n";

DEFTEST( Composite ) {
	XMLPtr xml( XML1 );
	DSComposite * p = (DSComposite *) DSComposite::FromXML( xml );
	Row r = p->Get();
	FAILNE( r.Size(), 3 );
	FAILNE( r.At(1), "2" );
}

const char * const XML2 =
	"<compose>\n"
		"<compose>\n"
			"<row values='1,2' />\n"
			"<row values='three' />\n"
		"</compose>\n"
		"<row values='four' />\n"
	"</compose>\n";

DEFTEST( Nested ) {
	XMLPtr xml( XML2 );
	DSComposite * p = (DSComposite *) DSComposite::FromXML( xml );
	Row r = p->Get();
	FAILNE( r.Size(), 4 );
	FAILNE( r.At(3), "four" );
}

#endif

//----------------------------------------------------------------------------

// end

