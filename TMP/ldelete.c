/* Deletion of the locks */

#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>

SYSCALL ldelete(int ldes){

	int pid;
	struct lockstruct *l;
	

	if(ldes < 0 || ldes > NLOCKS){
		kprintf("lock descriptor %d is invalid\n",ldes);
		return SYSERR;
	}
	if(locktab[ldes].lstatus == FREE || locktab[ldes].lstatus == DELETED){
		kprintf("lock status is invalid\n");
		return SYSERR;
	}
	if(locktab[ldes].lrwd == -1){
		kprintf("lock is neither for read, write or delete\n");
		return SYSERR;
	}
	
	/* free the lock and make the lrwd as deleted(required condition as per the question) */
	locktab[ldes].lstatus = FREE;	
	locktab[ldes].lrwd    = DELETED;
	
	// The locked process goes into the ready state after lock is deleted
	// so we remove the process from the process list and make its status as deletd
	int qhead = locktab[ldes].lqhead;
	qhead = nonempty(qhead);

	if(qhead){
		while((pid = getfirst(locktab[ldes].lqhead))!=-1){
			proctab[pid].lrwd[ldes] = DELETED;
			proctab[pid].waitforlock = 0;
			proctab[pid].pwaitret = DELETED;
			ready(pid,RESCHNO); // RESCHNO does not allow the process to be rescheduled.
		}
		resched();
	}

	return OK;
	
}
