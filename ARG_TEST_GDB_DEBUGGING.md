# Pintos `args-*` 테스트 GDB 디버깅 가이드

이 문서는 `pintos/tests/userprog/args-*` 테스트를 GDB로 확인하는 방법을 정리한 것이다. 특히 argument passing 구현 후 `argc`, `argv`, `rsp`, user stack layout이 이상할 때 보는 흐름을 기준으로 한다.

## 0. 큰 그림

Pintos의 `pintos --gdb` 옵션은 QEMU를 다음 상태로 실행한다.

```text
QEMU 실행
↓
CPU를 시작하지 않고 멈춤
↓
GDB 연결 대기
↓
GDB가 localhost:1234로 attach
↓
GDB에서 continue 하면 Pintos 부팅 시작
```

즉 터미널이 2개 필요하다.

```text
터미널 1: QEMU/Pintos 실행
터미널 2: GDB 실행 후 QEMU에 attach
```

## 1. 준비

프로젝트 루트에서 Pintos 환경을 활성화한다.

```bash
cd /workspaces/pintos_22.04_lab_docker
source pintos/activate
```

현재 로컬 경로 기준으로는 아래 위치가 프로젝트 루트다.

```bash
cd /Users/j/Desktop/Jungle/week9/pintos_lab_project1-2
source pintos/activate
```

그 다음 `userprog/build`로 이동해서 빌드한다.

```bash
cd pintos/userprog/build
make
```

`args-*` 테스트 실행 파일들은 빌드 후 보통 아래에 생긴다.

```text
pintos/userprog/build/tests/userprog/args-none
pintos/userprog/build/tests/userprog/args-single
pintos/userprog/build/tests/userprog/args-multiple
pintos/userprog/build/tests/userprog/args-many
pintos/userprog/build/tests/userprog/args-dbl-space
```

## 2. 일반 테스트 먼저 실행하기

GDB에 들어가기 전에 문제가 나는 테스트를 일반 실행으로 먼저 확인한다.

```bash
cd pintos/userprog/build
make tests/userprog/args-none.result
make tests/userprog/args-single.result
make tests/userprog/args-multiple.result
make tests/userprog/args-many.result
make tests/userprog/args-dbl-space.result
```

결과 파일은 아래처럼 확인한다.

```bash
cat tests/userprog/args-multiple.output
cat tests/userprog/args-multiple.errors
cat tests/userprog/args-multiple.result
```

전체 `args-*`만 빠르게 보고 싶으면:

```bash
make tests/userprog/args-none.result \
     tests/userprog/args-single.result \
     tests/userprog/args-multiple.result \
     tests/userprog/args-many.result \
     tests/userprog/args-dbl-space.result
```

## 3. `args-multiple`을 GDB로 실행하기

`make`가 내부적으로 실행하는 명령은 대략 이런 형태다.

```bash
pintos -v -k -T 60 -m 20 --fs-disk=10 \
  -p tests/userprog/args-multiple:args-multiple \
  -- -q -f run 'args-multiple some arguments for you!'
```

GDB로 보려면 여기에 `--gdb`를 붙인다.

터미널 1에서:

```bash
cd /Users/j/Desktop/Jungle/week9/pintos_lab_project1-2/pintos/userprog/build

pintos -v -k -T 60 -m 20 --fs-disk=10 \
  -p tests/userprog/args-multiple:args-multiple \
  --gdb \
  -- -q -f run 'args-multiple some arguments for you!'
```

그러면 QEMU가 시작되지만 Pintos는 아직 실행되지 않는다. `--gdb` 때문에 QEMU가 CPU를 멈춘 채 GDB 연결을 기다린다.

터미널 2에서:

```bash
cd /Users/j/Desktop/Jungle/week9/pintos_lab_project1-2/pintos/userprog/build
gdb kernel.o
```

GDB 안에서:

```gdb
target remote localhost:1234
```

연결 후 원하는 breakpoint를 건다.

```gdb
break process_exec
break load
break setup_stack
break do_iret
continue
```

## 4. 다른 `args-*` 테스트 명령

### `args-none`

```bash
pintos -v -k -T 60 -m 20 --fs-disk=10 \
  -p tests/userprog/args-none:args-none \
  --gdb \
  -- -q -f run args-none
```

예상 출력의 핵심:

```text
argc = 1
argv[0] = 'args-none'
argv[1] = null
```

