/*
 * acx.h
 *
 * Created: 3/20/2014 11:08:37 AM
 *  Author: E. Frank Barry
 *
 */


#ifndef ACX_H_
#define ACX_H_

// C and Assembly definitions
// includes max number of threads and number used
#define MAX_THREADS 8	
#define NUM_THREADS 8

// define stack sizes for each thread
#define STACK_SIZE0 128
#define STACK_SIZE1 128
#define STACK_SIZE2 128
#define STACK_SIZE3 128
#define STACK_SIZE4 128
#define STACK_SIZE5 128
#define STACK_SIZE6 128
#define STACK_SIZE7 128

// define total stack memory size using thread stack sizes
#define MEM_SIZE (STACK_SIZE0 + STACK_SIZE1 + STACK_SIZE2 + STACK_SIZE3 + STACK_SIZE4 + STACK_SIZE5 + STACK_SIZE6 + STACK_SIZE7)

// define offsets to each thread's stack base
#define STACK_OFFSET0 0
#define STACK_OFFSET1 (STACK_OFFSET0 + STACK_SIZE1)
#define STACK_OFFSET2 (STACK_OFFSET1 + STACK_SIZE2)
#define STACK_OFFSET3 (STACK_OFFSET2 + STACK_SIZE3)
#define STACK_OFFSET4 (STACK_OFFSET3 + STACK_SIZE4)
#define STACK_OFFSET5 (STACK_OFFSET4 + STACK_SIZE5)
#define STACK_OFFSET6 (STACK_OFFSET5 + STACK_SIZE6)
#define STACK_OFFSET7 (STACK_OFFSET6 + STACK_SIZE7)

// define thread IDs for each thread
#define STACK_ID0 0
#define STACK_ID1 1
#define STACK_ID2 2
#define STACK_ID3 3
#define STACK_ID4 4
#define STACK_ID5 5
#define STACK_ID6 6
#define STACK_ID7 7

// define thread context size 
#define THREAD_CONTEXT 21


#ifndef __ASSEMBLER__    // The following only apply to C files...


// macro to access the current thread id
#define x_getTID()	(x_thread_id)   //x_thread_id is a global declared in acx.c

//---------------------------------------------------------------------------
// PTHREAD is a type that represents how threads are called--
// It is just a pointer to a function returning void
// that is a passed an int and a char * as parameters.
//---------------------------------------------------------------------------
typedef void(*PTHREAD)(int,char*);

//---------------------------------------------------------------------------
// This union is used to provide access to individual bytes of a thread address
//---------------------------------------------------------------------------
typedef union{
	PTHREAD thread;
	uint8_t addr[3];
}PTU;

//---------------------------------------------------------------------------
// This type is used for entries in the stack control table
//---------------------------------------------------------------------------
typedef struct {
	uint8_t *sp;
	uint8_t *pStackBase;
} STACKCONTROL;

typedef uint8_t	byte;

//----------------------------------------------------------------------------
// ACX Function prototypes
//----------------------------------------------------------------------------
void x_init(void);
void x_delay(unsigned int);
unsigned long x_gtime();
void x_schedule(void);
void x_new(uint8_t, PTHREAD, bool);
void x_yield(void);
uint8_t bit2mask8(uint8_t);
void x_suspend(uint8_t);
void x_resume(uint8_t);
void x_disable(uint8_t);
void x_enable(uint8_t);


//----------------------------------------------------------------------------
// Helper Function prototypes
//----------------------------------------------------------------------------
void PSerial_open(unsigned char, long, int);
void PSerial_write(unsigned char, char);
void changeStack(uint8_t*, uint8_t*, int);

#endif


#endif /* ACX_H_ */
