#pragma once

#include <stdio.h>
#include <stdlib.h>

// Naive hashmap implementation

#define MAP_STANDART_BUCKET_COUNT 32

// -------------------------------------------------------------- Type degfinitions -- //

typedef uint64_t key_t;

typedef void* data_t;

// Returns a key depending on the data
typedef key_t(*HashFunc_T)(data_t in);

// Stack-like structure that holds a key and void pointer to data
typedef struct Bucket
{
	key_t			key;

	void* 			data;

	struct Bucket* 	next;
}
Bucket;

typedef struct
{
	Bucket* 		buckets;

	size_t 			bucket_count;

	size_t 			len;
}
Map;

// ------------------------------------------------------------ Function signatures -- //

Map* 		mapNew			();
void 		mapAdd 			(Map*, key_t key,  data_t data);
void 		mapAddByFunc	(Map*, HashFunc_T, data_t data);
void 		mapPrint 		(Map*);
_Bool		mapHasKey		(Map*, key_t key);
_Bool 		mapHasKeyByFunc	(Map*, HashFunc_T, data_t data);

static Bucket* 	_mapNewBucket	();
static _Bool 	_mapHasKeyRecur	(Bucket*, key_t key);

static void _mapAllocateBuckets	(Map* m, size_t n);
static void _mapAddRecur		(Bucket* b, key_t key, data_t data);
static void _mapPrintRecur		(Bucket* b);

// ---------------------------------------------------------------------- Functions -- //

Map* mapNew()
{
	Map* new = (Map*)malloc(sizeof(Map));

	_mapAllocateBuckets(new, MAP_STANDART_BUCKET_COUNT);

	new->len = 0;

	return new;
}

static inline Bucket* _mapNewBucket()
{
	Bucket* new = (Bucket*)calloc(1, sizeof(Bucket));

	return new;
}

static inline void _mapAllocateBuckets(Map* m, size_t n)
{
	m->buckets = (Bucket*)calloc(n, sizeof(Bucket));
	m->bucket_count = n;
}

void mapAdd(Map* m, key_t key, data_t data)
{
	uint32_t idx = key % m->bucket_count;

	m->len += 1;

	if (m->buckets[idx].next == NULL)
	{
		Bucket* new = _mapNewBucket();
		m->buckets[idx].next = new;
		new->key 	= key;
		new->data 	= data;
	}
	else 
		_mapAddRecur(m->buckets[idx].next, key, data);
}

void mapAddByFunc(Map* m, HashFunc_T hashfunc, data_t data)
{
	mapAdd(m, hashfunc(data), data);
}

static inline void _mapAddRecur(Bucket* b, key_t key, data_t data)
{
	if (b->key == key) {
		b->data = data;
	}

	else if (b->next == NULL)
	{
		Bucket* new = _mapNewBucket();
		b->next 	= new;
		new->key 	= key;
		new->data 	= data;
	}

	else {
		_mapAddRecur(b->next, key, data);
	}
}

_Bool mapHasKey(Map* m, key_t key)
{
	uint32_t idx = key % m->bucket_count;

	if (m->buckets[idx].next != NULL)
		return _mapHasKeyRecur(m->buckets[idx].next, key);
	else
		return false;
}

_Bool mapHasKeyByFunc(Map* m, HashFunc_T hashfunc, data_t data)
{
	return mapHasKey(m, hashfunc(data));
}

static inline _Bool _mapHasKeyRecur(Bucket* b, key_t key)
{
	if (b->key == key)
		return true;

	else if (b->next == NULL)
		return false;

	return _mapHasKeyRecur(b->next, key);
}

void mapPrint(Map* m)
{
	printf("hashmap object at %p\n", m);
	printf("len: %llu, buckets count: %llu\n", m->len, m->bucket_count);
	printf("-----------------------\n");

	for (int i = m->bucket_count; i--;)
	{
		if (m->buckets[i].next != NULL) {
			printf("---BUCKET %d\n", i);
			_mapPrintRecur(m->buckets[i].next);
		}
	}
}

static void _mapPrintRecur(Bucket* b)
{
	printf("(%p) key: %llu, data at: %p, next: %p\n", b, b->key, b->data, b->next);

	if (b->next != NULL)
		_mapPrintRecur(b->next);
}
