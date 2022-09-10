/*
	This program will create a Shared Memory Object and read interger values.
	This values had been wriiten by some other Shared Memory write program.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <semaphore.h>

typedef struct shmstruct
{
	int count;
}SHMSTRUCT;

sem_t * sem_mutex;			// for semaphore lock


int main(int argc, char **argv)
{
	// variable declaration - start
	int shm_fd = -1;
	int ret_val = 0;
	int index = 0;

	struct stat shmfd_strct;
	SHMSTRUCT *cnt_ptr;
	
	pid_t pid = 0;
	// variable declaration - end

	//fprintf(stdout, "Inside main\n");
	if(3 != argc)
	{
		fprintf(stdout, "Wrong no of arguments\n");
		fprintf(stdout, "After exe give one shared memory argument path and one semaphore lock\n");
		exit(EXIT_FAILURE);
	}
	

	// function to open a shared memory object descriptor using shm_open()
	fprintf(stdout, "%s\n", argv[0]);
	fprintf(stdout, "%s\n", argv[1]);
	fprintf(stdout, "%s\n", argv[2]);
	//shm_fd = shm_open(argv[1], O_CREAT|O_RDWR, S_IRWXU);
	shm_fd = shm_open(argv[1], O_RDWR, S_IRWXU);				// if not writable then can not change count value
	if(-1 == shm_fd)
	{
		fprintf(stdout, "Unable to open shared memory object descriptor\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}


/*
CAN NOT UPDATE FOR READ ONLY	
	// update the length of shared memory object
	ret_val = ftruncate(shm_fd, 256);
	if(-1 == ret_val)
	{
		fprintf(stdout, "Unable to update shared memory object lenghth\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}

*/

	// get info of shared memory object (st_mode, st_size, st_uid, st_gid)
	shmfd_strct.st_mode = 0;	shmfd_strct.st_uid = 0;
	shmfd_strct.st_size = 0;	shmfd_strct.st_gid = 0;
	
	ret_val = fstat(shm_fd, &shmfd_strct);
	if(-1 == ret_val)
	{
		fprintf(stdout, "Unable to get info of shared memory object\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}
		
	

	// map the descriptor to shared memory
	cnt_ptr = (SHMSTRUCT *)mmap(NULL, shmfd_strct.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	//cnt_ptr = (SHMSTRUCT *)mmap(NULL, shmfd_strct.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
	if(MAP_FAILED == cnt_ptr)
	{
		fprintf(stdout, "Unable to map shared memory object \n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}


	// after call to mmap descriptor may be closed
	ret_val = close(shm_fd);
	if(-1 == ret_val)
	{
		fprintf(stdout, "Unable to close descriptor\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}

/*
	// open semaphore
	ret_val = open("semOBJpath", O_CREAT | O_RDWR, S_IRWXU);
	if(-1 == ret_val)
	{
		fprintf(stdout, "Unable to open semaphore-1\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}

*/
	//sem_mutex = sem_open(argv[2], O_CREAT, S_IRWXU, 1);
	sem_mutex = sem_open(argv[2], 0);
	if(SEM_FAILED == sem_mutex)
	{
		fprintf(stdout, "Unable to open semaphore\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}
	
	
	pid = getpid();
	fprintf(stdout, "cnt_ptr: %p\n", cnt_ptr);
	fprintf(stdout, "cnt_ptr->count: %d\n\n", cnt_ptr->count);
	cnt_ptr->count = 0;
	for (index = 0; index < 5; index++)
	{
		//fprintf(stdout, "inside for loop\n");
		ret_val = sem_trywait(sem_mutex);
		if(-1 == ret_val)
		{
			fprintf(stdout, "sem_wait failed\n");
			perror(" ERROR: ");
			exit(EXIT_FAILURE);
		}
		//fprintf(stdout, "PID: %ld	count: %d\n", (long)pid, index);
		fprintf(stdout, "PID: %ld count: %d\n", (long)pid, cnt_ptr->count++);
		sleep(1);
		ret_val = sem_post(sem_mutex);
		if(-1 == ret_val)
		{
			fprintf(stdout, "sem_post failed\n");
			perror(" ERROR: ");
			exit(EXIT_FAILURE);
		}

	}
	
	
	// close semaphore
	ret_val = sem_close(sem_mutex);
	if(-1 == ret_val)
	{
		fprintf(stdout, "Unable to close semaphore\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}


	// unlink semaphore
	ret_val = sem_unlink(argv[2]);
	if(-1 == ret_val)
	{
		fprintf(stdout, "Unable to unlink semaphore\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}



	// remove maping of shared memory object
	ret_val = munmap(NULL, shmfd_strct.st_size);
	if(-1 == ret_val)
	{
		fprintf(stdout, "Unable to unmap shared memory object\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}

		 
	ret_val = shm_unlink(argv[1]);
	if(-1 == ret_val)
	{
		fprintf(stdout, "Unable to unlink shared memory object\n");
		perror(" ERROR: ");
		exit(EXIT_FAILURE);
	}
	

	fprintf(stdout, "\n");
	exit(EXIT_SUCCESS);

}	// end of int main(int argc, char **argv)


/*******************
		END OF FILE
********************/