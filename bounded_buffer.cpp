#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <semaphore.h>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_THREADS     2


using namespace std;

//---Global vars---
vector<string> waypoint_buffer;
int thread_id[MAX_THREADS];	// User defined id for thread
pthread_t p_thread[MAX_THREADS];// Threads
pthread_attr_t attr;		// Thread attributes
pthread_mutex_t count_mx;	// Protects count
pthread_cond_t cond_queue_empty, cond_queue_full;
int waypoint_available;

int receiving_input(){
  cout<<"receiving input"<<endl;
  //check for receiving receiving waypoint input
  return 1;
}

void * produce(void * s){


  //Produce new waypoints to add to bounded buffer

  while(1){
    pthread_mutex_lock(&count_mx);
    while (waypoint_available == 4)
        pthread_cond_wait(&cond_queue_empty, &count_mx);
     //Insert waypoint
     for(int i=waypoint_buffer.size(); i<4; i++){
       waypoint_buffer.push_back("new_waypoint");
       cout<<"produce\n"<<endl;

     }
     waypoint_available = waypoint_buffer.size();

     pthread_cond_signal(&cond_queue_full);
     pthread_mutex_unlock(&count_mx);

   }

   pthread_exit(NULL);
}

void * consume(void * s){
  //Consume
  while(1) {
    pthread_mutex_lock(&count_mx);
     while (waypoint_available < 4)
        pthread_cond_wait(&cond_queue_full, &count_mx);
     //Extract from queue
     waypoint_buffer.erase(waypoint_buffer.begin());
     cout<<"consume!\n"<<endl;
     waypoint_available = 3;

     pthread_cond_signal(&cond_queue_empty);
     pthread_mutex_unlock(&count_mx);

     //Process task
     usleep(1000000);
  }
   pthread_exit(NULL);
}

int main(){
  // Initialize mutex and attribute structures
  waypoint_available = 0;
  pthread_cond_init(&cond_queue_empty, NULL);
  pthread_cond_init(&cond_queue_full, NULL);
  pthread_mutex_init(&count_mx, NULL);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


  //buffer full -> CONSUME!
  thread_id[0] = 0;
  thread_id[1] = 1;
  pthread_create(&p_thread[0], &attr, produce, (void*) &thread_id[0]);

  //buffer not full -> PRODUCE!
  pthread_create(&p_thread[1], &attr, consume, (void*) &thread_id[1]);

  pthread_join(p_thread[0], NULL);
  pthread_join(p_thread[1], NULL);

  // Destroy mutex and attribute structures
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&count_mx);

  return 0;
}
