# Producer Consumer Simluation

A simulation exploring how to use system V semaphores and shared memory. 
Here is a outline of the function of each module:

## starter: 
	Construct a queue in shared memory and make semaphores.

## consumer:
	Consume jobs.
	If it gets stuck in the jobs queue for longer than 10 seconds stop consuming.
	Wait for all jobs to complete.
	If no other consumers are doing clean up, do clean up.
	If other consumers are doing clean up, wait for the end of execution semaphore.

## producer:
	Produce jobs.
	Wait for the end of execution semaphore and then quit.

## reader:
	Display a visualization of the que, also do clean up on exit.
