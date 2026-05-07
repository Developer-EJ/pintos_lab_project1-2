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
	switch (f->R.rax)
	{
	case SYS_WRITE:
		// 반환값이 있는 시스템콜은 다시 rax에 저장
		// 일단 write가 구현중이므로 터미널 출력으로 고정
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
	default:
		thread_exit();
	}
}

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

//은재님 create
// bool create(const char *file, unsigned initial_size)
// {
// 	if (file == NULL || pml4_get_page(thread_current()->pml4, file) == NULL)
// 		exit(-1);
// 	return filesys_create(file, initial_size);
// }

bool remove(const char *file)
{
	return filesys_remove(file);
}

int open(const char *file)
{
	if ( file == NULL) exit(-1);
	
	int fd = -1;
	struct thread *curr = thread_current();

	//유효한지는 아직 검사 안함, NULL인지만 체크
	if (filesys_open(file) != NULL) {
		/*파일을 오픈 한 다음에 thread_current()의 list에 넣어주어야 함
			0. fd_list로 fd 계산
			1. fd_elem 만들기	
			2. fd_list에 넣기
			3. fd_return
		*/
		if (list_empty(&curr->fd_list)) 
			fd = 2;
		else
			fd = (int)list_size(&curr->fd_list) + 2;

		struct fd_elem *fe = malloc(sizeof(struct fd_elem));
		fe->fd = fd;
		fe->file = file;
		list_push_back(&curr->fd_list, &fe->elem);
	}

	return fd;
}

bool create (const char *file, unsigned initial_size) {
	//초기 크기가 바이트인 새 파일 생성.
	bool result = false;

	if (file == NULL || initial_size < 0) {
		exit(-1);
	}
	
	result = filesys_create(file, initial_size);
	return result;
}

bool if_user_vaddr(const char *file) {
	if (file == NULL) exit(-1);
	if (file == NULL) exit(-1);
	if (file == NULL) exit(-1);
	return true;
}