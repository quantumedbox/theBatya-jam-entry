#pragma once

#include <stdlib.h>
#include <stdio.h>

#ifndef DEFAULT_MAX_ITER_LEN
#define DEFAULT_MAX_ITER_LEN -1	// Negative == unlimited
#endif

// TODO Блокирование изменений после начала итерации
// TODO Mulex lock
// TODO Забота о data alignment
// TODO !!! Аллокация всех элементов стека в памяти по порядку друг за другом и итерация через pointer offset, без нужды рекурсии
//			Индексация тогда станет максимально простой и независимой от размеров, next указатель может быть опущен, экономя память
//			Единственный минус это замедленное добавление новых элементов, так как вся структура требует перестройки (но возможна реализация capacity для буффера)
//			Но вся суть итератора - скорость итерации, она первостепенна

#define malloc_type(type) (type*)malloc(sizeof(type))

#define next_iteration_of_type(iter, type) (type*)nextIterator(iter)

#define ITERATION_STOP	NULL

#define check_stop_iteration(obj) if (obj == ITERATION_STOP) break;

#define ON_HEAP 	1
#define NOT_ON_HEAP 0
#define DEFAULT_ADD_ITER_FLAG ON_HEAP


// ----------------------------------------------------------------- Iterator usage -- //
/*

	Iterator* it = getIterator(i);
	while (true)
	{
		Type* obj = next_iteration_of_type(it, Type);
		check_stop_iteration(obj);
		...
	}

*/
// --------------------------------------------------------------- Type definitions -- //

typedef void* data_t;

