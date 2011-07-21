//---------------------------------------------------------------------------
// dmk_model.cpp
//
// The model describes how DMK generates data
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_collect.h"
#include "a_str.h"
#include "dmk_base.h"
#include "dmk_model.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"
#include <memory>
#include <algorithm>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Model entry is abstract base for generators & formatters
//----------------------------------------------------------------------------

ModelEntry :: ModelEntry() {
}

ModelEntry :: ~ModelEntry() {
}


//----------------------------------------------------------------------------
// Echoer is used to output literal text
//----------------------------------------------------------------------------

Echoer :: Echoer( const std::string & text ) : mText( text ) {
}

Echoer :: ~Echoer() {
}

void Echoer :: Generate( class Model * model ) {
	model->DefaultOutput() << mText << std::endl;
}

void Echoer :: Discard() {
}

//----------------------------------------------------------------------------
// create named generator with debug on/off
//----------------------------------------------------------------------------

Generator :: Generator( const string & name, bool debug, const FieldList & grp )
	: mName( name ), mDebug( debug ), mGroup( grp ) {
}

//----------------------------------------------------------------------------
// free up all sources & rows
//----------------------------------------------------------------------------

Generator :: ~Generator() {
	mRows.clear();
	ALib::FreeClear( mSources );
}

//----------------------------------------------------------------------------
// Name for the generator - must be unique or empty
//----------------------------------------------------------------------------

string Generator :: Name() const {
	return mName;
}

//----------------------------------------------------------------------------
// Handle grouping of output.
//----------------------------------------------------------------------------

struct Grouper {

	Grouper( const FieldList & fl ) : mFields( fl ) {}
	bool operator()( const Row & lhs, const Row & rhs ) const {
		return Cmp( lhs, rhs, mFields ) < 0;
	}
	FieldList mFields;
};


bool Generator :: HasGroup() const {
	return mGroup.Size() > 0;
}

void  Generator :: DoGroup()  {
	if ( mGroup.Size() == 0 ) {
		throw Exception( "No group specified" );
	}
	Grouper g( mGroup );
	std::sort( mRows.begin(), mRows.end(), g );
}

//----------------------------------------------------------------------------
// these are used after generation to access the generated rows
//----------------------------------------------------------------------------

int Generator :: Size() const {
	return mRows.size();
}

Row Generator :: RowAt( int i ) const {
	return mRows.at( i );
}


//----------------------------------------------------------------------------
// Add source - adding duplicate sources is wrong but not checked
//----------------------------------------------------------------------------

void Generator :: AddSource( DataSource * s ) {
	mSources.push_back( s );
}

//----------------------------------------------------------------------------
// Access top-level datat sources
//----------------------------------------------------------------------------

unsigned int  Generator :: SourceCount() const {
	return mSources.size();
}

DataSource * Generator :: SourceAt( unsigned int i ) const {
	return mSources.at( i );
}

//----------------------------------------------------------------------------
// Add all sources specified by kids of an xml element
//----------------------------------------------------------------------------

void Generator :: AddSources( const ALib::XMLElement * e ) {
	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		const ALib::XMLElement * ce = e->ChildElement( i );
		if ( ce ) {
			DataSource * ds  = TagDictionary::Instance()->CreateDS( ce );
			AddSource ( ds );
		}
	}
}

//----------------------------------------------------------------------------
// get single row by concating rows from sources
//----------------------------------------------------------------------------

Row Generator :: Get() {
	Row r;
	for ( unsigned int i = 0; i < SourceCount(); i++ ) {
		r.AppendRow( SourceAt( i )->Get() );
	}
	return r;
}

//----------------------------------------------------------------------------
// Work out the number of rows to produce if the user doesn't specify
// to do this we recursively send a "size" message to all kids
// and use the maximum returned value as our size
//----------------------------------------------------------------------------

int Generator:: GetSize() {
	int sz = 0;
	for ( unsigned int i = 0; i < SourceCount() ; i++ ) {
		int t = SourceAt(i)->Size();
		sz = std::max( sz, t );
	}
	return sz;
}

