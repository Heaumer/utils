/*
 * started from https://github.com/nelhage/ministrace/blob/for-blog/ministrace.c
 * cc fake.c -o fake -Wall;
 * see id.c for a sample target.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

#if 0
void (*f)(pid_t pid) handlesyscalls[1024] = {
	[SYS_getuid] = xgetuid,
}
#endif

/*
 * RAX for x86 64
 * EAX for x86
 * more about linu syscall:
 * http://docs.cs.up.ac.za/programming/asm/derick_tut/syscalls.html
 * http://callumscode.com/blog/3
 */
int
waitsyscall(pid_t pid)
{
	int status;

	for (status = 0;;) {
		ptrace(PTRACE_SYSCALL, pid, 0, 0);
		wait(&status);
		if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80)
			return 1;
		if (WIFEXITED(status))
			return 0;
	}

	return 42;
}

int
fakeit(char *argv[])
{
	struct user_regs_struct uregs;
	long syscall;
	pid_t pid;

	pid = fork();

	switch(pid) {
	case -1:
		perror("fork");
		return 1;
	case 0:
		ptrace(PTRACE_TRACEME);
		kill(getpid(), SIGSTOP);
		execvp(argv[0], argv);
		perror("execvp");
		break;
	default:
		wait(NULL);
		/*
		 * from ptrace(2): PTRACE_O_TRACESYSGOOD set bit 7 in
		 * signal number (0x80) when delivering a syscall.
		 */
		ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD);
		for (;;) {
			if (!waitsyscall(pid))
				break;

			syscall = ptrace(PTRACE_PEEKUSER, pid, sizeof(long)*ORIG_RAX);
			if (syscall == SYS_getuid) {
				if (!waitsyscall(pid))
					break;
				ptrace(PTRACE_GETREGS, pid, NULL, &uregs);
				uregs.rax = 0;
				ptrace(PTRACE_SETREGS, pid, NULL, &uregs);
			}
		}
		break;
	}

	return 0;
}

void
help(char *argv0)
{
	fprintf(stderr, "%s <cmd>\n", argv0);
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		help(argv[0]);
		return 1;
	}
	return fakeit(argv+1);
}
