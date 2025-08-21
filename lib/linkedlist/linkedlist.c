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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "linkedlist.h"

void linkedlist_append(l, item)
linkedlist_list *l;
void *item;
{
    linkedlist_node *new;
    
    new = malloc(sizeof(linkedlist_node));
    assert(new);
    new->value = item;
    new->next = NULL;
    if (l->head) {
        l->tail->next = new;
        new->prev = l->tail;
    } else {
        l->head = new;
        new->prev = NULL;
    }
    l->tail = new;
    l->nmemb++;
}

void linkedlist_appendlloc(l, item, size)
linkedlist_list *l;
void *item;
size_t size;
{
    linkedlist_node *new;
    
    new = malloc(sizeof(linkedlist_node));
    assert(new);
    new->value = malloc(size);
    assert(new->value);
    memcpy(new->value, item, size);
    new->next = NULL;
    if (l->head) {
        l->tail->next = new;
        new->prev = l->tail;
    } else {
        l->head = new;    
        new->prev = NULL;
    }
    l->tail = new;
    l->nmemb++;
}

void *linkedlist_find(l, mock, compar)
linkedlist_list *l;
void *mock;
int (*compar)(const void *, const void *);
{
    linkedlist_node *next;
    
    for (next=l->head;next;next=next->next)
        if (!compar(next->value, mock))
            return next->value;
    return NULL;
}

void *linkedlist_pop(l)
linkedlist_list *l;
{
    linkedlist_node *h;
    void *item;
    
    item = NULL;
    if ((h = l->head)) {
        item = h->value;
        l->head = h->next;
        if (h->next)
            l->head->prev = NULL;
        else
            l->tail = NULL;
        l->nmemb--;
        free(h);
    }
    return item;
}

linkedlist_node *linkedlist_delete(l, item)
linkedlist_list *l;
linkedlist_node *item;
{
    linkedlist_node *next;
    
    next = item->next;    
    if (item->prev)
        item->prev->next = next;
    else
        l->head = next;
    if (next)
        next->prev = item->prev;
    else
        l->tail = item->prev;
    l->nmemb--;
    free(item);
    return next;
}

void linkedlist_erase(l)
linkedlist_list *l;
{
    linkedlist_node *next, *curr;
    
    for (next = l->head; next;) {
        curr = next;
        next = next->next;
        free(curr);
    }
    l->nmemb = 0;
    l->head = l->tail = NULL;
}

void linkedlist_erasefree(l)
linkedlist_list *l;
{
    linkedlist_node *next, *curr;
    
    for (next = l->head; next;) {
        curr = next;
        next = next->next;
        free(curr->value);
        free(curr);
    }
    l->nmemb = 0;
    l->head = l->tail = NULL;
}

void linkedlist_move_all_last_first(ldest, lsrc)
linkedlist_list *ldest, *lsrc;
{
    while (lsrc->head)
        LINKEDLIST_MOVE_ONE(ldest, lsrc, lsrc->tail);
}

void linkedlist_move_all_first_last(ldest, lsrc)
linkedlist_list *ldest, *lsrc;
{
    while (lsrc->head)
        LINKEDLIST_MOVE_ONE(ldest, lsrc, lsrc->head);
}

linkedlist_to_array(arr, l, size)
unsigned char *arr;
linkedlist_list *l;
size_t size;
{
    linkedlist_node *next;
    int i;
    
    for (i=0, next=l->head;next;next=next->next, i++)
        memcpy(&arr[i*size], next->value, size);
    return i;
}