//----------------------------------------------------------------------------
// discard is used to inform sources that they may release any
// intermediate results they are hanging on to. it should be
// called after a generator has built its row set
//----------------------------------------------------------------------------

void Generator :: Discard() {
	for ( unsigned int i = 0; i < SourceCount() ; i++ ) {
		SourceAt(i)->Discard();
	}
}

//----------------------------------------------------------------------------
// Add row, checking for unique name done in builder
//----------------------------------------------------------------------------

void Generator :: AddRow( const Row & row )  {
	mRows.push_back( row );
}

//----------------------------------------------------------------------------
// Output a row in debug format on stream
// format is [field:value] where field is 1-based index
//----------------------------------------------------------------------------

void Generator :: DebugRow( const Row & row, std::ostream & os ) {
	for ( unsigned int i = 0; i < row.Size() ; i++ ) {
		os << "[" << (i + 1) << ":" << row.At(i) << "]";
	}
	os << std::endl;
}

//----------------------------------------------------------------------------
// local debug flag
//----------------------------------------------------------------------------

bool Generator :: Debug() const {
	return mDebug;
}


//----------------------------------------------------------------------------
// model has unique name - checked by model manager
//----------------------------------------------------------------------------

Model :: Model( const string & name, std::ostream & defout )
	: mName( name ), mDefOut( defout ) {
}

//----------------------------------------------------------------------------
// junk the generators
//----------------------------------------------------------------------------

Model :: ~Model() {
	Clear();
}

//----------------------------------------------------------------------------
// Get default output stream
//----------------------------------------------------------------------------

std::ostream & Model :: DefaultOutput() const {
	return mDefOut;
}

//----------------------------------------------------------------------------
// get model name
//----------------------------------------------------------------------------

string Model :: Name() const {
	return mName;
}

//----------------------------------------------------------------------------
// global debug status
//----------------------------------------------------------------------------

bool Model :: Debug() const {
	return true;
}

//----------------------------------------------------------------------------
// free the generators which we own
//----------------------------------------------------------------------------

void Model :: Clear() {
	ALib::FreeClear( mEntries );
}

//----------------------------------------------------------------------------
// add generator, taking ownership
//----------------------------------------------------------------------------

void Model :: AddGen( Generator * me ) {
	mEntries.push_back( me );
}

//----------------------------------------------------------------------------
// find generator by name - names are case sensitve
//----------------------------------------------------------------------------

