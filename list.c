#include "list.h"

void build(struct List *list) {
    list->head = (struct Node *) malloc(sizeof(struct Node));
    list->tail = list->head;
    list->head->next = NULL;
}

void add(char *data, struct List *list) {
    list->tail->next = (struct Node *) malloc(sizeof(struct Node));
    list->tail = list->tail->next;
    list->tail->next = NULL;
    list->tail->data = data;
}

void removeNode(char *data, struct List *list) {
    struct Node * temp;
    struct Node * anterior;
    if(list->head == list->tail) {
        return;
    }
    temp = list->head->next;
    anterior = list->head;
    while(temp->data != data) {
        anterior = temp;
        temp = temp->next;
    }
    anterior->next = temp->next;
    if(anterior->next == NULL)
        list->tail = anterior;
    free(temp);
}

void display(struct List list) {
    if(list.head == list.tail){
        printf("none\n");
        return;
    }

    struct Node * aux;
    aux = list.head->next;
    while(aux != NULL) {
        printf("%s", aux->data);
        aux = aux->next;
        if (aux != NULL) {
            printf(" ");
        }
    }
    printf("\n");
    free(aux);
}