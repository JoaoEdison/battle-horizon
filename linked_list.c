#include <stdlib.h>
#include <string.h>

struct node {
	void *data, *next, *prev;
};

struct list {
	struct node *first, *last;
	int size;
	int (*cmp)();
};

void list_insert(item, l, size)
void *item;
struct list *l;
size_t size;
{
	struct node *new;

	new = malloc(sizeof(struct node));
	new->data = malloc(size);
	if (!l->first)
		l->last = l->first = new;
	else {
		l->last->next = new;
		l->last = new;
	}
	memcpy(new->data, item, size);
	new->next = NULL;
	l->size++;
}

void list_remove(item, l)
struct node *item;
struct list *l;
{
	struct node *prev;
	
	if (item == l->first && item == l->last) {
		l->first = l->last = NULL;
		l->size--;
		free(item->data);
		free(item);
		return;
	}
	if (item == l->first) {
		l->first = item->next;
		l->size--;
		free(item->data);
		free(item);
		return;
	}
	for (prev = l->first; prev; prev = prev->next)
		if (prev->next == item)
			break;
	prev->next = item->next;
	if (item == l->last)
		l->last = prev;
	l->size--;
	free(item->data);
	free(item);
}

in_list(item, l)
void *item;
struct list *l;
{
	struct node *next;

	for (next = l->first; next; next = next->next)
		if (l->cmp(next->data, item))
			return 1;
	return 0;
}
