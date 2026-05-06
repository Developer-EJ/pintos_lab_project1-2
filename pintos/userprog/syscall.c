#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"

void syscall_entry(void);
void syscall_handler(struct intr_frame *);


// 시스템 콜 메서드 선언
int write(int fd, const void *buffer, unsigned size);
void exit(int status);
void halt(void);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081			/* Segment selector msr */
#define MSR_LSTAR 0xc0000082		/* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void syscall_init(void)
{
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48 |
							((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t)syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			  FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
void syscall_handler(struct intr_frame *f UNUSED)
{
	// TODO: Your implementation goes here.
	/* TODO: Validate every user pointer argument before dereferencing it
	 * inside write/create/remove/open and future file syscalls.
	 */
	switch (f->R.rax)
	{
	case SYS_WRITE:
		f->R.rax = write(1, (void *)f->R.rsi, f->R.rdx);
		break;
	case SYS_EXIT:
		exit(f->R.rdi);
		break;
	case SYS_HALT:
		halt();
		break;
	case SYS_CREATE:
		f->R.rax = create(f->R.rdi, f->R.rsi);
		break;
	case SYS_REMOVE:
		f->R.rax = remove(f->R.rdi);
		break;
	case SYS_OPEN:
		f->R.rax = open(f->R.rdi);
		break;
	case SYS_FILESIZE:
	case SYS_READ:


	case SYS_SEEK:
	case SYS_TELL:
	
	case SYS_CLOSE:
		break;
	
	default:
		thread_exit();
	}
}
/* TODO (Goal 1: 4/5/6/7)
 * 4) Make SYS_WRITE(fd == 1) reliable so test output appears first.
 * 5) Add reusable user-pointer validation helpers and exit(-1) on bad access.
 * 6) Keep SYS_HALT simple and use it to verify syscall dispatch end-to-end.
 * 7) Add SYS_CREATE / SYS_REMOVE / SYS_OPEN with filename validation first.
 */

int write(int fd, const void *buffer, unsigned size)
{
	if (fd == 1)
	{
		putbuf(buffer, size);
		return size;
	}

	// 일단 argu pass 구현하고 다음에..
	// // fd를 통해 파일 객체 가져오기
	// struct file *file =
	// 	if (file == NULL) return -1;

	// return file_write(file, buffer, size);
	
}




//Terminates Pintos by calling power_off() (declared in src/include/threads/init.h). This should be seldom used, because you lose some information about possible deadlock situations, etc.
void halt(void)
{
	power_off();
}

//Terminates the current user program, returning status to the kernel. If the process's parent waits for it (see below), this is the status that will be returned. Conventionally, a status of 0 indicates success and nonzero values indicate errors.
void exit(int status)
{
	struct thread *cur = thread_current();
	cur->exit_status = status;
	printf("%s: exit(%d)\n", cur->name, status);
	thread_exit();
}

/*Creates a new file called file initially initial_size bytes in size. Returns true if successful, false otherwise. Creating a new file does not open it: opening the new file is a separate operation which would require a open system call.*/
bool create(const char *file, unsigned initial_size)
{
	if (file == NULL)
		return false;
	else{
		filesys_create(file, initial_size);
		return true;
	}
		
	
	
	
}

/*Deletes the file called file. Returns true if successful, false otherwise. A file may be removed regardless of whether it is open or closed, and removing an open file does not close it.*/
/*You should implement the standard Unix semantics for files. 
That is, when a file is removed any process which has a file descriptor for that file may continue to use that descriptor. 
This means that they can read and write from the file. 
The file will not have a name, and no other processes will be able to open it, but it will continue to exist until all file descriptors referring to the file are closed or the machine shuts down*/
bool remove(const char *file)
{

}
/*Opens the file called file. Returns a nonnegative integer handle called a "file descriptor" (fd), or -1 if the file could not be opened. 
File descriptors numbered 0 and 1 are reserved for the console: fd 0 (STDIN_FILENO) is standard input, fd 1 (STDOUT_FILENO) is standard output. 
The open system call will never return either of these file descriptors, which are valid as system call arguments only as explicitly described below. 
Each process has an independent set of file descriptors. File descriptors are inherited by child processes. When a single file is opened more than once, whether by a single process or different processes, each open returns a new file descriptor. 
Different file descriptors for a single file are closed independently in separate calls to close and they do not share a file position. You should follow the linux scheme, which returns integer starting from zero, to do the extra.*/
int open(const char *file)
{
	
}


//다음번에

/*Change current process to the executable whose name is given in cmd_line, passing any given arguments. 
This never returns if successful. Otherwise the process terminates with exit state -1, if the program cannot load or run for any reason. 
This function does not change the name of the thread that called exec. Please note that file descriptors remain open across an exec call.*/
int exec (const char *cmd_line);

/*Create new process which is the clone of current process with the name THREAD_NAME. You don't need to clone the value of the registers except %RBX, %RSP, %RBP, and %R12 - %R15, which are callee-saved registers. 
Must return pid of the child process, otherwise shouldn't be a valid pid. In child process, the return value should be 0. The child should have DUPLICATED resources including file descriptor and virtual memory space. 
Parent process should never return from the fork until it knows whether the child process successfully cloned. That is, if the child process fail to duplicate the resource, the fork () call of parent should return the TID_ERROR.
The template utilizes the pml4_for_each() in threads/mmu.c to copy entire user memory space, including corresponding pagetable structures, but you need to fill missing parts of passed pte_for_each_func
*/
pid_t fork (const char *thread_name);

