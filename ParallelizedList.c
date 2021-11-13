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


SortedList_t *head;
SortedListElement_t *temp;
int spin_lock, x, num_threads, num_iterations;
bool m_flag=false, s_flag=false;
char sync_option;
pthread_mutex_t mutex; 

void check(int error, char *message, int exit_status){
    if(error < 0){
        fprintf(stderr,"%s Exiting with status %d.\n", message, exit_status);
        exit(exit_status);
    }
}
static inline unsigned long get_nanosec_from_timespec(struct timespec * spec){
    unsigned long ret= spec->tv_sec; //seconds
    ret = ret * 1000000000 + spec->tv_nsec; //nanoseconds
    return ret;
}


char * randomKey(){
    int size = 25;
    char* ret = (char*) malloc(sizeof(char)*size+1);
    for(int i = 0; i < size-1; i++){
        ret[i] = (char) rand()%26 + 'a';
    }
    ret[size-1] = '\0';
    return ret;
}

void SortedList_dump(SortedList_t *list){
    SortedListElement_t *temp = list->next;
    while(temp != list){
        if(temp->key==NULL){
            printf("NULL SHIT HERE\n");
        }
        else{
            printf("%s, length = %ld\n",temp->key, strlen(temp->key));
        }
        temp=temp->next;
    }
}

void signal_handler(int sig){
    if(sig == SIGSEGV)
        check(-1,"Segmentation Fault.",1);
}

void * thread_helper(void *ptr){
    SortedListElement_t *cur = ptr;
    if(m_flag) pthread_mutex_lock(&mutex); //M-case: acquire the lock
    if(s_flag){ //S-case: set spin and wait
        while (__sync_lock_test_and_set(&spin_lock, 1))
            while (spin_lock);
    }

    for(int i = 0; i < num_iterations; i++){
         SortedList_insert(head, (SortedListElement_t *) &cur[i]); //insert element into list in sorted manner
    }

    for(int i = 0; i < num_iterations; i++){
        SortedListElement_t * baller = SortedList_lookup(head, (char *) cur[i].key);
        if(baller==NULL){
            //printf("screwed up key is %s\n", cur[i].key);
            check(-1,"Corrupted List. Could not locate key in list.", 2);
        }
        SortedList_delete(baller);
    }

    if(s_flag) __sync_lock_release(&spin_lock); //S-case: release the lock once done
    if(m_flag) pthread_mutex_unlock(&mutex); //M-case: free the lock

    return NULL;
}

int main(int argc, char* argv[]){
    signal(SIGSEGV, signal_handler);

    num_threads = 1;
    num_iterations = 1;
    opt_yield=0; bool is_sync_passed=false;

    int ch;
    static struct option longopts[] = {
        { "threads", required_argument,  NULL,  't' },
        { "iterations", required_argument,  NULL,  'i' },
        { "yield", required_argument,  NULL,  'y' },
        { "sync", required_argument,  NULL,  's' },
        {  NULL,   0,            NULL,   0 }};

    while ((ch = getopt_long(argc, argv, "c", longopts, NULL)) != -1) {
        switch (ch) {
             case 't':
                     num_threads=atoi(optarg);
                     break;
             case 'i':
                     num_iterations=atoi(optarg);
                     break;
             case 'y': 
                    for(int i = 0; i < (int)strlen(optarg); i++){
                        if (optarg[i] == 'i') 
						    opt_yield = opt_yield | INSERT_YIELD;
					    else if (optarg[i] == 'd') 
						    opt_yield = opt_yield | DELETE_YIELD;
					    else if (optarg[i] == 'l') 
					        opt_yield = opt_yield | LOOKUP_YIELD;
                        else
                            check(-1,"Argument passed with \"yield\" must be \'i\', \'d\', or \'l\'.",1);
                    }
                    break;
             case 's':
                    sync_option=optarg[0];
                    is_sync_passed=true;
                    break;
             default:
                 check(-1, "Only valid parameters are \"--threads=thread_num\", \"--iterations=iter_num\", \"--yield\", and \"sync=x\".",1); 
        }
    }

   // printf("num threads = %d\n", num_threads);
   // printf("num iters = %d\n", num_iterations);
    int num_elements = num_threads * num_iterations;


    if(is_sync_passed){
        if(sync_option == 'm'){
            m_flag=true;
            // printf("m_flag babyyyy\n");
        }
        else if(sync_option=='s'){
            s_flag=true;
            // printf("s_flag babyyy\n");
        }
        else{
            check(-1,"Argument passed with \"sync\" must be \'m\' or \'s\'.", 1);
        }
    }


    head = (SortedList_t *) malloc(sizeof(SortedList_t)); //initialize the head pointer of linkedlist
    head->key=NULL;
    head->next=head;
    head->prev=head;

    temp = malloc(sizeof(SortedListElement_t)*num_elements); //initialize list of random ListElements 
    for(int i = 0; i < num_elements; i++){
        temp[i].key=randomKey();
        //printf("initialization: key = %s, length = %ld\n", temp[i].key, strlen(temp[i].key));
    }     

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);

    struct timespec begin, end; //clock initiation sequence 
    unsigned long diff = 0; clock_gettime(CLOCK_MONOTONIC, &begin);

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, &thread_helper, (void *)(&temp[i*num_iterations]));    

    for (int i = 0; i < num_threads; i++)
         x = pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end); //calculate clock time elapsed
    diff = get_nanosec_from_timespec(&end) - get_nanosec_from_timespec(&begin); 


    //SortedList_dump(head);
    //printf("size of list = %d\n", SortedList_length(head));
    if(SortedList_length(head)!=0)
        check(-1, "Corrupted List: Error deleting elements.", 2);
    // printf("Successful run! Threads did not corrupt list\n");

    //{none, i,d,l,id,il,dl,idl}
    printf("list-");
    if(opt_yield==0)
       printf("none-");
    else if (opt_yield == 1)
        printf("i-");
	else if (opt_yield == 2)
	    printf("d-");
	else if (opt_yield == 3)
	    printf("id-");
	else if (opt_yield == 4)
        printf("l-");
	else if (opt_yield == 5)
	    printf("il-"); 
	else if (opt_yield == 6)
	    printf("dl-"); 
	else if (opt_yield == 7)
	    printf("idl-"); 

    if(!is_sync_passed)
        printf("none,");
    else{
        if(s_flag)
            printf("s,");
        if(m_flag)
            printf("m,");
    }
    long operations = num_threads*num_iterations*3;
    long time_per_op = diff/operations;
    printf("%d,%d,%d,%ld,%ld,%ld\n", num_threads, num_iterations, 1, operations, diff, time_per_op);


    free(head);
    for(int i = 0; i < num_elements; i++)
        free((void *)temp[i].key);
    free(temp);
    free(threads);


}

