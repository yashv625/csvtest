//---------------------------------------------------------------------------
// dmk_random.h
//
// Random number generation for DMK.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_RANDOM_H
#define INC_DMK_RANDOM_H


namespace DMK {

//----------------------------------------------------------------------------
// Integer random number generator
//----------------------------------------------------------------------------

class RNG {
	public:
		static void Randomise( int n );
		static void Randomise();
		static int Random( int begin, int end );
		static int Random();
		static int GetSeed();

	private:
		static int mLastSeed;
		static bool mNeedRandomise;
};

//----------------------------------------------------------------------------
// Base for distribution classes
//----------------------------------------------------------------------------

class Distribution {

	public:

		virtual ~Distribution();
		virtual double NextReal() = 0;
		virtual int NextInt() = 0;

};

//----------------------------------------------------------------------------
// Triangular distribution
//----------------------------------------------------------------------------

class TriangleDist : public Distribution {

	public:

		TriangleDist( double begin, double mode, double end );
		~TriangleDist();

		double NextReal();
		int NextInt();

	private:

		struct TDImpl * mImpl;
};

//----------------------------------------------------------------------------
// Uniform distribution
//----------------------------------------------------------------------------

class UniformDist : public Distribution {

	public:

		UniformDist( double begin, double end );
		~UniformDist();

		double NextReal();
		int NextInt();

	private:

		struct UDImpl * mImpl;
};


//----------------------------------------------------------------------------


} // namespace

#endif

