#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

// Naive hashmap implementation

#define MAP_INIT_CAPACITY 	16		// always should be power of 2

#define MAP_INIT_THRESHOLD 	0.75

#define ON_HEAP 	0
#define NOT_ON_HEAP 1
#define SHARED_HEAP 2
#define NO_MEM_FLAG 3

// -------------------------------------------------------------- Type degfinitions -- //

typedef uint64_t key_t;

typedef void* data_t;

typedef uint8_t flag_t;

// Returns a key depending on the data
typedef key_t(*HashFunc_T)(data_t in);

// If set - on bucket deletion such func should be called upon data (if flag ON_HEAP is set)
typedef void(*DelFunc_T)(void* in);

// Stack-like structure that holds a key and void pointer to data
typedef struct Bucket
{
	key_t			key;

	void* 			data;

	uint8_t			flag;

	DelFunc_T		delfunc;

	struct Bucket* 	next;
}
Bucket;

typedef struct
{
	Bucket* 		buckets;

	// num of allocated buckets in memory
	size_t 			capacity;

	// percentage threshold that triggers resizing if len / capacity > threshold // should be normalized in (0, 1]
	float 			threshold;

	// item count
	size_t 			len;

	// mutex lock for accessing a map from multiple threads
	pthread_mutex_t lock;
}
Map;

// ------------------------------------------------------------ Function signatures -- //

