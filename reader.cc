// Reader 

# include "helper.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
int shmid;
int sem_key;

void clean_up(int sig) 
{
  int res;

  printf("Recieved signal: %i\n",sig);
  res = shmctl(shmid, IPC_RMID, NULL);
  if(res != 0)
  {
    printf("Failed to free shared memory! Got %i from shmctl\n",res);
  }

  res = sem_close(sem_key);
  if(res != 0)
  {
    printf("Failed to close semaphore! Got %i from sem_close\n",res);
  }
  exit(0);
}

// This is admittedly badly written!
// But I don't have time yet to clean it up.

int main (int argc, char ** argv)
{

  signal(SIGINT, clean_up);

  sem_key = sem_attach(SEM_KEY);
  queue * data;
  /* connect to (and possibly create) the segment: */
  if ((shmid = shmget(SHM_KEY, SHM_SIZE, 0644 | IPC_CREAT)) == -1)
  {
      perror("shmget");
      return 1;
  }

  while(true)
  {

    // attach to the segment to get a pointer to it:
    data = (queue *)shmat(shmid, (void *)0, 0);
    if (data == (queue *)(-1))
    {
      printf("Shared memory was deleted!\n");
      exit(1);
    }
    long int time_since_genesis = (long)time(NULL)-data->creation_time;
    jobtype * job = data->job;
    int quit = 1;
    for(int i = 0; i < data->size; ++i)
    {
      if(job[i].id!=0)
      {
        quit = 0;
      }
    }
    if(quit and (time_since_genesis > 300))
    {
      printf("The reader is bored, quitting.\n");
      break;  
    }

    printf("The time is %ld\n", time_since_genesis);
    if(data->front > data->end)
    {
      printf("AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH");
    }
    printf("Front: %i End: %i Size: %i\n", data->front, data->end, data->size);

    printf("QUE: [");
    for(int i = 0; i < data->size; ++i)
    {
        jobtype * job = data->job;
        if(job[i].id == 0)
        {
          printf(" _ ");
        }
        else
        {
          printf("[%i]", job[i].duration);
        }
    }
    printf("]\n");

    int front = data->front;
    int end = data->end;
    if(front==end)
    {
      front = 0;
      end = data->size;
    }
    for(int i = front; i <= end; ++i)
    {
      if(job[i].id != 0)
      {
        printf("job_id: %i duration: %i\n", job[i].id, job[i].duration);
      }
    }
    sleep(1);
    system("clear");
  }

  // detach from the segment
  if (shmdt(data) == -1) {
      perror("shmdt");
      exit(1);
  }
  clean_up(0);
  return 0;
}
