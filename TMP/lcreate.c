/* procedure to create a lock */

#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <q.h>

SYSCALL lcreate(){

	int i,lck_des=0;
	for(i=0;i<NLOCKS;i++){
		lck_des = nextl++;
		if(nextl >= NLOCKS)
			nextl = 0;
		if(locktab[lck_des].lstatus == FREE){
			locktab[lck_des].lstatus = IN_USE;
			locktab[lck_des].lrwd	 = -1;
			//proctab[currpid].lrwd	 = -1;
			kprintf("Lock: %d, Process id: %d", lck_des,currpid);
			return lck_des;
		}	
	}
	return SYSERR;
}
