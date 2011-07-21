//---------------------------------------------------------------------------
// dmk_row.h
//
// Row of CSV data
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_ROW_H
#define INC_DMK_ROW_H

#include "a_str.h"
#include "dmk_base.h"
#include <iosfwd>

namespace DMK {

//----------------------------------------------------------------------------
// A row is the basic DMK data structure, passed around between data sources
// and filters. As it is copied a lot, we make it reference counted.
//----------------------------------------------------------------------------

class Row {

	public:

		Row();
		explicit Row( const std::string & csv );
		Row( const Row & r );

		~Row();

		Row & operator = ( const Row & row );

		unsigned int Size() const;
		bool IsEmpty() const;

		const std::string & operator[] ( unsigned int  i ) const;
		const std::string & At( unsigned int  i ) const;

		Row & AppendValue( const std::string & val );
		Row & AppendCSV( const std::string & csv );
		Row & AppendRow( const Row & row );
		Row & AppendStrings( const Strings & cl );

		std::string AsCSV() const;

		void Erase( unsigned int col );

	private:

		mutable class RowRep * mRep;
		static int mInstCount;
};

//----------------------------------------------------------------------------
// Vectors of rows are used frequently.
//----------------------------------------------------------------------------

typedef std::vector <Row> Rows;

//----------------------------------------------------------------------------
// Comparison functions used by tags like unique to compare rows. We need
// to be able to compare rows wrt specific columns.
//----------------------------------------------------------------------------

bool operator < ( const Row & r1, const Row & r2 );
bool operator == ( const Row & r1, const Row & r2 );
bool operator != ( const Row & r1, const Row & r2 );
int Cmp( const Row & r1, const Row & r2, const class FieldList & fl );

//----------------------------------------------------------------------------
// Stream output for debug only.
//----------------------------------------------------------------------------

std::ostream & operator << ( std::ostream & os, const Row & row );

//----------------------------------------------------------------------------
// RowSource is an interface for classes that produce rows.
//----------------------------------------------------------------------------

class RowSource {

	public:

		RowSource();
		virtual ~RowSource();

		virtual Row Get() = 0;

};

//----------------------------------------------------------------------------

} // namespace







#endif

