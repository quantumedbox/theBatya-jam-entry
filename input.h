#pragma once

#define MAX_N_KEYS 128

#include "iter.h"

enum KEY_TYPES {
	UNKNOWN_KEY,
	MOVEMENT_KEY,
};

typedef struct {
	int key;
	int action;
	int type;
}
KeyBinding;

typedef struct
{
	Iterable* keyBindings;
}
KeyLayout;

void keyLayout_bindNewKey(KeyLayout* layout, int key, int action, int type)
{
	KeyBinding* bind = (KeyBinding*)malloc(sizeof(KeyBinding));
	bind->key = key;
	bind->action = action;
	bind->type = type;
	addIter(layout->keyBindings, (void*)bind);
}

KeyLayout* keyLayout_new(void)
{
	KeyLayout* new = (KeyLayout*)malloc(sizeof(KeyLayout));
	new->keyBindings = newIter();
	return new;
}

KeyBinding* keyLayout_processKey(KeyLayout* layout, int target_key)
{
	if (layout->keyBindings->len == 0)
		return NULL;	// Uninitialized keyLayout, maybe it should give an error instead

	Iterator* iter = getIterator(layout->keyBindings);
	while (iter->remains)
	{
		KeyBinding* bind = next_iterator_of_type(iter, KeyBinding);
		if (bind->key == target_key)
			return bind;
	}
	return NULL;		// No corresponding keys in given layout
}
