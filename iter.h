#pragma once

#include <stdlib.h>
#include <stdio.h>

#ifndef DEFAULT_MAX_ITER_LEN
#define DEFAULT_MAX_ITER_LEN -1	// Negative == unlimited
#endif

// TODO Функциональный метод итерации, приспосабливаемый к любым типам данных

#define malloc_type(type) (type*)malloc(sizeof(type))

#define next_iteration_of_type(iter, type) (type*)nextIterator(iter)

/*	Usage:

	Iterator* it = getIterator(i);
	while (it->remains)
	{
		Type* obj = next_iteration_of_type(it, Type);
	}

*/

// --------------------------------------------------------------- Type definitions -- //

typedef void* data_t;

// Stack chain of void* data
typedef struct IterElem
{
	struct IterElem* 	next;
	data_t 				data;
}
IterElem;	// ie

// Head of the iterator stack
// All functions are called upon it
typedef struct
{
	IterElem* 			first;
	IterElem*			last;	// fast access to addition of new elem
	IterElem*			penult;	// fast access to addition of new elem
	int32_t 			len;
	int32_t 			limit;	// if necessary, you can set a limit on the max amount of elements
}
Iterable;	// i

// Iteration is done via this
typedef struct
{
	IterElem* 			next;
	int32_t 			remains;
}
Iterator;	// it

// ------------------------------------------------------------ Function signatures -- //
// TODO delIndexIter()
// TODO pullIter()

Iterable* 	newIter		();
   _Bool 	addIter		(Iterable* i,	data_t data );
  data_t 	popIter		(Iterable* i);						// Stack-like pop from top
  data_t 	indexIter 	(Iterable* i,  int32_t idx  );		// List-like indexed retrieval
	void 	clearIter 	(Iterable* i);
	void 	printIter 	(Iterable* i);

// Iteration functions
Iterator* 	getIterator	(Iterable* i);						// Get iterator from iterable
  data_t 	nextIterator(Iterator* it);

#define     _S 	static 	// Internal realisation
_S  void 	_clearIter 	(IterElem* ie);
_S  void 	_printIter 	(IterElem* i, int32_t idx);
_S IterElem*_indexIterElem 		(Iterable* i,  uint32_t idx);
_S IterElem*_indexElemRecursive	(IterElem* ie, uint32_t cur, uint32_t idx);
#undef _S

// ------------------------------------------------------- Function implementations -- //

Iterable* newIter()	// New iterable
{
	Iterable* new = malloc_type(Iterable);
	new->first  	= NULL;
	new->last   	= NULL;
	new->penult 	= NULL;
	new->len 		= 0;
	new->limit		= DEFAULT_MAX_ITER_LEN;

	return new;
}

IterElem* newIterElem(data_t data)
{
	IterElem* new = malloc_type(IterElem);
	new->next 		= NULL;
	new->data 		= data;

	return new;
}

Iterator* getIterator(Iterable* i)
{
	Iterator* new = malloc_type(Iterator);
	new->next 	 	= i->first;
	new->remains 	= i->len;

	return new;
}

data_t nextIterator(Iterator* it)
{
	if (it->next == NULL)
		return NULL;	// Iterator exit, no more elems left

	data_t return_data = it->next->data;
	it->next = it->next->next;

	it->remains -= 1;
	return return_data;
}

// Returns false if operation isn't successful
_Bool addIter(Iterable* i, data_t data)
{
	if (i->limit > 0 && i->len + 1 > i->limit)
		return false;

	if (i->first == NULL) {
		i->first = newIterElem(data);
		i->last  = i->first;
	}
	else {
		i->last->next = newIterElem(data);
		if (i->len > 1) i->penult = i->last;
		i->last = i->last->next;
	}

	i->len += 1;
	return true;
}

// Pop the last element from Iter
// WARNING! free() on data should be called manually
data_t popIter(Iterable* i)
{
	if (i->last == NULL)
		return NULL;	// Nothing to pop

	data_t return_data = i->last->data;
	free(i->last);
	i->len -= 1;

	if (i->penult == NULL) {
		i->first = NULL;
	} else {
		IterElem* elem = _indexIterElem(i, i->len);
		elem->next = NULL;
		i->penult = elem;
	}

	return return_data;
}

// Pull the first element from Iter
// WARNING! free() on data should be called manually
data_t pullIter(Iterable* i)
{
	if (i->first == NULL)
		return NULL;

	data_t return_data = i->first->data;
	IterElem* new_first = i->first->next;
	free(i->first);
	i->first = new_first;

	if (new_first == i->last) 	  i->penult = NULL;
	else if (new_first == NULL)	  i->last = NULL;

	return return_data;
}

void clearIter(Iterable* i)
{
	if (i->first == NULL)
		return;
	_clearIter(i->first);

	i->first 	= NULL;
	i->last 	= NULL;
	i->penult 	= NULL;
	i->len 		= 0;
}

void _clearIter(IterElem* ie)
{
	if (ie->next != NULL) {
		_clearIter(ie->next);
		free(ie->next);
	}
	free(ie->data);
}

data_t indexIter(Iterable* i, int32_t idx)
{
	if (idx < 0)	// Python-like negative index access
		idx = i->len + idx - 1;
	IterElem* elem = _indexIterElem(i, idx);
	return elem->data;
}

static IterElem* _indexIterElem(Iterable* i, uint32_t idx)
{
	if (i->first == NULL)
		return NULL;	// Nothing to index
	if (idx < 0 || idx >= i->len)
		EXIT_ERROR(ITER_ACCESS_OUTOFBOUNDS_ERR);
	return _indexElemRecursive(i->first, 0, idx);
}

static IterElem* _indexElemRecursive(IterElem* ie, uint32_t cur, uint32_t idx)
{
	if (cur == idx)
		return ie;
	else if (ie->next != NULL)
		return _indexElemRecursive(ie->next, ++cur, idx);
	else
		EXIT_ERROR(ITER_ACCESS_OUTOFBOUNDS_ERR);

	return NULL;
}

// Print data addresses
void printIter(Iterable* i)
{
	if (i->first == NULL)
		return;
	_printIter(i->first, 0);
}

static void _printIter(IterElem* ie, int32_t i)
{
	logf("%d: data at %p, next: %p\n", i, ie->data, ie->next);
	if (ie->next != NULL)
		_printIter(ie->next, ++i);
}
