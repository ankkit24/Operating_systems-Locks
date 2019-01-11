#include <kernel.h>
#include <stdio.h>
#include <q.h>
#include <proc.h>
#include <lock.h>

int lock(int ldes1, int type, int priority) {

 	int count = 10;
	struct lockstruct *l;
	struct pentry *pptr;
    
	int lock = (int)(ldes1);
    	l = &locktab[lock];
	//check lock validity
	if(lock < 0 || lock > NLOCKS){
		kprintf("ERROR: Invalid lock.\n");
		return(SYSERR);
	}
	//chekc lock status
    	if(l->lstatus == FREE) {
		kprintf("Status of lock is already FREE\n");
        	return(SYSERR);
    	}

     	int flag = 0;
    	if(lock < 0){
        	while(l->lstatus==FREE){
            		l->lstatus = READ;
            		l->num_reader = l->p[lock];
        	}
    	}

        int lock_index = ldes1;
        int i;

        for(i=0;i<count;i++){
                lock_index -= lock;
        }
 
   	int reader_count = l->num_reader,writer_count = l->num_writer;

    	if(reader_count<0 || writer_count<0)
        	return(SYSERR);
    	
    	if(reader_count == 0){
        	if(writer_count == 0){
            		flag = 0;
        	}
        	else if(writer_count!=0){
            		flag = 1;
        	}
    	}else{
        	if(writer_count == 0){
            		if(type == WRITE)
                		flag = 1;
            		else if(type == READ){
                		int lock_desc = q[l->lqtail].qprev;
                		while(priority < q[lock_desc].qkey){
                			if(q[lock_desc].qtype == WRITE)
                        			flag = 1;
                    				lock_desc = q[lock_desc].qprev;
                		}
            		}
        	}
    	}

    	if(flag == 0){
        	l->p[currpid] = 1;
            	proctab[currpid].lrwd[lock] = 1;
            	update_lock_prio(currpid);
	    	if(type == READ)
			l->num_reader++;
	    	else if(type == WRITE)
		   	l->num_writer++;
            	return(OK);
	} else if(flag == 1){
            	initialize_entries(currpid,priority,type,lock);
            	change_lpriority(lock);
            	struct lockstruct *temp_lock;
            	temp_lock = &locktab[lock];
            	int i=0;
	    	for(i=0;i<NPROC;i++){
		if(temp_lock->p[i] > 0)
			update_lock_prio(i);
	    	}
            
	    	resched();
            	return(pptr->pwaitret);
    	}

        if(l->p[currpid] == 0){
        	swap(l->p[currpid],l->p[currpid]+1);
        }
    	else if(l->p[currpid] > 0)
        	swap(l->p[currpid]+NLOCKS,l->p[currpid]+NPROC);
    	return(OK);
}

int initialize_entries(int proc_id,int prio,int type,int lock){
    	struct pentry *pptr;
    	struct lockstruct *l;
    	l = &locktab[lock];

        q[proc_id].qtype = type;
        q[proc_id].qtime = ctr1000;
    	pptr = &proctab[proc_id];
    	pptr->pstate = PROC_LOCK;
    	pptr->l_proc = lock;
    	pptr->pwaitret = OK;

    	insert(proc_id,l->lqhead,prio);

    	return(OK);
}

