//---------------------------------------------------------------------------
// dmk_many2many.cpp
//
// many to many source
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
#include "dmk_random.h"
#include "dmk_modman.h"
#include <set>

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------

const char * const M2M_TAG 	= "m2m";
const char * const LEFT_TAG 	= "left";
const char * const RIGHT_TAG 	= "right";
const char * const UNIQ_ATTR 	= "unique";

//----------------------------------------------------------------------------
// The base many to many class supports the idea of left & rigght data sources
// to construct the two sides if the m2m relationshipand the allow dupes flag
// though it doesn't implement the dupe detection.
//----------------------------------------------------------------------------

class ManyToMany : public CompositeDataSource {

	public:

		ManyToMany( const FieldList & order, bool dupes );
		~ManyToMany();

		static DataSource * FromXML( const ALib::XMLElement * e );

		// left and right side
		struct Side {
			Side( const std::string &  gname, const FieldList & fields )
				: mGenName( gname ), mGen( 0 ), mFields( fields ) {}

			std::string mGenName;
			mutable Generator * mGen;
			FieldList mFields;
		};

	protected:

		typedef std::auto_ptr <ManyToMany> M2MPtr;

		bool AllowDupes() const;
		const Side * Left() const;
		const Side * Right() const;

	private:

		void GetSides( const ALib::XMLElement * e );

		bool mAllowDupes;
		mutable Side * mLeft, * mRight;

};

//----------------------------------------------------------------------------
// Sequential many to many
//----------------------------------------------------------------------------

class SeqManyToMany : public ManyToMany {

	public:

		SeqManyToMany( const FieldList & order, bool dupes );
		Row Get();

	private:

		int mLPos, mRPos, mOut;
};

//----------------------------------------------------------------------------
// Randomised many to many
//----------------------------------------------------------------------------


class RandManyToMany : public ManyToMany {

	public:

		RandManyToMany( const FieldList & order, bool dupes );
		Row Get();

	private:

		// simple row comparison functor
		struct RowCmp {
			bool operator()( const Row & lhs, const Row & rhs ) {
				for ( unsigned int i = 0; i < lhs.Size(); i++) {
					if ( lhs.At(i) < rhs.At(i) ) {
						return true;
					}
					else if ( lhs.At(i) > rhs.At(i) ) {
						return false;
					}
				}
				return false;
			}
		};

		bool AddRow( const Row & r );
		std::set <Row, RowCmp> mRows;
};


//----------------------------------------------------------------------------
// Register the base class - the base creates the actual types.
//----------------------------------------------------------------------------

static RegisterDS <ManyToMany> regrs1_( M2M_TAG );

//----------------------------------------------------------------------------
// Dups flag indicates if duplicates are allowed
//----------------------------------------------------------------------------

ManyToMany :: ManyToMany( const FieldList & order, bool dupes )
	: CompositeDataSource( order ), mAllowDupes( dupes ),
			mLeft( 0 ), mRight( 0 ) {
}

//----------------------------------------------------------------------------
// Junk the left/right members - these will always be null or dynamically
// allocated.
//----------------------------------------------------------------------------

ManyToMany :: ~ManyToMany() {
	delete mLeft;
	delete mRight;
}

//----------------------------------------------------------------------------
// Helper to get a generator or throw
//----------------------------------------------------------------------------

static Generator * GenByName( const string & gname ) {
	Generator * gr = ModelManager::Instance()->FindGen( gname );
	if ( gr == 0 ) {
		throw Exception( "Unknown generator: " + gname );
	}
	return gr;
}

//----------------------------------------------------------------------------
// Are dupes allowed?
//----------------------------------------------------------------------------

bool ManyToMany :: AllowDupes() const {
	return mAllowDupes;
}

//----------------------------------------------------------------------------
// Get the left/right side of the many to many at run-time, getting the
// pointer to the correct generator of not already initialied.
//----------------------------------------------------------------------------

const ManyToMany::Side * ManyToMany :: Left() const {
	if ( mLeft->mGen == 0 ) {
		mLeft->mGen = GenByName( mLeft->mGenName );
	}
	return mLeft;
}

const ManyToMany::Side * ManyToMany :: Right() const {
	if ( mRight->mGen == 0 ) {
		mRight->mGen = GenByName( mRight->mGenName );
	}
	return mRight;
}

//----------------------------------------------------------------------------
// Create the left/right sides at compile time. at this point the generator
// pointers are not available.
//----------------------------------------------------------------------------

