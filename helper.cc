/******************************************************************
 * The helper file that contains the following helper functions:
 * attach_to_enviroment - Attach to the shared memory and semaphores
 * count_remaining_jobs - Count the remaining jobs
 * check_arg - Checks if command line input is a number and returns it
 * sem_create - Create number of sempahores required in a semaphore array
 * sem_attach - Attach semaphore array to the program
 * sem_init - Initialise particular semaphore in semaphore array
 * sem_wait - Waits on a semaphore (akin to down ()) in the semaphore array
 * sem_timewait - Waits on a semaphore for a particular time
 * sem_signal - Signals a semaphore (akin to up ()) in the semaphore array
 * sem_close - Destroy the semaphore array
 ******************************************************************/

# include "helper.h"

int attach_to_enviroment(int &shmid, queue * &data, int &sem_key)
{
  #ifdef VERBOSE
  fprintf(stderr,"\tAttaching to shared memory.\n");
  #endif

  #ifdef VERBOSE
  fprintf(stderr,"\tGet shared memory pointer.\n");
  #endif

  shmid = shmget(SHM_KEY, SHM_SIZE, 0644);
  if (shmid == -1)
  {
      perror("shmget");
      return -1;
  }

  // Using shmid, get a pointer to the shared memory
  data = (queue *)shmat(shmid, (void *)0, 0);
  if (data == (queue *)(-1))
  {
      perror("shmat");
      return -1;
  }
  #ifdef VERBOSE
  fprintf(stderr,"\tAttaching to sempahore.\n");
  #endif
  // Get sem_key
  sem_key = sem_attach(SEM_KEY);
  if (sem_key == -1)
  {
      perror("sem_attach");
      return shmid;
  }

  return 0;
}

int count_remaining_jobs(queue * data)
{
  jobtype * job = data->job;
  int number_of_jobs = 0;
  for(int i = 0; i < data->size; ++i)
  {
    if(job[i].id != 0)
    {
      ++number_of_jobs;
    }
  }
  return number_of_jobs;
}

int check_arg (char *buffer)
{
  int i, num = 0;
  for (i=0; i < (int) strlen (buffer); i++)
  {
    if ((0+buffer[i]) > 57)
      return -1;
    num += pow (10, strlen (buffer)-i-1) * (buffer[i] - 48);
  }
  return num;
}

int sem_attach (key_t key)
{
  int id;
  if ((id = semget (key, 1,  0)) < 0)
    return -1;
  return id;
}
 
int sem_create (key_t key, int num)
{
  int id;
  if ((id = semget (key, num,  0666 | IPC_CREAT | IPC_EXCL)) < 0)
    return -1;
  return id;
}

int sem_init (int id, int num, int value)
{
  union semun semctl_arg;
  semctl_arg.val = value;
  if (semctl (id, num, SETVAL, semctl_arg) < 0)
    return -1;
  return 0;
}

void sem_wait (int id, short unsigned int num)
{
  struct sembuf op[] = {
    {num, -1, SEM_UNDO}
  };
  semop (id, op, 1);
}

int sem_timewait (int id, short unsigned int num, int tim)
{
  struct timespec ts = {tim, 0};
  struct sembuf op[] = {
    {num, -1, SEM_UNDO}
  };
  if (semtimedop(id, op, 1, &ts ) == -1 )
    if (errno == EAGAIN)
      return -1;
  return 0;
}

void sem_signal (int id, short unsigned int num)
{
  struct sembuf op[] = {
    {num, 1, SEM_UNDO}
  };
  semop (id, op, 1);
}

int sem_close (int id)
{
  if (semctl (id, 0, IPC_RMID, 0) < 0)
    return -1;
  return 0;
}
