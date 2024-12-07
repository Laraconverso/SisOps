#ifndef JOS_INC_SYSCALL_H
#define JOS_INC_SYSCALL_H

/* system call numbers */
enum {
	SYS_cputs = 0,
	SYS_cgetc,
	SYS_getenvid,
	SYS_env_destroy,
	SYS_page_alloc,
	SYS_page_map,
	SYS_page_unmap,
	SYS_exofork,
	SYS_env_set_status,
	SYS_env_set_pgfault_upcall,
	SYS_yield,
	SYS_ipc_try_send,
	SYS_ipc_recv,
	SYS_get_priority,  // Syscall to get the current process priority
	SYS_set_priority,  // Syscall to set the priority of the current process
	SYS_get_priority_id,  // Syscall to get priority of a process by ID
	NSYSCALLS             // Total number of syscalls
};

#endif /* !JOS_INC_SYSCALL_H */
