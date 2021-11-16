#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct Node {
    char *data;
    struct Node *next;
};

struct List{
    struct Node *head, *tail;
};

void build(struct List *list);
void add(char *data, struct List *list);
void removeNode(char *data, struct List *list);
void display(struct List list);

#endif