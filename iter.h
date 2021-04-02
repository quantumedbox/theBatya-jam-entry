#pragma once

#include <stdlib.h>
#include <stdio.h>

#ifndef DEFAULT_MAX_ITER_LEN
#define DEFAULT_MAX_ITER_LEN -1	// Negative == unlimited
#endif

#define ITERATION_STOP		NULL

#define SECURE_ITER_LIMIT 	0xFFFF // 65535

// TODO Блокирование изменений после начала итерации
// TODO Mulex lock
// TODO Забота о data alignment
// TODO Предупреждение о перезаполнении
// TODO Функция сортировки для подготовки к итерациям зависимых от друг друга (например, рендер и байндинг шейдеров)

#define malloc_type(type) (type*)malloc(sizeof(type))

#define malloc_zero(type) (type*)malloc(0)

#define realloc_type(ptr, type, size) (type*)realloc(ptr, sizeof(type) * size)

#define next_iteration_of_type(iter, type) (type*)nextIterator(iter)

#define check_stop_iteration(obj) if (obj == ITERATION_STOP) break;

// makes sure that such loop will not be infinite
#ifdef SECURE_ITERATION
# 	define while_iter(it) int i = SECURE_ITER_LIMIT; \
						  while(i-- > 0)
#else
# 	define while_iter(it) while(true)
#endif

#define ON_HEAP 	0	// when iterable is cleared it's used to determine that allocated data should be freed
#define NOT_ON_HEAP 1	// for application non-heap memory
#define SHARED_HEAP 2	// data should not be freed but it is in heap
#define NO_MEM_FLAG 3	// sets after clearing