// Stack chain of void* data
typedef struct IterElem
{
	struct IterElem* 	next;
	data_t 				data;	// there should be never be NULL
	_Bool				flag;	// may be ON_HEAP or NOT_ON_HEAP (reffering to data)
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

typedef data_t (*IterFunction_T)(IterElem* in);

typedef void (*IterMapFunction_T)(data_t in);

// Iteration is done via this
typedef struct
{
	IterElem* 			next;
//  IterElem*			last;	// for comparison in function-driven iteration
	int32_t 			remains;// TODO в данный момент это уже не нужно, ибо остановка происходит по возвращаемому значению. Следует почистить
	int32_t				idx;
	IterFunction_T 		itfunc;	// search function for custom iteration
	IterMapFunction_T 	mapfunc;// map function, that is called for every IterElem. if itfunc is present, mapfunc works only if itfunc check passed
}
Iterator;	// it

// ------------------------------------------------------------ Function signatures -- //

Iterable*	newIterLimited		(int32_t limit);
Iterable* 	newIter				();
   _Bool 	addIter				(Iterable* i,	data_t data, _Bool flag );
  data_t 	popIter				(Iterable* i);						// stack-like pop from the top
  data_t 	pullIter			(Iterable* i);						// stack-like pop from the beginning
  data_t 	indexIter 			(Iterable* i,  int32_t idx  );		// list-like indexed retrieval
	void 	delIndexIter		(Iterable* i,  int32_t idx  );
	void 	clearIter 			(Iterable* i);
	void 	printIter 			(Iterable* i);

// Iteration functions
Iterator* 	getIterator			(Iterable* i);						// get iterator from iterable
  data_t 	nextIterator		(Iterator* it);						// main iteration function for both functional and linear types
    void 	setIteratorFunc 	(Iterator* it, IterFunction_T f);
	void 	setIteratorMapFunc	(Iterator* it, IterMapFunction_T f);
    void* 	stopIterator		(Iterator* it);

#define     _S 	static 	// Internal realisation
_S  void 	_clearIter 			(IterElem* ie);
_S  void 	_printIter 			(IterElem* i, 	int32_t idx);
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

__forceinline Iterable* newIterLimited(int32_t limit)
{
	Iterable* new = newIter();
	new->limit = limit;

	return new;
}

IterElem* _newIterElem(data_t data, _Bool flag)
{
	IterElem* new = malloc_type(IterElem);
	new->next 		= NULL;
	new->data 		= data;
	new->flag 		= flag;

	return new;
}

Iterator* getIterator(Iterable* i)
{
	Iterator* new = malloc_type(Iterator);
	new->next 	 	= i->first;
//	new->last 		= i->last;
	new->remains 	= i->len;
	new->idx 		= 0;
	new->itfunc 	= NULL;
	new->mapfunc 	= NULL;

	return new;
}

__forceinline data_t nextIterator(Iterator* it)	// Важно обеспечить максимальную производительность данной функции
{
	if (it->next == NULL)
		return stopIterator(it);

	data_t return_data;

	if (it->itfunc == NULL)
	{
		if (it->remains == 0)
			return stopIterator(it);

		return_data = it->next->data;
		if (it->mapfunc != NULL)
			it->mapfunc(return_data);

		it->next = it->next->next;
		it->remains -= 1;
		it->idx 	+= 1;

		return return_data;
	}
	else
	{
		while (it->next != NULL)
		{
			return_data = it->itfunc(it->next);
			if (it->mapfunc != NULL)
				it->mapfunc(return_data);

			it->idx += 1;

			it->next = it->next->next;
			if(it->next == NULL) {
				return stopIterator(it);
			}
			if (return_data != NULL)
				return return_data;
		}
		return stopIterator(it);
	}
}

__forceinline void setIteratorFunc(Iterator* it, IterFunction_T func)
{
	it->itfunc = func;
}

__forceinline void setIteratorMapFunc(Iterator* it, IterMapFunction_T func)
{
	it->mapfunc = func;
}

__forceinline void* stopIterator(Iterator* it)
{
	free(it);	return ITERATION_STOP;
}

// Returns false if operation isn't successful
_Bool addIter(Iterable* i, data_t data, _Bool flag)
{
	if (i->limit > 0 && i->len + 1 > i->limit)
		return false;

	if (i->first == NULL) {
		i->first = _newIterElem(data, flag);
		i->last  = i->first;
	}
	else {
		i->last->next = _newIterElem(data, flag);
		if (i->len > 1) i->penult = i->last;
		i->last = i->last->next;
	}

	i->len += 1;
	return true;
}

__forceinline void delFirstElem(Iterable* i, _Bool erase)
{
	if (erase && i->first->flag == ON_HEAP)
		free(i->first->data);

	IterElem* to_del = i->first;

	i->first = i->first->next;

	if (i->first == NULL)	i->last = NULL;
	else if (i->len <= 2)	i->penult = NULL;

	free(to_del);
	i->len -= 1;
}

__forceinline void delLastElem(Iterable* i, _Bool erase)
{
	if (erase && i->last->flag == ON_HEAP)
		free(i->last->data);
	free(i->last);
	i->len -= 1;

	switch(i->len) {
	case 0:
		i->first = NULL;
		i->last = NULL;
		break;
	case 1:
		i->last = i->first;
		i->penult = NULL;
		break;
	case 2:
		i->last = i->penult;
		i->penult = NULL;
		break;
	default:
		i->last = i->penult;
		i->penult = _indexIterElem(i, i->len - 2);
	}
	if (i->last != NULL)
		i->last->next = NULL;
}

// Pop the last element from Iter
// WARNING! free() on data should be called manually
data_t popIter(Iterable* i)
{
	if (i->last == NULL)
		return NULL;	// Nothing to pop

	data_t return_data = i->last->data;
	delLastElem(i, false);
	return return_data;
}

// Pull the first element from Iter
// WARNING! free() on data should be called manually
data_t pullIter(Iterable* i)
{
	if (i->first == NULL)
		return NULL;

	data_t return_data = i->first->data;
	delFirstElem(i, false);
	return return_data;
}

// Python-like negative index access
__forceinline int32_t negateIndex(Iterable* i, int32_t idx)
{
	if (idx < 0)
		idx = i->len + idx - 1;
	return idx;
}

void delIndexIter(Iterable* i, int32_t idx)
{
	idx = negateIndex(i, idx);
	if 		(idx == 0) 		   delFirstElem(i, true);
	else if (idx == i->len - 1) delLastElem(i, true);
	else
	{
		IterElem* elem = _indexIterElem(i, idx - 1);
		IterElem* gap = elem->next->next;

		if (elem->next == i->penult)
		{
			if (i->len == 3)
				i->penult = NULL;
			else
				i->penult = elem;
		}

		if (elem->next->flag == ON_HEAP)
			free(elem->next->data);
		free(elem->next);
		i->len -= 1;

		elem->next = gap;
	}
}

data_t indexIter(Iterable* i, int32_t idx)
{
	idx = negateIndex(i, idx);
	IterElem* elem = _indexIterElem(i, idx);
	return elem->data;
}

static IterElem* _indexIterElem(Iterable* i, uint32_t idx)
{
	if (i->first == NULL)
		return NULL;	// Nothing to index
	if (idx < 0 || idx >= i->len) {
		char desc[128] = {'\0'};
		sprintf(desc, "index: %d", idx);
		EXIT_ERROR_DESC(ITER_ACCESS_OUTOFBOUNDS_ERR, desc);
	}
	return _indexElemRecursive(i->first, 0, idx);
}

static IterElem* _indexElemRecursive(IterElem* ie, uint32_t cur, uint32_t idx)
{
	if (cur == idx)
		return ie;
	else if (ie->next != NULL)
		return _indexElemRecursive(ie->next, ++cur, idx);
	else {
		char desc[128] = {'\0'};
		sprintf(desc, "index: %d", idx);
		EXIT_ERROR_DESC(ITER_ACCESS_OUTOFBOUNDS_ERR, desc);
	}

	return NULL;
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
	if (ie->flag == ON_HEAP)
		free(ie->data);
}

// Print data addresses
void printIter(Iterable* i)
{
	printf("Iteable at %p\n", i);
	printf("len: %d, limit: %d\n", i->len, i->limit);
	printf("first: %p, last: %p, penult: %p\n", i->first, i->last, i->penult);
	printf("---------------------------\n");
	if (i->first == NULL)
		return;
	_printIter(i->first, 0);
}

static void _printIter(IterElem* ie, int32_t i)
{
	logf("(%d)%p |h:%d| data at %p, next: %p\n", i, ie, ie->flag, ie->data, ie->next);
	if (ie->next != NULL)
		_printIter(ie->next, ++i);
}
