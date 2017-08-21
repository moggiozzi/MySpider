#include "alt_timers.h"
#include "pmu.h"
#include "qspi.h"
#include "alt_qspi.h"
#include "timer.h"
#include "logger.h"
#include "timer.h"
#include "mem_map_defs.h"

#include <stdint.h>
//#include <arm_neon.h>
#include <string.h> // memcmp

//#define BUILD_EXAMPLES
#ifdef BUILD_EXAMPLES

#define TST_BUF_SIZE (1024*1024)
volatile uint32_t local_var[TST_BUF_SIZE/4] = {0};
volatile uint32_t local_var2[TST_BUF_SIZE/4] = {0};
//empty cycle 655 us
//CToC 1316 us
//CToC memcpy 2062 us
//NcToC 15460 us
//NcToC memcpy 4171 us
//CToNc 761 us
//CToNC memcpy 4951 us
//NcToNc 26953 us
//NcToNc memcpy 9600 us
//NcToR 7624 us
//RToNc 9498 us
void testRamSpeed(void){
  uint32_t t1,t2,t3;
  uint32_t res[5];
  volatile uint32_t * ptr = &local_var[0];
  volatile uint32_t * ptr2 = &local_var2[0];
  const uint32_t n = TST_BUF_SIZE/4;
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for(int i=0;i<n;i++);
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("empty cycle %d us\n",t3/200);

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for(int i=0;i<n/10;i++){
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("CToC %d us\n",t3/200);
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  memcpy((void*)ptr, (void*)ptr2, TST_BUF_SIZE);
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("CToC memcpy %d us\n",t3/200);

  ptr = &local_var[0];
  ptr2 = (uint32_t*)MEMORY_SHARED_BUF_ADDR;
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for(int i=0;i<n/10;i++){
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("NcToC %d us\n",t3/200);
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  memcpy((void*)ptr, (void*)ptr2, TST_BUF_SIZE);
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("NcToC memcpy %d us\n",t3/200);

  ptr = (uint32_t*)MEMORY_SHARED_BUF_ADDR;
  ptr2 = &local_var[0];
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for(int i=0;i<n/10;i++){
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  res[3] = t3 = get_timers_diff(t1,t2);
  log_printf("CToNc %d us\n",t3/200);
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  memcpy((void*)ptr, (void*)ptr2, TST_BUF_SIZE);
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("CToNC memcpy %d us\n",t3/200);

  ptr = (uint32_t*)MEMORY_SHARED_BUF_ADDR;
  ptr2 = (uint32_t*)(MEMORY_SHARED_BUF_ADDR+4);
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for(int i=0;i<n/10;i++){
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
    ptr[i] = ptr2[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("NcToNc %d us\n",t3/200);
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  memcpy((void*)ptr, (void*)ptr2, TST_BUF_SIZE);
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("NcToNc memcpy %d us\n",t3/200);

  ptr = (uint32_t*)(MEMORY_SHARED_BUF_ADDR);
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for(int i=0;i<n/10;i++){
#if defined(__ARMCC_VERSION)
  __asm volatile ("LDR r1, [ptr]"); // r1 = *ptr
  __asm volatile ("LDR r1, [ptr]");
  __asm volatile ("LDR r1, [ptr]");
  __asm volatile ("LDR r1, [ptr]");
  __asm volatile ("LDR r1, [ptr]");
  __asm volatile ("LDR r1, [ptr]");
  __asm volatile ("LDR r1, [ptr]");
  __asm volatile ("LDR r1, [ptr]");
  __asm volatile ("LDR r1, [ptr]");
  __asm volatile ("LDR r1, [ptr]");
#endif
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("NcToR %d us\n",t3/200);

  for(int i=0;i<n/10;i++){
#if defined(__ARMCC_VERSION)
  __asm volatile ("STR r1, [ptr]"); // *ptr = r1
  __asm volatile ("STR r1, [ptr]");
  __asm volatile ("STR r1, [ptr]");
  __asm volatile ("STR r1, [ptr]");
  __asm volatile ("STR r1, [ptr]");
  __asm volatile ("STR r1, [ptr]");
  __asm volatile ("STR r1, [ptr]");
  __asm volatile ("STR r1, [ptr]");
  __asm volatile ("STR r1, [ptr]");
  __asm volatile ("STR r1, [ptr]");
#endif
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("RToNc %d us\n",t3/200);
}

// пример работы с таймером
void test_timer() {
  uint32_t t0, t1, t2, t3;
  for (int i = 0; i < 5; i++) {
    t0 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
    // !!! в случае переполнени€ вернет 0xffffffff
    // !!! если максимум тиков таймера меньше количества в запрошенной единице измерени€, то вернет 0
    t1 = alt_gpt_curtime_nanosecs_get(ALT_GPT_CPU_PRIVATE_TMR);
    t2 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
    t3 = alt_gpt_curtime_millisecs_get(ALT_GPT_CPU_PRIVATE_TMR);
    log_printf("%uticks %uns %uus %ums\n", t0, t1, t2, t3);
  }
}

// пример получени€ тиков процессора
void test_cyclecount() {
  uint32_t c1;
  c1 = get_pmu_cyclecount();
  log_printf("Current CPU ticks: %u\n", c1);
  c1 = get_pmu_cyclecount();
  log_printf("Current CPU ticks: %u\n", c1);
}

uint32_t test_data[1024*1024/4];
void test_qspi_flash(){
  uint32_t t1,t2,i,j;
  const uint32_t flash_addr_offset = 0x800000; // 8ћб
  int32_t state = ALT_E_SUCCESS;
  char data_r[1024];

  qspi_read_regs();
// ”величение частоты дает прирост чтени€ примерно в 2 раза
  state |= qspi_set_100MHz();
//  qspi_read_regs();
//// ƒанные по умолчанию читаютс€ в QUAD режиме, на запись достаточно SINGLE
//// не дает прироста
//  state |= qspi_set_quad_mode();

  // !!! flash пам€ть перед записью д.б. очищена !!!
  t1 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
  state |= qspi_clear(flash_addr_offset, sizeof(test_data));
  t2 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
  log_printf("erase time %umks state %d\n", get_timers_diff(t1,t2), state);
  //  t1 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
  //  state |= alt_qspi_erase_chip();
  //  while(!is_qspi_ready());
  //  t2 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
  //  log_printf("bulk erase %umks state %d\n", get_timers_diff(t1,t2), state);
  // проверить что пам€ть очищена
  if (state == ALT_E_SUCCESS)
    for(i = 0; i < sizeof(test_data); i += sizeof(data_r))
    {
      state |= alt_qspi_read((void*)data_r, i + flash_addr_offset, sizeof(data_r));
      while(!qspi_is_ready());
      for(j=0;j<sizeof(data_r) && data_r[j]==0xff;j++);
      if(j!=sizeof(data_r)){
        log_printf("ERROR: data not 0xFF\n");
        break;
      }
    }
  // запись
  for(int i = 0; i < countof(test_data); i++)
    test_data[i] = i;
  t1 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
  if (state == ALT_E_SUCCESS)
  state |= alt_qspi_write(flash_addr_offset, (const void*)test_data, sizeof(test_data));
  while(!qspi_is_ready());
  t2 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
  log_printf("write time %umks state %d\n", get_timers_diff(t1,t2), state);
  // чтение
  memset(test_data, 0, sizeof(test_data));
  t1 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
  state |= alt_qspi_read((void*)test_data, flash_addr_offset, sizeof(test_data));
  while(!qspi_is_ready());
  t2 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
  for(i = 0; i < countof(test_data); i++)
    if ( test_data[i] != i)
      state = ALT_E_ERROR;
  log_printf("read time %umks state %d\n", get_timers_diff(t1,t2), state);
  if ( state == ALT_E_SUCCESS )
    log_printf("QSPI flash RW OK.\n");
  else
    log_printf("QSPI flash RW ERROR!\n");
}

// !!! ¬ ѕЋ»— должна быть залита специальна€ верси€
void test_leds(){
  volatile uint32_t val;
  *((volatile int*)0xff200004) = 3;//разрешить управление светодиодами
  *((volatile int*)0xff200000) = 0;
  delay_mks(1000000);
  while(1)
  {
    //RESET_WATCHDOG();
    val = *((volatile int*)0xff200000);
    val = (val + 1)%4;
    *((volatile int*)0xff200000) = val;
    delay_mks(200000);
  }
}

//// ѕример использовани€ расширени€ NEON
//int test_neon(){
//  uint32_t i,t1,t2;
//  uint64_t val,a,b,c;
//  uint64x1_t out_uint64x1_t = vcreate_u64(0);
//  uint64x1_t arg0_uint64x1_t = vcreate_u64(0xdeadbeef00000000);
//  uint64x1_t arg1_uint64x1_t = vcreate_u64(0x00000000deadbeef);
//  t1 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
//  for(i=0;i<10000;i++)
//    out_uint64x1_t = vadd_u64(arg0_uint64x1_t, arg1_uint64x1_t);
//  t2 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
//  log_printf("x64 neon time %umks\n", get_timers_diff(t1,t2));
//  vst1_u64(&val, out_uint64x1_t);
//  if (val != 0xdeadbeefdeadbeef)
//    return -1;
//
//  a = 0xdeadbeef00000000;
//  b = 0x00000000deadbeef;
//  t1 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
//  for(i=0;i<10000;i++)
//    c = a + b;
//  t2 = alt_gpt_curtime_microsecs_get(ALT_GPT_CPU_PRIVATE_TMR);
//  log_printf("x64 cpu time  %umks\n", get_timers_diff(t1,t2));
//
//  return 0;
//}


#define TEST_BUF_SIZE (1024*1024)
void test_memory_speed(void){
//	1MB memcpy (in 200MHz ticks)
//	1) shared memory is ALT_MMU_ATTR_WBA
//	not shared to shared 1017247
//	shared to not shared 900077
//	2) shared memory is ALT_MMU_ATTR_NC_NC
//	not shared to shared 1022167
//	shared to not shared 903456

	uint32_t test_buf[TEST_BUF_SIZE/4];
	volatile uint32_t * ptr1 = (uint32_t *)MEMORY_SHARED_BUF_ADDR;
	volatile uint32_t * ptr2 = &test_buf[0];
	uint32_t t1,t2,t3;

	t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
    memcpy((void*)ptr1,(void*)ptr2,TEST_BUF_SIZE);
	t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
	t3 = get_timers_diff(t1,t2);
	log_printf("not shared to shared %u\n",t3);

	t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
	memcpy((void*)ptr2,(void*)ptr1,TEST_BUF_SIZE);
	t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
	t3 = get_timers_diff(t1,t2);
	log_printf("shared to not shared %u\n",t3);
}

void testFunction() { volatile uint32_t x=0; x++; x*=5; }
void doException()
{
	  volatile uint32_t x;
	  typedef void func(void);
	  func* f;
	  volatile uint32_t *ptr;

//	  // division by zero not generate exception
//	  x = 0;
//	  x = 123 / x;

	  // undefined instruction (jump to alt_int_handler_undef)
	  *(volatile uint32_t*)((uint32_t)&testFunction + 4) = 0xffffffff;
	  *(volatile uint32_t*)((uint32_t)&testFunction + 8) = 0xffffffff;
	  testFunction();

	  // unaligned PC address (jump to alt_int_handler_prefetch)
	  // set PC to X address + 1
	  x = 0xffffffff;
	  f = (func*)((uint32_t)&x + 1);
	  f();

	  // undefined instruction (jump to alt_int_handler_prefetch)
	  // set PC to X address
	  x = 0xffffffff;
	  f = (func*)(uint32_t)&x;
	  f();

	  // unaligned access (jump to alt_int_handler_abort)
	  ptr = (uint32_t *)0x100001;
	  x = *ptr;

	  // unaccessible memory access (jump to alt_int_handler_abort)
	  ptr = (uint32_t *)0x100000; // этой пам€ти нет в таблице MMU
	  x = *ptr;
}

#define TEST_BUF_FP_SIZE (1000000)
void test_float_speed(void){
  uint32_t t1,t2,t3;
  float mass_a[TEST_BUF_FP_SIZE];
  float mass_b[TEST_BUF_FP_SIZE];
  float mass_r[TEST_BUF_FP_SIZE];
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_a[i] = i;
    mass_b[i] = i + 1;
  }

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_r[i] = mass_a[i]+mass_b[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("\n\rfloat add %u\n\r",t3);

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_r[i] = mass_a[i]-mass_b[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("float sub %u\n\r",t3);

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_r[i] = mass_a[i]*mass_b[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("float mul %u\n\r",t3);

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_r[i] = mass_a[i]/mass_b[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("float div %u\n\r",t3);
}

void test_double_speed(void){
  uint32_t t1,t2,t3;
  double mass_a[TEST_BUF_FP_SIZE];
  double mass_b[TEST_BUF_FP_SIZE];
  double mass_r[TEST_BUF_FP_SIZE];
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_a[i] = i;
    mass_b[i] = i + 1;
  }

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_r[i] = mass_a[i]+mass_b[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("\n\rdouble add %u\n\r",t3);

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_r[i] = mass_a[i]-mass_b[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("double sub %u\n\r",t3);

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_r[i] = mass_a[i]*mass_b[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("double mul %u\n\r",t3);

  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  for (int i = 0; i < TEST_BUF_FP_SIZE; i++)
  {
    mass_r[i] = mass_a[i]/mass_b[i];
  }
  t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  t3 = get_timers_diff(t1,t2);
  log_printf("double div %u\n\r",t3);
}

#endif
