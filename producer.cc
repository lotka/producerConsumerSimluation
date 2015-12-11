#include "helper.h"

int main (int argc, char *argv[])
{
  if(argc != 3)
  {
    fprintf(stderr,"Incorrect usage, expected: producer id jobs\n");
    return ERROR;
  }

  int id = check_arg(argv[1]);
  int jobs = check_arg(argv[2]);

  if(not (0 < jobs and jobs < BUFFER_SIZE))
  {
    fprintf(stderr,"Job count invalid. Quitting...\n");
    return ERROR;
  }


  #ifdef VERBOSE
  fprintf(stderr,"Starting Producer with id: %i.\n",id);
  #endif 

  queue * data = NULL;
  int sem_key = -1;
  int shmid = -1;
  // Attach to shared memory and semaphores
  int result = attach_to_enviroment(shmid,data,sem_key);
  if(result == -1)
  {
    fprintf(stderr,"Producer could not attach to semaphores or shared memory.");
    return ERROR;
  }
  // Add one to active users
  sem_wait(sem_key,SEM_MUTEX);
  ++data->active_users;
  sem_signal(sem_key,SEM_MUTEX);

  // Get a pointer
  jobtype * job = (data->job);

  for(int i = 0; i < jobs; ++i)
  {
    sem_wait(sem_key,SEM_SPACE);
    sem_wait(sem_key,SEM_MUTEX);

      // Store the index of the new job
      int index = data->end;

	    // Create new job
	    job[index].id = index+1;
	    job[index].duration = 2 + (int)(5.0*rand()/RAND_MAX);

	    // Print information about job
	    long int time_since_creation = (long)time(NULL) - data->creation_time;
	    fprintf(stderr,"Producer(%i) time %li: Job id %i duration %i\n", id, time_since_creation, job[index].id, job[index].duration);

      // Increment the end
      data->end = (data->end + 1) % data->size;

    sem_signal(sem_key,SEM_MUTEX);
    sem_signal(sem_key,SEM_ITEM);

    // Sleep before producing another job
    sleep(2 + (int)(2.0*rand()/RAND_MAX));
  }

  long int end_time = (long)time(NULL) - data->creation_time;
  fprintf(stderr,"Producer(%i) time %ld: No more jobs to generate.\n", id, end_time);

  // Decrement active users
  sem_wait(sem_key,SEM_MUTEX);
  --data->active_users;
  sem_signal(sem_key,SEM_MUTEX);

  // Wait for the end of execution and then quit
  sem_wait(sem_key,SEM_END_OF_EXECUTION);

  // detach from the shared memory
  result = shmdt(data);
  if (result == -1)
  {
      perror("shmdt");
      return ERROR;
  }

  fprintf(stderr,"Producer(%i) Quitting...\n",id);

  return 0;
}