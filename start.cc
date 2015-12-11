/******************************************************************
 * Program for setting up semaphores/shared memory before running
 * the Producer and Consumer 
 * 
 * Outline of operation:
 * 1. Create shared memory
 * 2. Set up job strcture
 * 3. Create semaphores

The plan for the consumer is for it to only kill when its waiting 10 seconds
and its check there are no more jobs to be done
I think this is an nice compramise :) 


 * 
 ******************************************************************/

#include "helper.h"

int main (int argc, char **argv)
{
  if(argc != 2)
  {
    fprintf(stderr,"Incorrect usage, expected: start queueSize\n");
    return ERROR;
  }

  int queue_size = check_arg(argv[1]);

  if(not (0 < queue_size and queue_size < BUFFER_SIZE))
  {
    fprintf(stderr,"Queue size invalid. Quitting...\n");
    return ERROR;
  }



  queue * data = NULL;

  #ifdef VERBOSE
  fprintf(stderr,"Creating queue with size %i.\n",queue_size);
  #endif

  // Create shared memory
  int shmid = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT);
  // Check shmget found a valid shmid
  if (shmid == -1)
  {
      perror("shmget");
      return shmid;
  }
  // Semaphores
  int sem_key = sem_create(SEM_KEY,4);
  sem_init(sem_key,SEM_ITEM,  0);
  sem_init(sem_key,SEM_SPACE, queue_size);
  sem_init(sem_key,SEM_MUTEX, 1);
  sem_init(sem_key,SEM_END_OF_EXECUTION, 0);

  int result = attach_to_enviroment(shmid,data,sem_key);
  if(result == -1)
  {
    printf("Failed to attach to semaphore or shared memory.\n");
    return ERROR;
  }

  // Initialize the circular queue
  data->front = 0;
  data->end   = 0;
  data->active_users = 0;
  data->size = queue_size;
  data->creation_time = (long)time(NULL);

  // Get the pointer to the job queue
  jobtype * job = data->job;

  // Zero all the jobs
  for(int i = 0; i < data->size; ++i)
  {
    job[i].id = 0;
    job[i].duration = 0;
  }

  assert(count_remaining_jobs(data) == 0);

  // detach from the shared memory
  result = shmdt(data);
  if (result == -1)
  {
      perror("shmdt");
      return ERROR;
  }

  // // Delete the semaphores
  // result = sem_close(sem_key);
  // if(result != 0)
  // {
  //     perror("sem_close");
  //     // Don't return, try to clear up shared memory
  // }

  // // Delete the shared memory
  // result = shmctl(shmid, IPC_RMID, NULL);
  // if(result == -1)
  // {
  //     perror("shmdt");
  //     return ERROR;
  // }

  return 0;
}
