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
	Iter* keyBindings;
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
	if (lenIter(layout->keyBindings) == 0)
		return NULL;	// Uninitialized keyLayout, maybe it should give a error

	startIter(layout->keyBindings);
	int remains;
	do {
		KeyBinding* bind = (KeyBinding*)nextIter(layout->keyBindings);
		if (bind->key == target_key)
			return bind;
		remains = remainsIter(layout->keyBindings);
	}
	while (remains);

	return NULL;		// No corresponding keys in given layout
}
