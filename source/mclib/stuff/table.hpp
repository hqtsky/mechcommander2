//===========================================================================//
// File:	table.hh                                                         //
// Contents: Interface specification for Tables                              //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once

#ifndef _TABLE_HPP_
#define _TABLE_HPP_

#include <stuff/link.hpp>
#include <stuff/memoryblock.hpp>
#include <stuff/iterator.hpp>
#include <stuff/sortedsocket.hpp>

namespace Stuff
{

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TableEntry ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class Table;

class TableEntry : public Link
{
  public:
	TableEntry(Table* table, Plug* plug);
	~TableEntry(void);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TableEntryOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

enum
{
	TableEntry_Memoryblock_Allocation = 100
};

template <class V> class TableEntryOf : public TableEntry
{
  public:
	TableEntryOf(Table* table, Plug* plug, const V& value);
	~TableEntryOf(void);

	PVOID
	operator new(size_t);
	void operator delete(PVOID where);

	V GetValue() { return value; }
	V* GetValuePointer() { return &value; }

  private:
	static MemoryBlock* allocatedMemory;
	static CollectionSize allocationCount;
	V value;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~ TableEntryOf templates ~~~~~~~~~~~~~~~~~~~~~~~~

template <class V> MemoryBlock* TableEntryOf<V>::allocatedMemory   = nullptr;
template <class V> CollectionSize TableEntryOf<V>::allocationCount = 0;

template <class V>
TableEntryOf<V>::TableEntryOf(Table* table, Plug* plug, const V& value)
	: TableEntry(table, plug)
{
	this->value = value;
}

template <class V> TableEntryOf<V>::~TableEntryOf() {}

template <class V> PVOID TableEntryOf<V>::operator new(size_t)
{
	Verify(allocationCount >= 0);
	if (allocationCount++ == 0)
	{
		allocatedMemory = new MemoryBlock(sizeof(TableEntryOf<V>),
			TableEntry_Memoryblock_Allocation,
			TableEntry_Memoryblock_Allocation, "Stuff::TableEntryOf",
			Stuff::ConnectionEngineHeap);
		Register_Object(allocatedMemory);
	}
	Verify(allocationCount < INT_MAX);
	Check_Object(allocatedMemory);
	return allocatedMemory->New(void);
}

template <class V> void TableEntryOf<V>::operator delete(PVOID where)
{
	Check_Object(allocatedMemory);
	allocatedMemory->Delete(where);
	if (--allocationCount == 0)
	{
		Unregister_Object(allocatedMemory);
		delete allocatedMemory;
		allocatedMemory = nullptr;
	}
	Verify(allocationCount >= 0);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const IteratorPosition TableNullIndex = IteratorPosition(-1);

class Table : public SortedSocket
{
	friend class TableEntry;
	friend class TableIterator;

  public:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Public interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//

	//
	//--------------------------------------------------------------------
	// Constructor, Destructor and testing
	//--------------------------------------------------------------------
	//
	Table(Node* node, bool has_unique_entries);
	~Table(void);

	void TestInstance(void);
	static bool TestClass(void);
	static bool ProfileClass(void);

	//
	//-----------------------------------------------------------------------
	// IsEmpty - Returns true if the socket contains no plugs.
	//-----------------------------------------------------------------------
	//
	bool IsEmpty(void);

  protected:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Protected interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//
	void AddImplementation(Plug* plug);
	void AddValueImplementation(Plug* plug, PCVOID value);
	Plug* FindImplementation(PCVOID value);
#if 0
		Plug
		* FindCloseImplementation(PCVOID value);
#endif

  private:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Private interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//
	virtual TableEntry* MakeTableEntry(Plug* plug, PCVOID value);

	virtual IteratorPosition CompareTableEntries(
		TableEntry* entry1, TableEntry* entry2);

	virtual IteratorPosition CompareValueToTableEntry(
		PCVOID value, TableEntry* entry);

	void AddTableEntry(TableEntry* entry);
	void SortTableEntries(void);
	IteratorPosition SearchForValue(PCVOID value);
#if 0
		IteratorPosition
		SearchForCloseValue(PCVOID value);
#endif
	IteratorPosition SearchForTableEntry(TableEntry* entry);
	void RemoveNthTableEntry(CollectionSize index);

	//
	//--------------------------------------------------------------------
	// Private data
	//--------------------------------------------------------------------
	//
	TableEntry** array;
	CollectionSize numItems, maxItems;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TableOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template <class T, class V> class TableOf : public Table
{
  public:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Public interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//
	TableOf(Node* node, bool has_unique_entries);
	~TableOf(void);

	//
	//--------------------------------------------------------------------
	// Socket methods (see Socket for full listing)
	//--------------------------------------------------------------------
	//
	void AddValue(T plug, const V& value)
	{
		AddValueImplementation(Cast_Object(Plug*, plug), &value);
	}
	void Remove(T plug) { RemovePlug(Cast_Object(Plug*, plug)); }
	T Find(const V& value) { return (T)FindImplementation(&value); }
#if 0
		T
		FindClose(const V& value)
		{
			return (T)FindCloseImplementation(&value);
		}
#endif

  private:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Private interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//
	TableEntry* MakeTableEntry(Plug* plug, PCVOID value)
	{
		return new TableEntryOf<V>(this, plug, *Cast_Pointer(const V*, value));
	}
	IteratorPosition CompareTableEntries(
		TableEntry* entry1, TableEntry* entry2);
	IteratorPosition CompareValueToTableEntry(PCVOID value, TableEntry* entry);
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TableOf templates ~~~~~~~~~~~~~~~~~~~~~~~~~~~

template <class T, class V>
TableOf<T, V>::TableOf(Node* node, bool has_unique_entries)
	: Table(node, has_unique_entries)
{
}

template <class T, class V> TableOf<T, V>::~TableOf() {}

template <class T, class V>
IteratorPosition TableOf<T, V>::CompareTableEntries(
	TableEntry* entry1, TableEntry* entry2)
{
	V* ptr1 = Cast_Object(TableEntryOf<V>*, entry1)->GetValuePointer(void);
	V* ptr2 = Cast_Object(TableEntryOf<V>*, entry2)->GetValuePointer(void);
	Check_Pointer(ptr1);
	Check_Pointer(ptr2);
	if (*ptr1 == *ptr2)
		return 0;
	else
		return IteratorPosition((*ptr1 > *ptr2) ? 1 : -1);
}

template <class T, class V>
IteratorPosition TableOf<T, V>::CompareValueToTableEntry(
	PCVOID value, TableEntry* link)
{
	Check_Pointer(value);
	V* ptr = Cast_Object(TableEntryOf<V>*, link)->GetValuePointer(void);
	Check_Pointer(ptr);
	if (*Cast_Pointer(const V*, value) == *ptr)
		return 0;
	else
		return IteratorPosition(
			(*Cast_Pointer(const V*, value) > *ptr) ? 1 : -1);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TableIterator ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class TableIterator : public SortedIterator
{
  public:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Public interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//

	//
	//--------------------------------------------------------------------
	// Constructors, Destructor and testing
	//--------------------------------------------------------------------
	//
	explicit TableIterator(Table* table);
	~TableIterator(void);
	void TestInstance(void);

	//
	//--------------------------------------------------------------------
	// Iterator methods (see Iterator for full listing)
	//--------------------------------------------------------------------
	//
	void First(void);
	void Last(void);
	void Next(void);
	void Previous(void);
	CollectionSize GetSize(void);
	void Remove(void);

  protected:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Protected interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//
	PVOID ReadAndNextImplementation(void);
	PVOID ReadAndPreviousImplementation(void);
	PVOID GetCurrentImplementation(void);
	PVOID GetNthImplementation(CollectionSize index);
	Plug* FindImplementation(PCVOID value);

	TableEntry* GetCurrentEntry() { return NthEntry(currentPosition); }

  private:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Private interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//
	void ReceiveMemo(IteratorMemo memo, PVOID content);

	TableEntry* NthEntry(CollectionSize index)
#if defined(_ARMOR)
		;
#else
	{
		return array[index];
	}
#endif

	void IncrementPosition()
	{
		if (++currentPosition >= numItems)
			currentPosition = TableNullIndex;
	}

	void DecrementPosition()
	{
		if (intptr_t(--currentPosition) < 0)
			currentPosition = TableNullIndex;
	}

	//
	//--------------------------------------------------------------------
	// Private data
	//--------------------------------------------------------------------
	//
	TableEntry** array;
	CollectionSize numItems;
	IteratorPosition currentPosition;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~ TableIteratorOf ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template <class T, class V> class TableIteratorOf : public TableIterator
{
  public:
	//
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	// Public interface
	//--------------------------------------------------------------------
	//--------------------------------------------------------------------
	//

	//
	//--------------------------------------------------------------------
	// Constructors and Destructor
	//--------------------------------------------------------------------
	//
	explicit TableIteratorOf(TableOf<T, V>* table);
	Iterator* MakeClone(void);
	~TableIteratorOf(void);

	//
	//--------------------------------------------------------------------
	// Iterator methods (see Iterator for full listing)
	//--------------------------------------------------------------------
	//
	T ReadAndNext() { return (T)ReadAndNextImplementation(void); }

	T ReadAndPrevious() { return (T)ReadAndPreviousImplementation(void); }

	T GetCurrent() { return (T)GetCurrentImplementation(void); }

	T GetNth(CollectionSize index) { return (T)GetNthImplementation(index); }

	T Find(const V& value) { return (T)FindImplementation(&value); }

	V GetValue()
	{
		return Cast_Object(TableEntryOf<V>*, GetCurrentEntry())->GetValue(void);
	}
};

//~~~~~~~~~~~~~~~~~~~~~~~~ TableIteratorOf templates ~~~~~~~~~~~~~~~~~~~~~~~

template <class T, class V>
TableIteratorOf<T, V>::TableIteratorOf(TableOf<T, V>* table)
	: TableIterator(table)
{
}

template <class T, class V> Iterator* TableIteratorOf<T, V>::MakeClone()
{
	return new TableIteratorOf<T, V>(*this);
}

template <class T, class V> TableIteratorOf<T, V>::~TableIteratorOf() {}
}
#endif
