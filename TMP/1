/* Header file for lock implementation */

#ifndef _LOCK_H_

#define READ		1 /* constant for READ lock */
#define WRITE		2 /* constant for WRITE lock */
#define DELETED 	3 /* constant for DELETED lock */
#define FREE		0
#define IN_USE 		1
#define NLOCKS 		50 /* Number of locks allowed */
/* defining a lock structure */

struct lockstruct {
	int lrwd ;		/* for read write or delete indicator */
	int lpriority;		/* lock priority */
	int lstatus;		/* to check if lock is free */
	int ltime; 		/* begin waiting for the lock */
	int p[NPROC];		/* holds the process logs for the lock */
	int lqhead;		/* head of the queue for the lock */
	int lqtail;		/* tail of the queue of the lock */
	int next;			
	int num_reader;		/* number of lock readers */
	int num_writer;		/* number of lock writers */
	
};

extern struct lockstruct locktab[];
extern int nextl;
extern unsigned long ctr1000;
#endif
