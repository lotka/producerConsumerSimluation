#include "helper.h"

int main (int argc, char *argv[])
{
  // Check for the correct number of arguements
  if(argc != 2)
  {
    fprintf(stderr,"Incorrect usage, expected: consumer id\n");
    return ERROR;
  }

  int id = check_arg(argv[1]);


  #ifdef VERBOSE
  fprintf(stderr,"Starting consumer with id: %i.\n",id);
  #endif 

  queue * data = NULL;
  int sem_key = -1;
  int shmid = -1;


  // Attach to shared memory and semaphores
  int result = attach_to_enviroment(shmid,data,sem_key);
  if(result == -1)
  {
    fprintf(stderr,"Consumer could not attach to semaphores or shared memory.");
    return ERROR;
  }

  // Report self as active user
  sem_wait(sem_key,SEM_MUTEX);
  ++data->active_users;
  sem_signal(sem_key,SEM_MUTEX);
  while(true)
  {
    if(sem_timewait(sem_key,SEM_ITEM,10) == -1)
    {
      long int end_time = (long)time(NULL)-data->creation_time;
      fprintf(stderr,"Consumer(%i) time %1.0ld: No Jobs left or too many consumers.\n", id, end_time);
      break;
    }
    sem_wait(sem_key,SEM_MUTEX);

      jobtype * job = data->job;

      // Save job info locally
      int sleep_time = job[data->front].duration;
      int job_id = job[data->front].id;

      // Delete the job
      job[data->front].id = 0;
      job[data->front].duration = 0;

      // Increment the front
      data->front = (data->front + 1) % (data->size);

    sem_signal(sem_key,SEM_MUTEX);
    sem_signal(sem_key,SEM_SPACE);

    // Get job creation time
    long int creation_time = (long)time(NULL)-data->creation_time;
    fprintf(stderr,"Consumer(%i) time  %li: Job id %i executing sleep duration %i\n",id,creation_time,job_id,sleep_time);

    // Pretend to be working for the job time
    sleep(sleep_time);

    // Get time when job finishes
    long int finish_time = (long)time(NULL)-data->creation_time;
    fprintf(stderr,"Consumer(%i) time %li: Job id %i completed\n",id,finish_time,job_id);
  }
  // Stop being an active user
  sem_wait(sem_key,SEM_MUTEX);
  --data->active_users;
  sem_signal(sem_key,SEM_MUTEX);


  // Wait until all jobs are done before attempting
  // the clean up
  sem_wait(sem_key,SEM_MUTEX);
  while(count_remaining_jobs(data) > 0)
  {
    #ifdef VERBOSE
    fprintf(stderr,"Waiting for all jobs to finish..\n");
    #endif
    sleep(1);
  }
  sem_signal(sem_key,SEM_MUTEX);


  int users = -1;
  while(users != 0)
  {
    // Signal that another consumer has started the clean up.
    if(data->active_users == -1)
    {
      fprintf(stderr,"Consumer(%i) Another consumer is preparing to do the cleanup. Quitting...\n",id);
      sem_wait(sem_key,SEM_END_OF_EXECUTION);
      return 0;
    }
    if(data->active_users == 0)
    {
      users = 0;
      sem_wait(sem_key,SEM_MUTEX);
      data->active_users = -1;
      sem_signal(sem_key,SEM_MUTEX);
    }
    sleep(2);
  }
  // Wait for other consumers to quit because of the -1
  // and ensure this is the only consumer
  sleep(10);

  // Signal which allows the producers and other consumers to quit
  sem_signal(sem_key,SEM_END_OF_EXECUTION);

  // Give the producers some time to exit
  sleep(1);
  fprintf(stderr,"Consumer(%i) Performing memory and semaphore cleanup.\n", id);

  // Delete the semaphores
  result = sem_close(sem_key);
  if(result != 0)
  {
      perror("sem_close");
      // Don't return, try to clear up shared memory
  }

  // Delete the shared memory
  result = shmctl(shmid, IPC_RMID, NULL);
  if(result == -1)
  {
      perror("shmdt");
      return ERROR;
  }

  return 0;
}