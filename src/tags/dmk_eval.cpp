//---------------------------------------------------------------------------
// dmk_eval.cpp
//
// MExpression evaluation for dmk3
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_expr.h"
#include "dmk_source.h"
#include "dmk_tagdict.h"
#include "dmk_xmlutil.h"
#include "dmk_strings.h"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const EVAL_TAG 			= "eval";
const char * const EXPR_ATTRIB			= "expr";
const char * const OUT_FUNC				= "out";

//----------------------------------------------------------------------------

class DSEval : public CompositeDataSource {

	public:

		DSEval( const FieldList & order );
		bool Compile( const std::string & expr );
		Row Get();

		static DataSource * FromXML( const ALib::XMLElement * e );

	private:

		ALib::Expression mExpr;

};

//----------------------------------------------------------------------------

static RegisterDS <DSEval> regrs1_( EVAL_TAG );

//----------------------------------------------------------------------------
// The Out() function adds its single parameter to the Output row, which is
// then output by Get().
//----------------------------------------------------------------------------

static Row Output;

std::string OutFunc( const std::deque <string> & params ) {
	Output.AppendValue( params[0] );
	return params[0];
}

static ALib::Expression::AddFunc af1_( OUT_FUNC, OutFunc, 1 );

//----------------------------------------------------------------------------

// standard ctor
DSEval :: DSEval( const FieldList & order )
	: CompositeDataSource( order ) {
}

// compiole to RPN
bool DSEval :: Compile( const string & expr )  {
	return mExpr.Compile( expr ) == "";
}

// pass input fields to expression to evaluate and ouput the resulting
// values which will be stored in the static Output vector
Row DSEval :: Get() {
	Row r = CompositeDataSource::Get();
	mExpr.ClearPosParams();
	Output = Row();
	for ( unsigned int i = 0; i < r.Size(); i++ ) {
		mExpr.AddPosParam( r.At(i) );
	}
	mExpr.Evaluate();
	return Output;
}

// create from xml
DataSource * DSEval :: FromXML( const ALib::XMLElement * e ) {
	RequireChildren( e );
	RequireAttrs( e, AttrList( EXPR_ATTRIB, 0 ) );
	AllowAttrs( e, AttrList( ORDER_ATTRIB, EXPR_ATTRIB, 0 ) );
	string expr = e->AttrValue( EXPR_ATTRIB, " " );
	std::auto_ptr <DSEval> eval( new DSEval( GetOrder( e ) ) );
	if ( ! eval->Compile( expr ) ) {
		throw XMLError( "Invalid expression", e );
	}
	eval->AddChildSources( e );
	return eval.release();
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef DMK_TEST

#include "a_myth.h"
using namespace ALib;
using namespace DMK;

DEFSUITE( "Eval" );

const char * const XML1 =
	"<eval expr=\"Out($1 + 2)\">\n"
		"<row values='7,2' />\n"
	"</eval>\n";

DEFTEST( Simple ) {
	XMLPtr xml( XML1 );
	DSEval * p = (DSEval *) DSEval::FromXML( xml );
	Row r = p->Get();
	FAILNE( r.Size(), 1 );
	FAILNE( r.At(0), "9" );
}
const char * const XML2 =
	"<eval expr=\"Out($1 + 2); Out( $2 + 3 )\">\n"
		"<row values='7,2' />\n"
	"</eval>\n";

DEFTEST( TwoOut ) {
	XMLPtr xml( XML2 );
	DSEval * p = (DSEval *) DSEval::FromXML( xml );
	Row r = p->Get();
	FAILNE( r.Size(), 2 );
	FAILNE( r.At(0), "9" );
	FAILNE( r.At(1), "5" );
}

#endif

//----------------------------------------------------------------------------

// end

