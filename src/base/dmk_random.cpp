//---------------------------------------------------------------------------
// dmk_random.cpp
//
// random number generation for dmk
// we now use boost to supply random numbers & distributions
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "dmk_base.h"
#include "dmk_random.h"

#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include "boost/shared_ptr.hpp"

using std::string;
using std::vector;

namespace DMK {

//----------------------------------------------------------------------------
// Global random number generator
//----------------------------------------------------------------------------

typedef boost::minstd_rand RNGType;
static RNGType theGen;

//----------------------------------------------------------------------------
// Default RNG seed
//----------------------------------------------------------------------------

int RNG::mLastSeed = 350891;
bool RNG::mNeedRandomise = true;

//----------------------------------------------------------------------------
// Randomise generator with value N
//----------------------------------------------------------------------------

void RNG :: Randomise( int n ) {
	mNeedRandomise = false;
	mLastSeed = n;
	theGen.seed( n );
	theGen();
}

//----------------------------------------------------------------------------
// Ensure RNG is randomised
//----------------------------------------------------------------------------

void RNG::Randomise() {
	if ( mNeedRandomise  ) {
		Randomise( mLastSeed );
	}
}

//----------------------------------------------------------------------------
// Get random number in range [begin,end) - not used much in current code
// which uses distributions.
//----------------------------------------------------------------------------

int RNG :: Random( int begin, int end ) {
	if ( mNeedRandomise  ) {
		Randomise( mLastSeed );
	}

	if ( begin >= end ) {
		throw Exception( "Invalid random number range" );
	}
	int n = end - begin;
	const int bsize = INT_MAX / n;
	int r;
	do {
		r = theGen() / bsize;
	} while( r >= n );
	return begin + r;
}

//----------------------------------------------------------------------------
// Get raw random number.
//----------------------------------------------------------------------------

int RNG :: Random() {
	if ( mNeedRandomise ) {
		Randomise( mLastSeed );
	}
	return theGen();
}

//----------------------------------------------------------------------------
// Get last number used to seed RNG.
//----------------------------------------------------------------------------

int RNG :: GetSeed() {
	return mLastSeed;
}

//----------------------------------------------------------------------------
// Implementation of trianguular distribution
// These should probably be templated
//----------------------------------------------------------------------------

struct TDImpl {

	typedef boost::triangle_distribution <double> DistType;
	typedef boost::variate_generator<RNGType&, DistType> GenType;
	typedef boost::generator_iterator<GenType> IterType;

	GenType mGen;
	IterType mIter;

	TDImpl( double begin, double mode, double end )
		: mGen( theGen, DistType( begin, mode, end ) ), mIter( &mGen ) {
	}

	double Next() {
		return * mIter++;
	}
};

//----------------------------------------------------------------------------
// Implementation of uniform distribution
//----------------------------------------------------------------------------

struct UDImpl {

	typedef boost::uniform_real <double> DistType;
	typedef boost::variate_generator<RNGType&, DistType> GenType;
	typedef boost::generator_iterator<GenType> IterType;

	GenType mGen;
	IterType mIter;

	UDImpl( double begin, double end )
		: mGen( theGen, DistType( begin, end ) ), mIter( &mGen ) {
	}

	double Next() {
		return * mIter++;
	}
};



//----------------------------------------------------------------------------
// Base virtual dtor
//----------------------------------------------------------------------------

Distribution :: ~Distribution() {
	// nothing
}

//----------------------------------------------------------------------------
// Triangle distribution uses a mode to skew the distribution
//----------------------------------------------------------------------------

TriangleDist :: TriangleDist( double begin, double mode, double end )
		: mImpl( new TDImpl( begin, mode, end ) ) {
}

TriangleDist :: ~TriangleDist() {
	delete mImpl;
}

double TriangleDist :: NextReal() {
	RNG::Randomise();
	return mImpl->Next();
}

int TriangleDist :: NextInt() {
	RNG::Randomise();
	return int( mImpl->Next() );
}

//----------------------------------------------------------------------------
// Uniform distributions spreads values across range
//----------------------------------------------------------------------------

UniformDist :: UniformDist( double begin, double end )
		: mImpl( new UDImpl( begin,  end ) ) {
}

UniformDist :: ~UniformDist() {
	delete mImpl;
}

double UniformDist ::NextReal() {
	RNG::Randomise();
	return mImpl->Next();
}

int UniformDist ::NextInt() {
	RNG::Randomise();
	return int( mImpl->Next() );
}



//----------------------------------------------------------------------------

} // namespace


// end

