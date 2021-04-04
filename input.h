#pragma once

#define MAX_N_KEYS 128

#include "iter.h"

enum KEY_TYPES {
	UNKNOWN_KEY,
	MOVEMENT_KEY,
	CONTROL_KEY,
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
	iterAdd(layout->keyBindings, (void*)bind, ON_HEAP);
}

KeyLayout* keyLayout_new(void)
{
	KeyLayout* new = (KeyLayout*)malloc(sizeof(KeyLayout));
	new->keyBindings = iterNew();
	return new;
}

KeyBinding* keyLayout_processKey(KeyLayout* layout, int target_key)
{
	if (layout->keyBindings->len == 0)
		return NULL;	// Uninitialized keyLayout, maybe it should give an error instead

	Iterator* iter = getIterator(layout->keyBindings);
	while_iter(iter)
	{
		KeyBinding* bind = next_iteration_of_type(iter, KeyBinding);
		check_stop_iteration(iter, bind);

		if (bind->key == target_key)
			return bind;
	}
	return NULL;		// No corresponding keys in given layout
}
