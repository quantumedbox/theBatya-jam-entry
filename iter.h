#pragma once

#include <stdlib.h>
#include <stdio.h>

// Generic data pointer iterator
// it doesn't work with values directly,
// all the casts to types should be explicit
typedef struct Iter
{
	void* data;
	struct Iter* next;
	_Bool is_iterated;
}
Iter;

Iter* newIter();
void addIter(Iter* iter, void* data);
void* popIter(Iter* iter);	// Stack-like pop from top
void* indexedIter(Iter* iter, int index);	// Indexed retrieval
void clearIter(Iter* iter);
int lenIter(Iter* iter);
void printIter(Iter* iter);

// Iteration functions
void startIter(Iter* iter);
void* nextIter(Iter* iter);
int remainsIter(Iter* iter);

static void _printIter(Iter* iter, int i);
static int _lenIter(Iter* iter, int len);
static int _remainsIter(Iter* iter, int remains);
static void* _nextIter(Iter* iter);
static void* _indexedIter(Iter* iter, int current_index, int target_index);

// First element of Iter is always NULL and points to the actual first element
Iter* newIter()
{
	Iter* new = (Iter*)malloc(sizeof(Iter));
	new->data = NULL;
	new->next = NULL;

	return new;
}

void addIter(Iter* iter, void* data)
{
	if (iter->next == NULL) {
		iter->next = newIter();
		iter->next->data = data;
	} else
		addIter(iter->next, data);
}

// Print data addresses
void printIter(Iter* iter)
{
	_printIter(iter->next, 0);
}

static void _printIter(Iter* iter, int i)
{
	logf("%d: data at %p, next: %p\n", i, iter->data, iter->next);
	if (iter->next != NULL)
		_printIter(iter->next, ++i);
}

// Pop the last element from Iter
// WARNING! in this case free() on data should be called manually
void* popIter(Iter* iter)
{
	if (iter->next != NULL) {
		if (iter->next->next == NULL) {
			void* ret_data = iter->next->data;
			free(iter->next);
			iter->next = NULL;
			return ret_data;
		}
		else
			return popIter(iter->next);
	}
	return NULL;	// Empty Iter
}

void clearIter(Iter* iter)
{
	if (iter->next != NULL) {
		clearIter(iter->next);
		free(iter->next);
		iter->next = NULL;
	}
	free(iter->data);
}

static int _lenIter(Iter* iter, int len)
{
	if (iter->next != NULL)
		return _lenIter(iter->next, len+1);
	return len;
}

int lenIter(Iter* iter)
{
	if (iter->next != NULL)
		return _lenIter(iter->next, 1);
	return 0;
}

void startIter(Iter* iter)
{
	iter->is_iterated = false;
	if (iter->next != NULL)
		startIter(iter->next);
}

int remainsIter(Iter* iter)
{
	if (iter->next != NULL)
		return _remainsIter(iter->next, 0);
	return 0;
}

static int _remainsIter(Iter* iter, int remains)
{
	if (!iter->is_iterated)
		++remains;
	if (iter->next != NULL)
		return _remainsIter(iter->next, remains);
	return remains;
}

void* nextIter(Iter* iter)
{
	if (iter->next != NULL)
		return _nextIter(iter->next);
	return NULL;
}

static void* _nextIter(Iter* iter)
{
	if (!iter->is_iterated) {
		iter->is_iterated = true;
		return iter->data;
	}
	if (iter->next != NULL)
		return _nextIter(iter->next);
	return NULL;
}

void* indexedIter(Iter* iter, int index)
{
	int i = index;
	if (index < 0)	// Python-like negative index access
		i = lenIter(iter) + index - 1;
	if (0 <= i && i < lenIter(iter) && lenIter(iter) != 0)
		return _indexedIter(iter->next, -1, i);
	else
		EXIT_ERROR(ITER_ACCESS_OUTOFBOUNDS_ERR);
	return NULL;	// It should not really be able to return NULL
}

static void* _indexedIter(Iter* iter, int current_index, int target_index)
{
	++current_index;
	if (current_index == target_index)
		return iter->data;
	else if (iter->next != NULL)
		return _indexedIter(iter->next, current_index, target_index);
	else
		EXIT_ERROR(ITER_ACCESS_OUTOFBOUNDS_ERR);
	return NULL;	// It should not really be able to return NULL
}