### `args-single`

```bash
pintos -v -k -T 60 -m 20 --fs-disk=10 \
  -p tests/userprog/args-single:args-single \
  --gdb \
  -- -q -f run 'args-single onearg'
```

예상 출력의 핵심:

```text
argc = 2
argv[0] = 'args-single'
argv[1] = 'onearg'
argv[2] = null
```

### `args-multiple`

```bash
pintos -v -k -T 60 -m 20 --fs-disk=10 \
  -p tests/userprog/args-multiple:args-multiple \
  --gdb \
  -- -q -f run 'args-multiple some arguments for you!'
```

예상 출력의 핵심:

```text
argc = 5
argv[0] = 'args-multiple'
argv[1] = 'some'
argv[2] = 'arguments'
argv[3] = 'for'
argv[4] = 'you!'
argv[5] = null
```

### `args-many`

```bash
pintos -v -k -T 60 -m 20 --fs-disk=10 \
  -p tests/userprog/args-many:args-many \
  --gdb \
  -- -q -f run 'args-many a b c d e f g h i j k l m n o p q r s t u v'
```

### `args-dbl-space`

```bash
pintos -v -k -T 60 -m 20 --fs-disk=10 \
  -p tests/userprog/args-dbl-space:args-dbl-space \
  --gdb \
  -- -q -f run 'args-dbl-space two  spaces!'
```

주의: `two`와 `spaces!` 사이에 공백이 2개 있다. `strtok_r(..., " ", ...)`를 쓰면 연속 공백은 보통 하나의 구분자 묶음처럼 처리되므로 결과 인자는 아래처럼 나와야 한다.

```text
argc = 3
argv[0] = 'args-dbl-space'
argv[1] = 'two'
argv[2] = 'spaces!'
argv[3] = null
```

## 5. breakpoint를 어디에 걸까

argument passing 문제는 보통 `process_exec()`, `load()`, `setup_stack()`, `do_iret()` 근처에서 확인한다.

```gdb
break process_exec
break load
break setup_stack
break do_iret
```

추천 관찰 순서:

```text
process_exec
  커널이 받은 원본 command line 확인

load
  실행 파일 이름을 제대로 분리했는지 확인

setup_stack
  user stack page가 매핑되고 rsp가 USER_STACK으로 잡히는지 확인

argument passing 코드 직후
  rdi, rsi, rsp와 stack memory 확인

do_iret
  유저 모드로 넘어가기 직전 최종 intr_frame 확인
```

`load()` 안의 argument passing 코드 직후에 정확히 멈추고 싶으면 파일/라인 breakpoint를 쓰면 된다.

```gdb
list userprog/process.c:350
break userprog/process.c:라인번호
```

예:

```gdb
break userprog/process.c:430
```

라인 번호는 코드 수정에 따라 달라지므로 `list`로 실제 위치를 먼저 확인한다.

## 6. GDB에서 꼭 확인할 값

Pintos x86-64에서 user program 시작 시 중요한 값은 `struct intr_frame` 안에 있다.

```text
if_->R.rdi = argc
if_->R.rsi = argv
if_->rsp   = user stack pointer
if_->rip   = user program entry point
```

`load()` 안에서 `if_`를 볼 때:

```gdb
print/x if_->rsp
print/d if_->R.rdi
print/x if_->R.rsi
print/x if_->rip
```

의미:

```text
if_->R.rdi : argc
if_->R.rsi : argv, 즉 &argv[0]
if_->rsp   : fake return address가 놓인 user stack 주소
```

`do_iret()`에 들어온 뒤에는 인자가 `struct intr_frame *tf` 같은 이름일 수 있다. 실제 함수 정의를 보고 이름에 맞춰 확인한다.

```gdb
print/x if_->rsp
print/d if_->R.rdi
print/x if_->R.rsi
```

## 7. user stack 메모리 확인하기

`rsp` 근처를 8바이트 단위로 본다.

```gdb
x/32gx if_->rsp
```

문자열들이 들어간 높은 주소 쪽을 문자로 보고 싶으면:

```gdb
x/s 0x주소
```

예:

```gdb
x/s 0x4747ffed
```

`argv` 배열을 확인하려면 `rsi`가 가리키는 곳을 8바이트 포인터 배열로 본다.

```gdb
x/8gx if_->R.rsi
```

