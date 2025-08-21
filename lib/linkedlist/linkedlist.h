/*
    Generic doubly linked list implementation in C.
    Copyright (C) 2025  João Manica  <joaoedisonmanica@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>

typedef struct node {
    void *value;
    struct node *next, *prev;
} linkedlist_node;

typedef struct {
    linkedlist_node *head, *tail;
    int nmemb;
} linkedlist_list;

void linkedlist_append(linkedlist_list *l, void *item);
void linkedlist_appendlloc(linkedlist_list *l, void *item, size_t size);
void *linkedlist_find(linkedlist_list *l, void *mock, int (*compar)(const void *, const void *));
void *linkedlist_pop(linkedlist_list *l);

/* Do not frees memory in item->value. */
linkedlist_node *linkedlist_delete(linkedlist_list *l, linkedlist_node *item);
/* Do free and delete the node. */
#define LINKEDLIST_DELETEFREE(L, ITEM) free((ITEM)->value), linkedlist_delete(L, ITEM)

/* Do not frees memory in values and list argument. */
void linkedlist_erase(linkedlist_list *l);
/* Frees memory in values. But not the list argument. */
void linkedlist_erasefree(linkedlist_list *l);


#define LINKEDLIST_MOVE_ONE(LDEST, LSRC, ITEM) \
    do {\
        linkedlist_append(LDEST, (ITEM)->value);\
        linkedlist_delete(LSRC, ITEM);\
    } while (0)

/* Move all itens from lsrc to ldest, from last to first. */
void linkedlist_move_all_last_first(linkedlist_list *ldest, linkedlist_list *lsrc);
/* Move all itens from lsrc to ldest in order. */
void linkedlist_move_all_first_last(linkedlist_list *ldest, linkedlist_list *lsrc);

/* CAUTION: Assumes that arr has sufficient capacity. */
int linkedlist_to_array(unsigned char *arr, linkedlist_list *l, size_t size);

#endif
