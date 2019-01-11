/* Lock initialzaiton */

#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>

struct lockstruct locktab[];
nextl = 0;
 
SYSCALL linit(){
	kprintf("Initializing locks\n");
	int i;

	for(i=0;i<NLOCKS;i++){
		locktab[i].lrwd 	= 0;    /* initial status as zero */
		locktab[i].lpriority 	= -1;
		locktab[i].lstatus   	= FREE; /* all locks are free initially */
		locktab[i].lqhead	= newqueue();
		locktab[i].lqtail 	= locktab[i].lqhead + 1;
		int proc;
		for(proc = 0; proc<NPROC; proc++){
			locktab[i].p[proc] = 0;
		}
	}
	return OK;
}
