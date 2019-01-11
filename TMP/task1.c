#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <sem.h>
#include <stdio.h>


void process1(int p1){
        int i=0;
	kprintf("A starts:\n");
        while(i++ < 50)
                kprintf("A");
	kprintf("\nA sleeps...\n");
        sleep(1);
        i=0;
	kprintf("A wakes and starts again:\n");
        while(i++<50)
                kprintf("A");
	kprintf("\nA completes!!\n");
}

void process2(int p2){
        int x = wait(p2);
        kprintf("B starts:\n");
        sleep(1);
        int i=0;
        while(i++<30)
                kprintf("B");
	kprintf("\nB completes!!\n");	
        signal(p2);
}

void process3(int p3){
        int i=0;
	kprintf("Y starts:\n");
        while(i++<50)
                kprintf("Y");
        kprintf("\nY sleeps...\n");
        sleep(3);
        i=0;
        while(i++<50)
                kprintf("Y");
        kprintf("\nY completes\n");
}

void process4(int p4){
        int x = lock(p4,WRITE,20);
        kprintf("Z starts:\n");
	int i=0;
	while(i++<30)
		kprintf("Z");
        sleep(1);
        kprintf("\nZ completes\n");
        releaseall(1,p4);
}

/*int main(){

        int sem = screate(1);
        int semaphore1 = create(process1,2000,10,"A",1,sem);
        int semaphore2 = create(process2,2000,25,"B",1,sem);

        kprintf("Starting test for semaphore process: 'A'.\n");
        resume(semaphore1);
        sleep(1);
        kprintf("Starting test for semaphore process: 'B'.\n");
        resume(semaphore2);
        sleep(3);

        kprintf("\nTesting another approach by applying reader/writers lock.\n");

        int lock_test = lcreate();
        int rwl1 = create(process3,2000,30,"Y",1,lock_test);
        int rwl2 = create(process4,2000,40,"Z",1,lock_test);
        kprintf("\nStarting test for read-write lock: 'Y'.\n");
        resume(rwl1);
        sleep(1);
        kprintf("\nStarting test for read-write lock: 'Z'.\n");
        resume(rwl2);

        sleep(3);
        shutdown();
        return 0;
}*/

