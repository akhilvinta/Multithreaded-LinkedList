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

void check(int error, char *message){
    if(error < 0){
        fprintf(stderr, "%s Exiting with status 1.\n", message);
        exit(1);
    }
}

static inline unsigned long get_nanosec_from_timespec(struct timespec * spec){
    unsigned long ret= spec->tv_sec; //seconds
    ret = ret * 1000000000 + spec->tv_nsec; //nanoseconds
    return ret;
}


long long counter = 0; //counter which will be printed at end of main
bool m_flag=false, s_flag=false, c_flag=false, y_flag = false;  //flags for parameters
int spin_lock, x = 0;
int iterations; char sync_option; 
pthread_mutex_t mutex; 

void add(long long *pointer, long long value){

    if(c_flag){
        long long prev, sum;
        do {
            prev = *pointer;
            sum = prev + value;
            if (y_flag)
                sched_yield();
        } while(__sync_val_compare_and_swap (pointer, prev, sum) != prev);
        return;
    }


    if(m_flag) pthread_mutex_lock(&mutex); //M-case: acquire the lock
    if(s_flag){ //S-case: set spin and wait
        while (__sync_lock_test_and_set(&spin_lock, 1))
            while (spin_lock);
    }

    long long sum = *pointer + value;
    if (y_flag)
        sched_yield();
    *pointer=sum;

    if(s_flag) __sync_lock_release(&spin_lock); //S-case: release the lock once done
    if(m_flag) pthread_mutex_unlock(&mutex); //M-case: free the lock

}


void * thread_helper(){
    for (int i = 0; i < iterations; i++)
        add(&counter,1);
    for (int i = 0; i < iterations; i++)
        add(&counter,-1);
    return NULL;

}

int main(int argc, char* argv[]) {

    int num_threads=1;
    iterations=1;
    bool is_sync_passed = false;

    int ch;
    static struct option longopts[] = {
        { "threads", required_argument,  NULL,  't' },
        { "iterations", required_argument,  NULL,  'i' },
        { "yield", no_argument,  NULL,  'y' },
        { "sync", required_argument,  NULL,  's' },
        {  NULL,   0,            NULL,   0 }};

    while ((ch = getopt_long(argc, argv, "c", longopts, NULL)) != -1) {
        switch (ch) {
             case 't':
                     num_threads=atoi(optarg);
                     break;
             case 'i':
                     iterations=atoi(optarg);
                     break;
             case 'y': 
                    y_flag = true;
                    break;
             case 's':
                    sync_option=optarg[0];
                    is_sync_passed=true;
                    break;
             default:
                 check(-1, "Only valid parameters are \"--threads=thread_num\", \"--iterations=iter_num\", \"--yield\", and \"sync=x\". "); 
        }
    }

    if(is_sync_passed){
        if(sync_option == 'm'){
            m_flag=true;
        }
        else if(sync_option=='s'){
            s_flag=true;
        }
        else if(sync_option=='c'){
            c_flag=true;
        }
        else{
            check(-1,"Argument passed with \"sync\" must be \'m\', \'s\', or \'c\'.");
        }
    }


    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    
    if(m_flag)
        pthread_mutex_init(&mutex, NULL);

    struct timespec begin, end; //clock initiation sequence 
    unsigned long diff = 0; clock_gettime(CLOCK_MONOTONIC, &begin);


    for(int i = 0; i < num_threads; i++){
        x=pthread_create(&threads[i], NULL, &thread_helper, NULL);
        check(x,"Failed to create thread.");
    }
    for (int i = 0; i < num_threads; i++){
         x = pthread_join(threads[i], NULL);
         check(x,"Failed to join thread.");
    }

    clock_gettime(CLOCK_MONOTONIC, &end); //calculate clock time elapsed
    diff = get_nanosec_from_timespec(&end) - get_nanosec_from_timespec(&begin); 

    int ops_performed = num_threads * iterations * 2;
    long time_per_op = diff/ops_performed;
    if(!y_flag && !is_sync_passed)
        printf("add-none,%d,%d,%d,%lu,%ld,%lld\n",num_threads, iterations,ops_performed,diff,time_per_op,counter);
    else if(!y_flag && m_flag)
        printf("add-m,%d,%d,%d,%lu,%ld,%lld\n",num_threads, iterations,ops_performed,diff,time_per_op,counter);
    else if(!y_flag && s_flag)
        printf("add-s,%d,%d,%d,%lu,%ld,%lld\n",num_threads, iterations,ops_performed,diff,time_per_op,counter);
    else if(!y_flag && c_flag)
        printf("add-c,%d,%d,%d,%lu,%ld,%lld\n",num_threads, iterations,ops_performed,diff,time_per_op,counter);
    else if(y_flag && !is_sync_passed)
        printf("add-yield-none,%d,%d,%d,%lu,%ld,%lld\n",num_threads, iterations,ops_performed,diff,time_per_op,counter);
    else if(y_flag && m_flag)
        printf("add-yield-m,%d,%d,%d,%lu,%ld,%lld\n",num_threads, iterations,ops_performed,diff,time_per_op,counter);
    else if(y_flag && s_flag)
        printf("add-yield-s,%d,%d,%d,%lu,%ld,%lld\n",num_threads, iterations,ops_performed,diff,time_per_op,counter);
    else if(y_flag && c_flag)
        printf("add-yield-c,%d,%d,%d,%lu,%ld,%lld\n",num_threads, iterations,ops_performed,diff,time_per_op,counter);
    else
        check(-1,"Invalid combination of parameters. ");

    // printf("%lld\n", counter);
    // printf("Time elapsed: %lu nanoseconds\n", diff);
    free(threads);
    exit(0);
}