그 포인터가 가리키는 문자열을 하나씩 확인한다.

```gdb
x/s *(char **)if_->R.rsi
x/s *((char **)if_->R.rsi + 1)
x/s *((char **)if_->R.rsi + 2)
```

또는 포인터 값을 먼저 보고:

```gdb
print/x *((char **)if_->R.rsi + 0)
print/x *((char **)if_->R.rsi + 1)
print/x *((char **)if_->R.rsi + 2)
```

그 주소를 `x/s`로 확인한다.

```gdb
x/s 0x포인터값
```

## 8. 올바른 stack layout 기준

예를 들어 실행 명령이:

```text
args-multiple some arguments for you!
```

라면 최종적으로는 이런 구조여야 한다.

```text
높은 주소
------------------------------------------------
"you!\0"
"for\0"
"arguments\0"
"some\0"
"args-multiple\0"
padding for alignment
NULL                    argv[argc]
&"you!"                 argv[4]
&"for"                  argv[3]
&"arguments"            argv[2]
&"some"                 argv[1]
&"args-multiple"        argv[0]
0                       fake return address
------------------------------------------------
낮은 주소
```

레지스터는 이렇게 맞아야 한다.

```text
RDI = argc
RSI = &argv[0]
RSP = fake return address 위치
```

GDB에서 확인할 때는:

```gdb
print/d if_->R.rdi
print/x if_->R.rsi
print/x if_->rsp
x/16gx if_->rsp
x/8gx if_->R.rsi
```

## 9. alignment 확인

`tests/userprog/args.c`는 아래 조건을 검사한다.

```c
if (((unsigned long long) argv & 7) != 0)
  msg ("argv and stack must be word-aligned, actually %p", argv);
```

즉 `argv`, 다시 말해 `RSI`가 8바이트 정렬되어 있어야 한다.

GDB에서:

```gdb
print/x if_->R.rsi
print/d ((unsigned long long) if_->R.rsi) & 7
```

결과가 `0`이어야 한다.

`rsp`도 fake return address를 push한 뒤 적절히 정렬되어 있어야 한다.

```gdb
print/x if_->rsp
print/d ((unsigned long long) if_->rsp) & 7
```

## 10. 자주 나는 실수

### 1. `argv[i]`에 user stack 주소가 아니라 kernel 주소를 넣음

틀린 패턴:

```c
arg_str[i] = temp;
...
memcpy (if_->rsp, &arg_str[i], sizeof (char *));
```

이러면 `argv[i]`가 user stack 안의 문자열 주소가 아니라 커널 스택/커널 페이지 안의 주소를 가리킬 수 있다. 유저 프로그램이 그 주소를 읽으면 page fault가 난다.

올바른 흐름:

```text
문자열을 user stack에 복사
복사된 user stack 주소를 별도 배열에 저장
argv[i]에는 그 user stack 주소를 push
```

### 2. `argv[argc] = NULL`을 안 넣음

`args.c`는 `i <= argc`까지 출력한다.

```c
for (i = 0; i <= argc; i++)
```

따라서 마지막 `argv[argc]`가 반드시 `NULL`이어야 한다.

### 3. `RSI`에 kernel 배열 주소를 넣음

틀린 패턴:

```c
if_->R.rsi = arg_str;
```

`arg_str`는 커널 함수의 지역 배열이다. 유저 프로그램 입장에서는 접근할 수 없는 주소다.

올바른 값:

```text
if_->R.rsi = user stack 안에 만든 argv[0]의 주소
```

즉 `argv` 포인터 배열을 user stack에 push한 뒤, 그 시작 주소를 `RSI`에 넣어야 한다.

### 4. 문자열 포인터 저장 시 `&temp`를 넣음

틀린 패턴:

```c
arg_str[count] = &temp;
```

`temp`는 `char *` 변수다. `&temp`는 `char **`이고, 지역 변수 자체의 주소다.

대부분 원하는 것은:

```c
arg_str[count] = temp;
```

또는 더 정확히는 user stack에 복사한 뒤:

```c
arg_addr[count] = if_->rsp;
```

처럼 복사된 문자열의 user 주소를 저장하는 것이다.

### 5. `memcpy(if_->rsp, 0, size)` 사용

틀린 패턴:

```c
memcpy (if_->rsp, 0, sizeof (uint8_t));
```