// ----------------------------------------------------------------- Iterator usage -- //
/*

	Iterator* it = getIterator(i);
	while_iter(it)
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
	data_t 				data;	// there should be never be NULL
	uint8_t				flag;	// may be ON_HEAP or NOT_ON_HEAP (reffering to data)
}
IterElem;	// ie

// Head of the iterator stack
// All functions are called upon it
typedef struct
{
	IterElem* 			elems;	// pointer to in-memory IterElem array of Iterable.cap length
	int32_t 			len;	// populated slots
	int32_t 			cap;	// allocated slots
	int32_t 			limit;	// if necessary, you can set a limit on the max amount of elements
								// negative value means that there's no limit (which could be dangerous)
}
Iterable;	// i

// NULL as return means that the item did not pass a check
typedef data_t (*IterFunction_T)(IterElem* in);

// Modifies the iterated data
typedef void (*IterMapFunction_T)(data_t in);

// Iteration is done via this
typedef struct
{
	IterElem* 			current;// fast access without pointer deref
	uint32_t			idx;
	uint32_t			remains;
	IterFunction_T 		itfunc;	// search function for custom iteration
	IterMapFunction_T 	mapfunc;// map function, that is called for every IterElem. if itfunc is present, mapfunc works only if itfunc check passed
}
Iterator;	// it

// ------------------------------------------------------------ Function signatures -- //

// TODO Maybe we should rethink the naming conventions of this module ?

Iterable*	newIterLimited		(int32_t limit);
Iterable*	newIterCaped		(int32_t cap);
Iterable* 	newIter				();
	void 	delIter 			(Iterable* i);
 	void 	setCapIter			(Iterable* i, uint32_t cap);
   _Bool 	addIter				(Iterable* i,	data_t data, _Bool flag);
  data_t 	popIter				(Iterable* i);						// stack-like pop from the top
  data_t 	pullIter			(Iterable* i);						// stack-like pop from the beginning
  data_t 	indexIter 			(Iterable* i,  int32_t nidx);		// list-like indexed retrieval
	void 	delIndexIter		(Iterable* i,  int32_t nidx);
	void 	clearIter 			(Iterable* i);
	void 	printIter 			(Iterable* i);

// Iteration functions
Iterator* 	getIterator			(Iterable* i);						// get iterator from iterable
  data_t 	nextIterator		(Iterator* it);						// main iteration function for both functional and linear types
    void 	setIteratorFunc 	(Iterator* it, IterFunction_T f);
	void 	setIteratorMapFunc	(Iterator* it, IterMapFunction_T f);
    void* 	stopIterator		(Iterator* it);

// Internal realisation
static void _delElem			(Iterable* i, uint32_t idx, _Bool to_free);

// ---------------------------------------------------------------------- Functions -- //

Iterable* newIter()	// New iterable
{
	Iterable* new = malloc_type(Iterable);
	new->elems 		= malloc_zero(IterElem);
	new->len 		= 0;
	new->cap 		= 0;
	new->limit		= DEFAULT_MAX_ITER_LEN;

	return new;
}

__forceinline Iterable* newIterLimited(int32_t limit)
{
	Iterable* new = newIter();
	new->limit = limit;

	return new;
}

__forceinline Iterable* newIterCaped(int32_t cap)
{
	Iterable* new = newIter();
	setCapIter(new, cap);

	return new;
}

Iterator* getIterator(Iterable* i)
{
	Iterator* new 	= malloc_type(Iterator);
	new->current 	= i->elems;
	new->remains	= i->len;
	new->idx 		= -1;
	new->itfunc 	= NULL;
	new->mapfunc 	= NULL;

	return new;
}

__forceinline data_t nextIterator(Iterator* it)	// Важно обеспечить максимальную производительность данной функции
{
	if (it->remains <= 0)
		return stopIterator(it);

	#define stepIterator()	it->idx 	+= 1; \
							it->remains -= 1; \
							it->current += 1
	data_t return_data;

	if (it->itfunc)
	{
		while (it->remains > 0)
		{
			return_data	= it->itfunc(it->current);
			stepIterator();

			if (return_data)
			{
				if (it->mapfunc)
					it->mapfunc(return_data);

				return return_data;
			}
		}
		return stopIterator(it);
	}
	else
	{
		return_data = it->current->data;
		stepIterator();

		if (it->mapfunc)
			it->mapfunc(return_data);

		return return_data;
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

// if current len is greater than new cap -> all items at the end that didn't fit will be discarded and new len will be equal to cap
__forceinline void setCapIter(Iterable* i, uint32_t cap)
{
	i->elems 		= realloc_type(i->elems, IterElem, cap);
	if (i->len > cap)
		i->len 		= cap;
	i->cap 			= cap;
}

void delIter(Iterable* i)
{
	clearIter(i);
	free(i->elems);
	free(i);
}

// Returns false if operation wasn't successful
_Bool addIter(Iterable* i, data_t data, _Bool flag)
{
	if (i->limit > 0 && i->len + 1 > i->limit)
		return false;

	if (i->cap < i->len + 1)
		setCapIter(i, i->cap+1);

	IterElem* elem = &(i->elems[i->len]);
	elem->data = data;
	elem->flag = flag;

	i->len += 1;
	return true;
}

// Pop the last element from Iter
// WARNING! free() on data should be called manually
data_t popIter(Iterable* i)
{
	if (i->len == 0)
		return NULL;

	data_t return_data = i->elems[i->len-1].data;
	_delElem(i, i->len-1, false);
	return return_data;
}

// Pull the first element from Iter
// WARNING! free() on data should be called manually
data_t pullIter(Iterable* i)
{
	if (i->len == 0)
		return NULL;

	data_t return_data = i->elems[0].data;
	_delElem(i, 0, false);
	return return_data;
}

// Shift all the elems back in the array, discarding the element at idx (Usually, when deleting not from the very end)
static inline void _shiftElems(Iterable* i, uint32_t idx)
{
	IterElem* prev_elem = &(i->elems[idx]);
	IterElem* cur_elem  = &(i->elems[idx+1]);
	for (register uint32_t remains = i->len - 1; remains--;)
	{
		prev_elem->data = cur_elem->data;
		prev_elem->flag = cur_elem->flag;

		++prev_elem; 	++cur_elem;
	}
}

static inline void _delElem(Iterable* i, uint32_t idx, _Bool to_free)
{
	IterElem* elem = &(i->elems[idx]);
	if (to_free && elem->flag == ON_HEAP) {
		free(elem->data);
	}
	if (idx != i->len-1) {
		_shiftElems(i, idx);
	}

	i->len -= 1;
}

// Python-like negative index access
static inline int32_t _negateIndex(Iterable* i, int32_t nidx)
{
	if (nidx < 0)
		nidx = i->len + nidx - 1;
	return nidx;
}

void delIndexIter(Iterable* i, int32_t nidx)
{
	_delElem(i, _negateIndex(i, nidx), true);
}

data_t indexIter(Iterable* i, int32_t nidx)
{
	return i->elems[_negateIndex(i, nidx)].data;
}

void clearIter(Iterable* i)
{
	IterElem* cur_elem = &(i->elems[i->len-1]);
	for (register uint32_t remains = i->len; remains--;)
	{
		if (cur_elem->flag == ON_HEAP)
			free(cur_elem->data);
		cur_elem->data = NULL;
		cur_elem->flag = NO_MEM_FLAG;

		--cur_elem;
	}

	i->len = 0;
}

static char* _iterFlagDesriprions[] = {
	"HEAP",
	"NOT HEAP",
	"SHARED HEAP",
	"NOT SET"
};

// Print data addresses
void printIter(Iterable* i)
{
	printf("Iteable at %p\n", i);
	printf("len: %d, cap: %d, limit: %d\n", i->len, i->cap, i->limit);
	printf("---------------------------\n");
	if (i->len > 0)
	{
		IterElem* cur_elem = i->elems;
		for (register uint32_t remains = i->len; remains--;)
		{
			logf("(%d)%p |%s| data at %p\n", i->len - remains, cur_elem, _iterFlagDesriprions[cur_elem->flag], cur_elem->data);

			++cur_elem;
		}
	}
}
