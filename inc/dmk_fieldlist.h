//---------------------------------------------------------------------------
// dmk_fieldlist.h
//
// Field list is a list of row field indexes. Yhe stored indexes are
// zero-based, but the input indexes, in the form of comma-separated
// lists are 1-based.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_FIELDLIST_H
#define INC_DMK_FIELDLIST_H

#include "dmk_base.h"
#include "dmk_row.h"

namespace DMK {

//----------------------------------------------------------------------------

class FieldList {

	public:

		explicit FieldList( const std::string & s = "" );
		~FieldList();

		void Clear();

		unsigned int Size() const;
		unsigned int At( unsigned int i ) const;// returns zero based value

		bool Contains( unsigned int idx ) const;

		Row OrderRow( const Row & r, const std::string & emptyval = "" ) const;

	private:

		std::vector <unsigned int> mFields;

};

//----------------------------------------------------------------------------

class FieldPairs {

	public:

		FieldPairs( const std::string & fp = "" );

		unsigned int Size() const;

		unsigned int AtLeft( unsigned int i ) const;
		unsigned int AtRight( unsigned int i ) const;

	private:

		std::vector <unsigned int > mLeft, mRight;

};

//----------------------------------------------------------------------------

} // namespace

#endif

