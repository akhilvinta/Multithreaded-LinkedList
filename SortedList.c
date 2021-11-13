//Akhil Vinta
//405288527
//akhil.vinta@gmail.com

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h> 
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "SortedList.h"

int opt_yield = 0;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    SortedListElement_t *temp = list->next; 
    while(temp != list){
        if(strcmp(temp->key, element->key) >= 0)
           break;
        if (opt_yield & INSERT_YIELD) {
          	sched_yield();
        }
        temp=temp->next;
    }
    element->prev=temp->prev;
    element->next=temp;
    temp->prev->next=element;
    temp->prev=element;
}

int SortedList_delete(SortedListElement_t *element){
    if(element->next->prev != element || element->prev->next != element || element == NULL)
        return 1;

    if (opt_yield & DELETE_YIELD){
        sched_yield();
    }

    element->next->prev = element->prev;
    element->prev->next = element->next;
    return 0;
}



SortedListElement_t* SortedList_lookup(SortedList_t *list, const char *key){
    SortedListElement_t *temp = list->next;
    while(temp != list){
        if(strcmp(key, temp->key) == 0)
            return temp;
        if (opt_yield & LOOKUP_YIELD){
          		sched_yield();
        }
        temp=temp->next;
    }
    return NULL;
}


int SortedList_length(SortedList_t *list){
    int length = 0;
    SortedListElement_t *temp = list->next;
    while(temp != list){
        length++;
        temp=temp->next;
    }
    return length;
}



