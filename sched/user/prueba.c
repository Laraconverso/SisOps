
#include <inc/lib.h>

void
spawn_process(const char *name, int priority)
{
	if (fork() == 0) {  // proceso hijo
		sys_set_priority(
		        0, priority);  // Setea la prioridad de los procesos hijos
		int i = 0;
		while (i < 10) {  // 10 iteraciones
			cprintf("Proceso %s con prioridad %d ejecutándose: "
			        "iteración %d\n",
			        name,
			        priority,
			        i++);
			sys_yield();
		}
		exit();  // termina el proceso despues de 10 iteraciones
	}
}


void
umain(int argc, char **argv)
{
	// Creo procesos con distintas prioridades
	spawn_process("*** Alta prioridad", 10);
	spawn_process("** Media prioridad", 50);
	spawn_process("* Baja prioridad", 100);

	while (1) {
		sys_yield();
	}
}
