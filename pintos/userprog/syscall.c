#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include "filesys/filesys.h" // filesys_open, filesys_create, filesys_remove
#include "filesys/file.h"	 // file_close, file_read, file_length
#include "devices/input.h"	 // input_getc
#include "threads/init.h"	 // power_off
#include "threads/vaddr.h"	 // is_user_vaddr

void syscall_entry(void);
void syscall_handler(struct intr_frame *);

// 시스템 콜 메서드 선언
int write(int fd, const void *buffer, unsigned size);
void exit(int status);
void halt(void);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int read(int fd, void *buffer, unsigned size);
void close(int fd);
int filesize(int fd);

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
static void check_address(const void *addr)
{
	if (addr == NULL || !is_user_vaddr(addr) || pml4_get_page(thread_current()->pml4, addr) == NULL)
		exit(-1);
}

void syscall_handler(struct intr_frame *f UNUSED)
{
	// TODO: Your implementation goes here.
	switch (f->R.rax)
	{
	case SYS_WRITE:
		f->R.rax = write(f->R.rdi, (void *)f->R.rsi, f->R.rdx);
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
	case SYS_READ:
		f->R.rax = read(f->R.rdi, (void *)f->R.rsi, f->R.rdx);
		break;
	case SYS_CLOSE:
		close(f->R.rdi);
		break;
	case SYS_FILESIZE:
		f->R.rax = filesize(f->R.rdi);
		break;
	default:
		thread_exit();
	}
}

int write(int fd, const void *buffer, unsigned size)
{
	check_address(buffer);

	if (fd == 1)
	{
		putbuf(buffer, size);
		return size;
	}

	if (fd < 2 || fd >= 128)
		return -1;

	struct file *file = thread_current()->fd_table[fd];
	if (file == NULL)
		return -1;

	return file_write(file, buffer, size);
}

void exit(int status)
{
	// 종료 코드 저장
	thread_current()->exit_code = status;
	thread_exit();
}

void halt(void)
{
	power_off();
}

bool create(const char *file, unsigned initial_size)
{
	if (file == NULL || pml4_get_page(thread_current()->pml4, file) == NULL)
		return -1;
	return filesys_create(file, initial_size);
}

bool remove(const char *file)
{
	return filesys_remove(file);
}

int open(const char *file)
{
	if (file == NULL || pml4_get_page(thread_current()->pml4, file) == NULL || is_user_vaddr(file) == false)
		exit(-1);
	struct file *opened_file = filesys_open(file);
	if (opened_file == NULL)
		return -1;

	struct file **cur_thread_fd_table = thread_current()->fd_table;
	// 테이블을 순회하면서 넣을 수 있는지 검사
	for (int fd = 2; fd < 128; fd++)
	{
		if (cur_thread_fd_table[fd] == NULL)
		{
			cur_thread_fd_table[fd] = opened_file;
			// 삽입시 즉시 fd를 return하여 함수 종료
			return fd;
		}
	}

	// 만약 fd 테이블에 넣을 수 없으면 close
	file_close(opened_file);
	exit(-1);
}

int read(int fd, void *buffer, unsigned size)
{
	check_address(buffer);

	if (fd == 0)
	{
		for (unsigned i = 0; i < size; i++)
			((uint8_t *)buffer)[i] = input_getc();
		return size;
	}

	if (fd < 2 || fd >= 128)
		return -1;

	struct file *file = thread_current()->fd_table[fd];
	if (file == NULL)
		return -1;

	return file_read(file, buffer, size);
}

void close(int fd)
{
	if (fd < 2 || fd >= 128)
		exit(-1);
	struct file *file = thread_current()->fd_table[fd];
	if (file == NULL)
		exit(-1);
	file_close(file);
	thread_current()->fd_table[fd] = NULL;
}

int filesize(int fd)
{
	struct file *file = thread_current()->fd_table[fd];
	if (file == NULL)
		exit(-1);
	return file_length(file);
}