void ManyToMany::GetSides( const ALib::XMLElement * e ) {
	for ( unsigned int i = 0; i < e->ChildCount() ; i++ ) {
		const ALib::XMLElement * ce = e->ChildElement( i );
		if ( ce->Name() != LEFT_TAG && ce->Name() != RIGHT_TAG ) {
			XMLERR( ce, "Invalid tag name " << ce->Name() );
		}
		ForbidChildren( ce );
		string gen = ce->AttrValue( GEN_ATTRIB );
		FieldList fl = FieldList( ce->AttrValue( FIELDS_ATTRIB, "1" ) );
		if (  ce->Name() == LEFT_TAG ) {
			if ( mLeft != 0 ) {
				XMLERR( ce, "Duplicate " << LEFT_TAG << " tag" );
			}
			mLeft = new Side( gen, fl );
		}
		else {
			if ( mRight != 0 ) {
				XMLERR( ce, "Duplicate " << RIGHT_TAG << " tag" );
			}
			mRight = new Side( gen, fl );
		}
	}
}

//----------------------------------------------------------------------------
// Create the correct many to many type from XML, depending on the
// attributes. A many to many can only have two children - the left and
// right sources.
//----------------------------------------------------------------------------

DataSource * ManyToMany :: FromXML( const ALib::XMLElement * e ) {

	RequireChildCount( e, 2 );

	AllowAttrs( e, AttrList( ORDER_ATTRIB, UNIQ_ATTR, RANDOM_ATTRIB , 0 ) );

	bool unique = GetBool( e, UNIQ_ATTR, YES_STR );
	bool rand = GetBool( e, RANDOM_ATTRIB, YES_STR );

	M2MPtr mp;

	if ( rand ) {
		mp = M2MPtr( new RandManyToMany( GetOrder( e ), ! unique ));
	}
	else {
		mp = M2MPtr( new SeqManyToMany( GetOrder( e ), ! unique ));
	}

	mp->GetSides( e );

	return mp.release();
}

//----------------------------------------------------------------------------
// Sequential many to many simply loops through all possible left/right
// combinations. Useful for debugging.
//----------------------------------------------------------------------------

SeqManyToMany :: SeqManyToMany( const FieldList & order, bool dupes )
					: ManyToMany( order, dupes ),
					  mLPos(0), mRPos(0), mOut(0) {
}

//----------------------------------------------------------------------------
// Get left and right fields sequentially & concat them. If we don't allow
// dupes, we stop when the product of the left * right sources is exceeded.
//----------------------------------------------------------------------------

Row SeqManyToMany :: Get() {

	int lsize = Left()->mGen->Size();
	int rsize = Right()->mGen->Size();

	if ( mOut >=  lsize * rsize && ! AllowDupes() ) {
		throw Exception( "Duplicate in many to many" );
	}

	Row lr = Left()->mFields.OrderRow( Left()->mGen->RowAt( mLPos++ ) );
	Row rr = Right()->mFields.OrderRow( Right()->mGen->RowAt( mRPos++ ));

	mLPos %= lsize;
	mRPos %= rsize;
	mOut++;

	return lr.AppendRow( rr );

}

//----------------------------------------------------------------------------
// Generate randomised m any to many entries
//----------------------------------------------------------------------------

RandManyToMany :: RandManyToMany( const FieldList & order, bool dupes )
					: ManyToMany( order, dupes ) {
}

//----------------------------------------------------------------------------
// Create random row. If we are not allowing dupes, check it is unique and
// if not try again until retry limit is reached.
//----------------------------------------------------------------------------

Row RandManyToMany :: Get() {

	int lsize = Left()->mGen->Size();
	int rsize = Right()->mGen->Size();
	Row r;
	int tries = 20;

	while(1) {
		if ( ! AllowDupes() && tries-- < 0 ) {
			throw Exception( "Duplicate row in many to many" );
		}

		int li = RNG::Random( 0, lsize );
		int ri = RNG::Random( 0, rsize );
		r = Left()->mFields.OrderRow( Left()->mGen->RowAt( li ) );
		r.AppendRow(  Right()->mFields.OrderRow( Right()->mGen->RowAt( ri )) );

		if ( AllowDupes() || AddRow( r ) ) {
			break;
		}
	}

	return r;
}

//----------------------------------------------------------------------------
// Try to insert row indexes into set, returning success indicator.
//----------------------------------------------------------------------------

bool RandManyToMany :: AddRow( const Row & rs ) {
	return mRows.insert( rs ).second;
}

//----------------------------------------------------------------------------

} // namespace



