/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;
	disable(ps);
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	struct  lockstruct *lptr;
	int	dev;
	int i=0;
	struct lockstruct *lock_ptr;
	int temp =0;

	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

    case PROC_LOCK:
		lock_ptr = &locktab[pptr->l_proc];
	    if(lock_ptr->p[currpid] == 0){
        	swap(lock_ptr->p[currpid],lock_ptr->p[currpid]+1);
    	}
    	else if(lock_ptr->p[currpid] > 0){
			temp=0;
        	while(temp < 0){
            	swap(lock_ptr->p[currpid]+NLOCKS,lock_ptr->p[currpid]);
        	}
    	}
    	else{
        	swap(lock_ptr->p[currpid]+NLOCKS,lock_ptr->p[currpid]+NPROC);
    	}

		handle_kill_lock(pid);
		break;
	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}

void handle_kill_lock(int pid){
	struct	pentry	*pptr;	
	struct  lockstruct *lptr;
	dequeue(pid);
	pptr= &proctab[pid];
	lptr = &locktab[pptr->l_proc];

	locktab[pptr->l_proc].p[pid] = 0;
	change_lpriority(pptr->l_proc);
	int i=0;
	while(i < NPROC){
		if(lptr->p[i] > 0)
			update_lock_prio(i);
		i++;
	}
	pptr->pstate = PRFREE;
}