`memcpy`의 두 번째 인자는 복사할 source 주소다. `0`을 넘기면 NULL 주소에서 읽으려 하므로 잘못이다.

0으로 채우려면:

```c
memset (if_->rsp, 0, size);
```

포인터 NULL을 push하려면:

```c
char *null = NULL;
if_->rsp -= sizeof null;
memcpy (if_->rsp, &null, sizeof null);
```

### 6. command line을 파싱하다가 원본 문자열을 너무 일찍 망가뜨림

`strtok_r`는 문자열 안의 공백을 `'\0'`로 바꾼다. 따라서 실행 파일 이름을 얻으려고 원본 `file_name`에 바로 `strtok_r`를 쓰면, 나중에 전체 인자를 다시 세거나 복사할 때 이미 잘린 문자열을 보고 있을 수 있다.

안전한 방식:

```text
원본 command line을 별도 페이지/버퍼에 복사
파일 이름 파싱용 복사본 하나
인자 전체 파싱용 복사본 하나
```

## 11. 추천 GDB 세션 예시

터미널 1:

```bash
cd /Users/j/Desktop/Jungle/week9/pintos_lab_project1-2/pintos/userprog/build

pintos -v -k -T 60 -m 20 --fs-disk=10 \
  -p tests/userprog/args-multiple:args-multiple \
  --gdb \
  -- -q -f run 'args-multiple some arguments for you!'
```

터미널 2:

```bash
cd /Users/j/Desktop/Jungle/week9/pintos_lab_project1-2/pintos/userprog/build
gdb kernel.o
```

GDB:

```gdb
target remote localhost:1234
break load
break setup_stack
break do_iret
continue
```

`load()`에서 멈추면:

```gdb
next
next
print file_name
x/s file_name
```

argument passing 코드 직후로 이동한 뒤:

```gdb
print/d if_->R.rdi
print/x if_->R.rsi
print/x if_->rsp
x/32gx if_->rsp
x/8gx if_->R.rsi
```

각 argv 문자열 확인:

```gdb
x/s *((char **)if_->R.rsi + 0)
x/s *((char **)if_->R.rsi + 1)
x/s *((char **)if_->R.rsi + 2)
x/s *((char **)if_->R.rsi + 3)
x/s *((char **)if_->R.rsi + 4)
print/x *((char **)if_->R.rsi + 5)
```

마지막 값은 `0x0`이어야 한다.

```text
argv[argc] == NULL
```

## 12. QEMU가 멈춘 것처럼 보일 때

`--gdb`를 붙이면 QEMU는 일부러 멈춘다. 정상이다.

터미널 2에서 GDB를 붙이고:

```gdb
target remote localhost:1234
continue
```

를 해야 Pintos가 부팅된다.

## 13. 1234 포트가 이미 사용 중일 때

이전 QEMU가 살아 있으면 새 QEMU가 GDB 포트를 못 열 수 있다.

확인:

```bash
ps -ax -o pid,command | grep qemu
```

필요하면 해당 QEMU를 종료한다.

```bash
kill -TERM <PID>
```

그래도 안 죽으면 마지막 수단:

```bash
kill -KILL <PID>
```

## 14. 디버깅 후 결과 확인

GDB 없이 다시 일반 테스트를 돌린다.

```bash
cd /Users/j/Desktop/Jungle/week9/pintos_lab_project1-2/pintos/userprog/build
make tests/userprog/args-multiple.result
cat tests/userprog/args-multiple.output
cat tests/userprog/args-multiple.result
```

`PASS`가 나오면 해당 테스트는 통과다.

```text
PASS
```

전체 userprog 테스트를 돌리려면:

```bash
make check
```

## 15. 빠른 체크리스트

argument passing 코드가 맞는지 볼 때는 아래만 먼저 확인한다.

```text
[ ] 실행 파일 이름만 filesys_open()에 들어가는가?
[ ] argc가 정확한가?
[ ] RDI에 argc를 넣었는가?
[ ] argv 포인터 배열을 user stack에 만들었는가?
[ ] RSI가 kernel 주소가 아니라 user stack의 argv[0] 주소인가?
[ ] 각 argv[i]가 user stack 안의 문자열 주소를 가리키는가?
[ ] argv[argc]가 NULL인가?
[ ] fake return address 0을 push했는가?
[ ] rsp가 fake return address 위치인가?
[ ] argv 주소가 8바이트 정렬되어 있는가?
```