// for every hashing object on heap should be declared void* del function with the VOID_ prefix
#define mapAddByFuncHeap(m, hashfunc, data, delfunc) \
	   _mapAddByFuncHeap(m, hashfunc, data, VOID_##delfunc)

		Map* 	mapNew				();
		void 	mapAdd 				(Map*, key_t key,  data_t, uint8_t flag);
		void 	mapAddByFunc		(Map*, HashFunc_T, data_t, uint8_t flag);
	   	bool	mapHasKey			(Map*, key_t key);
	   	bool 	mapHasKeyByFunc		(Map*, HashFunc_T, data_t);
	   	void 	mapDelKey			(Map*, key_t key);
	   	void 	mapDelKeyByFunc		(Map*, HashFunc_T, data_t);
		void 	mapSetDelFunc		(Map*, key_t key,  DelFunc_T);
		void 	mapSetDelFuncByFunc	(Map*, HashFunc_T, data_t, DelFunc_T);
	   	void 	mapClear			(Map*);
		void 	mapPrint 			(Map*);

 		void 	_mapAddByFuncHeap	(Map*, HashFunc_T, data_t, DelFunc_T);

static 	Bucket* _mapNewBucket		();
static  void 	_mapExtend			(Map*);
static  Bucket* _mapGetBucket		(Map*,    key_t key);
static  Bucket* _mapGetBucketRecur	(Bucket*, key_t key);
static 	bool 	_mapHasKeyRecur		(Bucket*, key_t key);
static 	void 	_mapAllocateBuckets	(Map*, 	 size_t n);
static 	bool 	_mapAddRecur		(Bucket*, key_t key, data_t, uint8_t flag);
static 	void 	_mapPrintRecur		(Bucket*);
static  void 	_mapExtend			(Map*);
static  void 	_mapClearStack		(Bucket*);

static  void 	_mapReallocateBucketStack	(Bucket* buckets, Bucket* stack, size_t cap);
static  void 	_mapReallocateBucketRecur	(Bucket* stack,   Bucket* bucket);

// ----------------------------------------------------------------- Hash functions -- //

// TODO Generic macro for data types

// Hash function that uses address of data to generate key
// It should be used for allocated structs that are known for caller
key_t hashAddress(data_t in)
{
	key_t key  = (key_t)in;

	key  = (~key) + (key << 21);
	key ^= key >> 24;
	key  = (key + (key << 3)) + (key << 8);
	key ^= key >> 14;
	key  = (key + (key << 2)) + (key << 4);
	key ^= key >> 28;
	key += key << 31;

	return key;
}

key_t hash64int(data_t in)
{
	key_t key  = *(int64_t*)in;

	key  = (~key) + (key << 21);
	key ^= key >> 24;
	key  = (key + (key << 3)) + (key << 8);
	key ^= key >> 14;
	key  = (key + (key << 2)) + (key << 4);
	key ^= key >> 28;
	key += key << 31;

	return key;
}

// ---------------------------------------------------------------------- Functions -- //


Map* mapNew()
{
	Map* new = (Map*)malloc(sizeof(Map));

	_mapAllocateBuckets(new, MAP_INIT_CAPACITY);

	pthread_mutex_init(&new->lock, NULL);

	new->threshold 	= MAP_INIT_THRESHOLD;
	new->len 		= 0;

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
	m->capacity = n;
}


void mapAdd(Map* m, key_t key, data_t data, uint8_t flag)
{
	pthread_mutex_lock(&m->lock);

	uint32_t idx = key % m->capacity;

	if (m->buckets[idx].next == NULL)
	{
		m->len += 1;

		Bucket* new = _mapNewBucket();
		m->buckets[idx].next = new;
		new->key 	= key;
		new->data 	= data;
		new->flag 	= flag;
	}
	else if (_mapAddRecur(m->buckets[idx].next, key, data, flag))
		m->len += 1;

	if (((float)m->len / m->capacity) >= m->threshold)
		_mapExtend(m);

	pthread_mutex_unlock(&m->lock);
}


__forceinline void mapAddByFunc(Map* m, HashFunc_T hashfunc, data_t data, uint8_t flag)
{
	mapAdd(m, hashfunc(data), data, flag);
}


__forceinline void _mapAddByFuncHeap(Map* m, HashFunc_T hashfunc, data_t data, DelFunc_T delfunc)
{
	key_t hash = hashfunc(data);

	mapAdd(m, hash, data, ON_HEAP);
	mapSetDelFunc(m, hash, delfunc);
}


// Returns true if new bucket was added to the stack
static inline bool _mapAddRecur(Bucket* b, key_t key, data_t data, uint8_t flag)
{
	if (b->key == key) {
		b->data = data;

		return false;
	}

	else if (b->next == NULL)
	{
		Bucket* new = _mapNewBucket();
		b->next 	= new;
		new->key 	= key;
		new->data 	= data;
		new->flag 	= flag;

		return true;
	}

	return _mapAddRecur(b->next, key, data, flag);
}


void mapDelKey(Map* m, key_t key)
{
	// TODO
}


__forceinline void mapDelKeyByFunc(Map* m, HashFunc_T hashfunc, data_t data)
{
	mapDelKey(m, hashfunc(data));
}


bool mapHasKey(Map* m, key_t key)
{
	bool return_data = false;

	pthread_mutex_lock(&m->lock);

	uint32_t idx = key % m->capacity;

	if (m->buckets[idx].next != NULL)
		return_data = _mapHasKeyRecur(m->buckets[idx].next, key);

	pthread_mutex_unlock(&m->lock);

	return return_data;
}


__forceinline bool mapHasKeyByFunc(Map* m, HashFunc_T hashfunc, data_t data)
{
	return mapHasKey(m, hashfunc(data));
}

static inline bool _mapHasKeyRecur(Bucket* b, key_t key)
{
	if (b->key == key)
		return true;

	else if (b->next == NULL)
		return false;

	return _mapHasKeyRecur(b->next, key);
}


// Create new bucket array and reallocate existing elems in it
static void _mapExtend(Map* m)
{
	size_t old_capacity = m->capacity;

	m->capacity = pow(2, log2(m->capacity)+1);

	Bucket* new_array = (Bucket*)calloc(m->capacity, sizeof(Bucket));

	#pragma omp parallel for
	for (register int i = 0; i < old_capacity; i++)
	{
		if (m->buckets[i].next != NULL)
			_mapReallocateBucketStack(new_array, m->buckets[i].next, m->capacity);
	}

	free(m->buckets);
	m->buckets = new_array;
}


// Recursevly distribute all elements of bucket stack to the buckets array
static void _mapReallocateBucketStack(Bucket* buckets, Bucket* stack, size_t cap)
{
	uint32_t idx = stack->key % cap;

	Bucket* stack_next = stack->next;

	if (buckets[idx].next == NULL) {
		buckets[idx].next = stack;
		stack->next = NULL;
	}
	else
		_mapReallocateBucketRecur(buckets[idx].next, stack);

	if (stack_next != NULL)
		_mapReallocateBucketStack(buckets, stack_next, cap);
}


// Attach bucket argument to the last element of bucket stack
static void _mapReallocateBucketRecur(Bucket* stack, Bucket* bucket)
{
	if (stack->next == NULL) {
		stack->next = bucket;
		bucket->next = NULL;
	}
	else
		_mapReallocateBucketRecur(stack->next, bucket);
}


void mapDel(Map* m)
{
	// TODO Resolve the issue of deleting shared map

	mapClear(m);
	free(m->buckets);
	free(m);
}


void mapClear(Map* m)
{
	pthread_mutex_lock(&m->lock);

	#pragma omp parallel for
	for (register int i = 0; i < m->capacity; i++)
	{
		if (m->buckets[i].next != NULL) {
			_mapClearStack(m->buckets[i].next);
			m->buckets[i].next = NULL;
		}
	}

	m->len = 0;

	if (m->capacity > MAP_INIT_CAPACITY)
	{
		free(m->buckets);
		_mapAllocateBuckets(m, MAP_INIT_CAPACITY);
	}

	pthread_mutex_unlock(&m->lock);
}


static void _mapClearStack(Bucket* stack)
{
	if (stack->flag == ON_HEAP) {
		if (stack->delfunc)
			stack->delfunc(stack->data);
		else
			free(stack->data);
	}

	if (stack->next != NULL)
		_mapClearStack(stack->next);

	free(stack);
}


void mapSetDelFunc(Map* m, key_t key, DelFunc_T delfunc)
{
	pthread_mutex_lock(&m->lock);

	Bucket* target = _mapGetBucket(m, key);

	target->delfunc = delfunc;

	pthread_mutex_unlock(&m->lock);
}


__forceinline void mapSetDelFuncByFunc(Map* m, HashFunc_T hashfunc, data_t data, DelFunc_T delfunc)
{
	mapSetDelFunc(m, hashfunc(data), delfunc);
}


static Bucket* _mapGetBucket(Map* m, key_t key)
{
	uint32_t idx = key % m->capacity;

	return _mapGetBucketRecur(m->buckets[idx].next, key);
}


static Bucket* _mapGetBucketRecur(Bucket* stack, key_t key)
{
	if (stack->key == key)
		return stack;

	else if (stack->next != NULL)
		return _mapGetBucketRecur(stack->next, key);

	else EXIT_ERROR(MAP_NO_KEY_ERR);

	return NULL;
}


void mapPrint(Map* m)
{
	fprintf(stdout, "hashmap object at %p\n", m);
	fprintf(stdout, "len: %llu, threshold: %.2f, buckets count: %llu\n", m->len, m->threshold, m->capacity);
	fprintf(stdout, "-----------------------\n");

	for (int i = m->capacity; i--;)
	{
		if (m->buckets[i].next != NULL) {
			fprintf(stdout, "---BUCKET %d\n", i);
			_mapPrintRecur(m->buckets[i].next);
		}
	}
}


static char* _mapFlagDesriprions[] = {
	"HEAP",
	"NOT HEAP",
	"SHARED HEAP"
};


static void _mapPrintRecur(Bucket* b)
{
	fprintf(
		stdout, "(%p) |%s%s| key: %llu, data at: %p, next: %p\n",
		b,
		_mapFlagDesriprions[b->flag],
		b->delfunc ? "+DF" : "",
		b->key,
		b->data,
		b->next
	);

	if (b->next != NULL)
		_mapPrintRecur(b->next);
}