Generator * Model :: FindGen( const std::string & name ) const {
	for ( unsigned int i = 0; i < EntryCount() ; i++ ) {
		Generator * gp = GenAt( i );
		if ( gp && gp->Name() == name ) {
			return gp;
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// if indexed entry is generator return pointer else null
//----------------------------------------------------------------------------

Generator * Model :: GenAt( unsigned int i ) const {
	Generator * gp =  dynamic_cast <Generator *>( mEntries.at( i ));
	return gp;
}


//----------------------------------------------------------------------------
// Manage entries
//----------------------------------------------------------------------------

unsigned int Model :: EntryCount() const {
	return mEntries.size();
}

void Model :: AddEntry( ModelEntry * e ) {
	mEntries.push_back( e );
}

ModelEntry * Model :: EntryAt( unsigned int i ) const {
	return mEntries.at( i );
}


//----------------------------------------------------------------------------
// Generate all data for model by calling generate on every generator
// once rows are generated, any intermediate results are discarded
//----------------------------------------------------------------------------

void Model :: Generate() {
	for ( unsigned int i = 0; i < EntryCount(); i++ ) {
		EntryAt( i )->Generate( this );
		EntryAt( i )->Discard();
	}
}


//----------------------------------------------------------------------------
// add name/value pair - name meay already exist
//----------------------------------------------------------------------------

void Model :: AddDef( const string & name, const string & val ) {
	mDict.Replace( name, val );
}

//----------------------------------------------------------------------------
// getv defined value or rthrow if does not exist
//----------------------------------------------------------------------------

string Model :: GetDefValue( const string & name ) const {
	const string * s = mDict.GetPtr( name );
	if ( s == 0 ) {
		throw Exception( "Unknown define: " + ALib::SQuote( name ) );
	}
	return * s;
}

//----------------------------------------------------------------------------
// Model builder
//----------------------------------------------------------------------------

ModelBuilder :: ModelBuilder() {
}

ModelBuilder ::  ~ModelBuilder() {
}

//----------------------------------------------------------------------------
// Build a model from an XML tree, returning pointer to new model
// the caller is responsible for managing the returned model
//----------------------------------------------------------------------------

Model * ModelBuilder :: Build( const ALib::XMLElement * tree ) {

	string name = tree->AttrValue( NAME_ATTRIB, "" );
	std::auto_ptr <Model> model( new Model( name, std::cout ) );

	for ( unsigned int i = 0; i < tree->ChildCount(); i++ ) {
		const ALib::XMLElement * ce = tree->ChildElement( i );
		if ( ce ) {
			if ( ce->Name() == DMKGEN_TAG) {
				BuildGenerator( model.get(), ce );
			}
			else if ( ce->Name() == DMKECHO_TAG ) {
				BuildEchoer( model.get(), ce );
			}
			else if ( ce->Name() == DMKDEF_TAG ) {
				AddDefine( model.get(), ce );
			}
			else {
				throw Exception( "Invalid top-level tag "
									+ ALib::SQuote( ce->Name() ) );
			}
		}
	}
	return model.release();
}

//----------------------------------------------------------------------------
// build a single uniquely named generator & add it to the model
//----------------------------------------------------------------------------

void ModelBuilder :: BuildGenerator( Model * model,
										const ALib::XMLElement * e ) {
	Generator * g = TagDictionary::Instance()->CreateGen( e );

	if ( (! ALib::IsEmpty( g->Name() )) &&  model->FindGen( g->Name() )) {
		throw XMLError( "duplicate name " + ALib::SQuote( g->Name() ), e );
	}
	model->AddGen( g );
}


//----------------------------------------------------------------------------
// add define to model
//----------------------------------------------------------------------------

void ModelBuilder :: AddDefine( Model * model, const ALib::XMLElement * e ) {
	RequireAttrs( e, AttrList( NAME_ATTRIB, VALUE_ATTRIB, 0 ) );
	ForbidChildren( e );
	string name = e->AttrValue( NAME_ATTRIB );
	if ( ALib::IsEmpty( name ) ) {
		throw XMLError( "'name' attribute cannot be empty", e );
	}
	string value = e->AttrValue( VALUE_ATTRIB );
	model->AddDef( name, value );
}

//----------------------------------------------------------------------------
// Build an echoer from the text content of the tag
//----------------------------------------------------------------------------

void ModelBuilder :: BuildEchoer( Model * model, const ALib::XMLElement * e ) {
	AllowAttrs( e, AttrList( 0 ) );
	string text = ChildText( e );
	model->AddEntry( new Echoer( text ) );
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------
// Testing
//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
#include "a_xmlparser.h"

using namespace ALib;
using namespace DMK;

DEFSUITE( "Model" );

// very simple model for test
const char * const XML1 =
	"<csvt>\n"
		"<def name=\"meaning\" value=\"42\" />\n"
		"<gen name='zod' debug='yes'>\n"
			"<row values='foo'/>\n"
		"</gen>\n"
	"</csvt>\n";

static XMLElement * GetTree1() {
	XMLTreeParser tp;
	XMLElement * e = tp.Parse( XML1 );
	STOPEQ( e, 0 );
	return e;
}

DEFTEST( Buld1 ) {

	Model * m = 0;
	ModelBuilder mb;
	XMLElement * e = GetTree1();
	m = mb.Build( e );
	FAILEQ( m, 0 );
	FAILNE( m->EntryCount(), 1 );
	TestStream ts;
	m->Generate();
	FAILNE( ts.Size(), 3 );
	FAILNE( ts.At(1), "[1:foo]" );
	string mv = m->GetDefValue( "meaning" );
	FAILNE( mv, "42" );

}



#endif

//----------------------------------------------------------------------------

// end

