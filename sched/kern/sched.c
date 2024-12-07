#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define BOOST_CYCLES 100

void sched_halt(void);
void sched_rr(void);
void priority_sched(void);

// Choose a user environment to run and run it.
struct stadistics {
	uint32_t amount_of_sc_calls;
	uint32_t runs_total;
	uint32_t sched_times_boosted;
};

struct stadistics stats = { 0, 0, 0 };

void
sched_yield(void)
{
	stats.amount_of_sc_calls++;

#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	sched_rr();
#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
	priority_sched();
#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv) {
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}


void
sched_rr()
{
	// Inicializo start según el entorno actual. Si hay un proceso en ejecución
	// con ENVX recibo el indice del actual y le sumo 1 para buscar el proximo sino en 0
	int start = curenv ? ENVX(curenv->env_id) + 1 : 0;
	// corro el for NENV veces ya que es el numero total de entornos disponibles
	for (int i = 0; i < NENV; i++) {
		// calculo el indice revisando todos los entornos a partir de
		// start y si llega al final vuelve al principio (cirucular)
		int index = (start + i) % NENV;
		if (envs[index].env_status == ENV_RUNNABLE) {
			// aca termina el for porque el env_run toma el control del entorno
			stats.runs_total++;
			env_run(&envs[index]);
		}
	}
	// si no se encuentra ningun proceso con la prioridad pasada
	// miro el status actual. Si es running, continuo ejecutando
	if (curenv && curenv->env_status == ENV_RUNNING) {
		stats.runs_total++;
		env_run(curenv);
	}
	// Si no encuentro nada, detengo la CPU.
	sched_halt();
}

void
priority_sched(void)
{
	struct Env *idle = curenv;
	struct Env *highest_priority_env = NULL;

	static int boost_counter = 0;

	if (++boost_counter >= BOOST_CYCLES) {
		for (int i = 0; i < NENV; i++) {
			if (envs[i].env_status == ENV_RUNNABLE &&
			    envs[i].priority < DEFAULT_PRIORITY) {
				envs[i].priority =
				        DEFAULT_PRIORITY;  // Boost priority to the maximum
				envs[i].env_times_boosted++;
				stats.sched_times_boosted++;
			}
		}
		boost_counter = 0;  // Reset counter
	}

	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			if (!highest_priority_env ||
			    envs[i].priority > highest_priority_env->priority) {
				highest_priority_env = &envs[i];
			}
		}
	}

	if (highest_priority_env) {
		if (highest_priority_env->priority > 1) {
			highest_priority_env->priority--;
		}
		stats.runs_total++;
		env_run(highest_priority_env);
	} else {
		if (idle && idle->env_status == ENV_RUNNING) {
			env_run(idle);
		} else {
			sched_halt();
		}
	}
}


// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
print_statistics()
{
	cprintf("\n=> Scheduler statistics: \n");

	cprintf("* Times schedule was called: %d\n", stats.runs_total);
	cprintf("* Times schedule has boosted priorities: %d\n",
	        stats.sched_times_boosted);

	cprintf("\n=> Processes statistics: \n");

	cprintf("Env Id \t | runs \t | boosts \t "
	        "\n");
	cprintf("———————————————————————————————————————"
	        "\n");

	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_runs != 0) {
			cprintf("%d \t | %d \t | %d \t \n",
			        envs[i].env_id,
			        envs[i].env_runs,
			        envs[i].env_times_boosted);
		};
	}
}

void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");

		print_statistics();

		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
