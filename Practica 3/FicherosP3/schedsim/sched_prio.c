#include "sched.h"


int prio_quantum=1;


struct prio_data {
     int remaining_ticks_slice;
};

static int task_new_prio(task_t* t) {
	 struct prio_data* cs_data=malloc(sizeof(struct prio_data));

	    if (!cs_data)
	        return 1;  

	    
	    cs_data->remaining_ticks_slice=prio_quantum;
	    t->tcs_data=cs_data;
	    return 0;
}

static void task_free_prio(task_t* t){
    if (t->tcs_data){
        free(t->tcs_data);
        t->tcs_data=NULL;
    }
}

static task_t* pick_next_task_prio(runqueue_t* rq,int cpu) {
    task_t* t=head_slist(&rq->tasks); 

    if (t) {
    
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
        rq->cur_task=t;
    }

    return t;
}

static int compare_tasks_cpu_prio(void *t1,void *t2) {
	task_t* tsk1=(task_t*)t1;
	task_t* tsk2=(task_t*)t2;
	return tsk1->prio-tsk2->prio;
}

static void enqueue_task_prio(task_t* t,int cpu, int runnable) {
    runqueue_t* rq=get_runqueue_cpu(cpu);

    struct prio_data* cs_data=(struct prio_data*) t->tcs_data;

    if (t->on_rq || is_idle_task(t))
        return;

    if (t->flags & TF_INSERT_FRONT){
      
        t->flags&=~TF_INSERT_FRONT;
        sorted_insert_slist_front(&rq->tasks, t, 1, compare_tasks_cpu_prio);
    }
    else
        sorted_insert_slist(&rq->tasks, t, 1, compare_tasks_cpu_prio); 

    t->on_rq=TRUE;
    cs_data->remaining_ticks_slice=prio_quantum; 


   
    if (!runnable){
        task_t* current=rq->cur_task;
     
        t->last_cpu=cpu;

       
        if (preemptive_scheduler && !is_idle_task(current) && t->runnable_ticks_left<current->runnable_ticks_left) {
            rq->need_resched=TRUE;
            current->flags|=TF_INSERT_FRONT; 
        }
    }
}


static void task_tick_prio(runqueue_t* rq,int cpu){

    task_t* current=rq->cur_task;

    struct prio_data* cs_data=(struct prio_data*) current->tcs_data;

    if (is_idle_task(current))
        return;

    cs_data->remaining_ticks_slice--; 

    if (cs_data->remaining_ticks_slice<=0)
           rq->need_resched=TRUE; 


}

static task_t* steal_task_prio(runqueue_t* rq,int cpu){
    task_t* t=tail_slist(&rq->tasks);

    if (t) {
        remove_slist(&rq->tasks,t);
        t->on_rq=FALSE;
    }
    return t;
}

sched_class_t prio_sched={
	.task_new=task_new_prio,
	.task_free=task_free_prio,
    .pick_next_task=pick_next_task_prio,
    .enqueue_task=enqueue_task_prio,
    .task_tick=task_tick_prio,
    .steal_task=steal_task_prio
};
