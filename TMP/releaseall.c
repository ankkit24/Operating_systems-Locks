#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <q.h>
#include <lock.h>

int releaseall(int numlocks,int args, ...){
    	STATWORD ps;    
    	disable(ps);    
    
    	int queue_node,lock_prio,lock, lock_index,type_lock;
	int max=-1;
	int reader_lock =-1, writer_lock = -1;
	
    	struct lockstruct *l;
	struct pentry *pptr;

    	int i=0;
    	while(i<numlocks) {
        	int flag = -1;
		lock = (int)(*((&args) + i));
       		l = &locktab[lock];
		
		// validating lock
        	if(lock < 0 || lock > NLOCKS)
            		return(SYSERR);

		//condition to swap the process logs of the locks 	
        	if(l->p[currpid] == 0)
            		swap(l->p[currpid],l->p[currpid]+1);
        	else
            		swap(l->p[currpid]+NLOCKS,l->p[currpid]+NPROC);
	
       		l->p[currpid] = 0;
        	proctab[currpid].lrwd[lock] = 0;
		pptr = &proctab[currpid];
		
		int i;
		for(i=0; i<NLOCKS; i++){			
			if(pptr->lrwd[i] > 0 && max < locktab[i].lpriority)
				max = locktab[i].lpriority;
		}
		
		if(pptr->pprio <= max)
			pptr->pinh = max;
		else if(pptr->pprio > max)
			pptr->pinh = 0;

		if(l->num_writer > 0)
			l->num_writer --;
		if(l->num_reader > 0)
			l->num_reader --;

		unsigned long t1=0, t2=0;

        	queue_node = q[l->lqtail].qprev;
        	lock_prio = type_lock = 0;
  	    	if(q[queue_node].qkey == q[q[queue_node].qprev].qkey){
            		lock_prio = q[queue_node].qkey;
  			//traversing the lock queue to get the type of the lock i.e reader or writer lock
            		while(q[queue_node].qkey == lock_prio) { 
				if(q[queue_node].qtype == READ && q[queue_node].qtime > t1){
					 reader_lock = queue_node;
					 break;
				}else if(q[queue_node].qtype == WRITE && q[queue_node].qtime > t2){
					  writer_lock = queue_node; 
					  break;
				}else
					break;
                		if(reader_lock >= 0){
                    			if(writer_lock >= 0){
                        			if (t1-t2 < 1000 || t2 - t1 < 1000){
			                		type_lock = writer_lock;	
		                		}else if(t1 > t2) {
            			    			type_lock = reader_lock;
        		        		}
		                		else if(t1 < t2) {
    			            			type_lock = writer_lock;
		                		}		
                    			}
                		}
                		queue_node = q[queue_node].qprev;
            		}
			
            		if(lock < 0){
                		while(l->lstatus==FREE){
					l->num_reader	= l->p[lock];
                    			l->lstatus 	= READ;
                    			l->lqhead 	= q[type_lock].qkey;
                		}
            		}

            		if(l->num_reader == 0 && flag == -1){
                		if(l->num_writer == 0){
                    			if(q[type_lock].qtype == WRITE){
                        			release_write(lock,type_lock);
                        			flag = 0;
                    			}
                		}
            		}else if(l->num_writer == 0 && flag==-1){
                		if(q[type_lock].qtype == WRITE){
                    			reader_check(lock);
                    			flag = 0;
                		}
            		} 
	  	}
  
        	if(q[queue_node].qkey != q[q[queue_node].qprev].qkey) {
			if(q[queue_node].qtype==READ && l->num_writer == 0){
				reader_check(lock);
			}else if(q[queue_node].qtype==WRITE && l->num_reader == 0)
				release_write(lock,queue_node);
        	}
		//while increment i
		i++;
		if(i>=numlocks)
			break;
	}	
    	resched();
    	return(OK);
}

void reader_check(int l_id) {
    	struct lockstruct *l;
    	l = &locktab[l_id];
    	int x, q_prev, max=-1;

    	for(x=q[l->lqtail].qprev;x!=l->lqhead;x=q[x].qprev){
        	if(q[x].qkey > max){
            		if(q[x].qtype == WRITE){
                		max = q[x].qkey;
            		}
        	}
    	}
    	for(x=q[l->lqtail].qprev;x!=l->lqhead;){
        	if(q[x].qkey >= max){
            		if(q[x].qtype == READ){
                		q_prev = q[x].qprev;
                		release_write(l_id,x);
                		x = q_prev;
            		}
        	}
    	}
}

void update_lock_prio(int id){
     	struct pentry *pptr;
     	pptr = &proctab[id];
     	int i=0,max=-1;
     	for(i=0;i<NLOCKS;i++){      
        	if(pptr->lrwd[i] <= 0 && max < locktab[i].lpriority)
               		max = locktab[i].lpriority;    
         	else if(pptr->lrwd[i] > 0 && max < locktab[i].lpriority)
               		max = locktab[i].lpriority;
     	}
   	pptr->pinh = ( pptr->pprio > max ? 0 : max );
}

//method to perform swap 
void swap(int global_max_prio, int local_max_prio){
	if(global_max_prio >= 0 && local_max_prio >=0){
		int temp = global_max_prio;
		global_max_prio = local_max_prio;
		local_max_prio = temp;
	}
}


void release_write(int l_id, int p) {
    	struct lockstruct *l;
    	l = &locktab[l_id];
    	l->p[p] = 1;
    	proctab[currpid].lrwd[l_id] = 1;

    	int type = q[p].qtype;
    	if(type == WRITE)
		l->num_writer ++;
    	if(type == READ)
		l->num_reader++;

    	//calling the function to change the priority
    	change_lpriority(l_id);
    	int i=0;
    	for(i =0; i<NPROC; i++){
		if(l->p[i]>0)
	   		update_lock_prio(i);	
    	}
    	dequeue(p);
    	ready(p, RESCHNO);
}

void change_lpriority(int l_id){
    	struct lockstruct *l;
    	int max_priority = 0;
    	l = &locktab[l_id];
    	int temp = q[l->lqtail].qprev;

    	while(temp!=l->lqhead){
        	if(proctab[temp].pprio > max_priority)
                	max_priority = proctab[temp].pprio;
        	temp = q[temp].qprev;
    	}
    	l->lpriority = max_priority;
}
