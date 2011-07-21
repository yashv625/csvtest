//---------------------------------------------------------------------------
// dmk_source.h
//
// Base classes for all data sources These are:
//
//	- DataSource
//	- CompositeDataSource
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_DMK_SOURCE_H
#define INC_DMK_SOURCE_H

#include "dmk_base.h"
#include "dmk_row.h"
#include "dmk_fieldlist.h"

namespace DMK {

//----------------------------------------------------------------------------
// Base for root & composite data sources
//----------------------------------------------------------------------------

class DataSource : public RowSource {

	CANNOT_COPY( DataSource );

	public:

		DataSource( const FieldList & order = FieldList() );

		virtual Row Get() = 0;
		virtual int Size() = 0;
		virtual void Reset() = 0;
		virtual void Discard();

		Row Last() const;

	protected:

		Row SaveLast( const Row & row ) const;
		Row Order( const Row & row ) const;

	private:

		FieldList mOrder;
		mutable Row mLast;

};

//----------------------------------------------------------------------------
// Utility class
//----------------------------------------------------------------------------

class SourceList {

	public:

		SourceList();
		virtual ~SourceList();

		void Add( DataSource * ds );
		void Clear();

		unsigned int Size() const;
		const DataSource * At( unsigned int i ) const;
		DataSource * At( unsigned int i );

	private:

		mutable std::vector <DataSource *> mSources;
};

//----------------------------------------------------------------------------
// Composite of several child data sources
//----------------------------------------------------------------------------

class CompositeDataSource : public DataSource{

	public:

		CompositeDataSource( const FieldList & order = FieldList() );
		~CompositeDataSource();

		void AddSource( DataSource * src );
		unsigned int SourceCount() const;
		DataSource * SourceAt( unsigned int i ) const;

		Row Get();
		int Size();
		void Discard();
		void Reset();

		virtual void AddChildSources( const ALib::XMLElement * e );

	private:

		std::vector <DataSource *> mSources;
};

//----------------------------------------------------------------------------
// Base class for composite sources that must create & store at least
// one  result set.
//----------------------------------------------------------------------------

class Intermediate : public CompositeDataSource {

	public:

		Intermediate( const FieldList & order, bool rand  );
		~Intermediate();

		Row Get();
		int Size();
		void Discard();

		virtual void Populate() = 0;

	protected:

		void AddRow( const Row & row );
		const Rows & ResultRows() const;

	private:

		void SafePopulate();

		Rows mRows;

		bool mRand;
		unsigned int mPos;
};

//----------------------------------------------------------------------------

} // namespace

#endif

