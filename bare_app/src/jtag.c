#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ff.h"
#include "logger.h"
#include <socal/socal.h>
#include <socal/hps.h>
#include <socal/alt_sysmgr.h>
#include <socal/alt_scanmgr.h>
#include <soc_cv_av/socal/hps.h>
#include <soc_cv_av/socal/alt_scanmgr.h>
#include <soc_cv_av/socal/alt_sysmgr.h>
#include <hwlib.h>

// TODO 1) Не реализована проверка выходных из JTAG'a данных (TDO)
// TODO 2) Найти и устранить узкое место: скорость программирования около 60 сек., хотя с компа заливается около 20 сек.

#define MAX_STRING_LENGTH (4*1024)

int ftStatus;
unsigned char byOutputBuffer[1024]; // Buffer to hold MPSSE commands and data to be sent to the FT2232H
unsigned char byInputBuffer[1024]; // Buffer to hold data read from the FT2232H
uint32_t dwNumBytesToSend = 0; // Index to the output buffer
uint32_t dwNumBytesSent = 0; // Count of actual bytes sent - used with FT_Write
uint32_t dwNumBytesToRead = 0; // Number of bytes available to read in the driver's input buffer
uint32_t dwNumBytesRead = 0; // Count of actual bytes read - used with FT_Read

//define buffer accumulating commands to be sent
#define WR_BUF_SIZE 512
unsigned char wr_buffer[WR_BUF_SIZE * 2];
int wr_buf_ptr = 0;

//global pointers to sdr tdi, tdo, mask and smask arrays
//we expect that dr data can be very long, but we do not know exact size
//size of arrays allocated will be defined during SVF strings parcing
unsigned char* psdr_tdi_data = NULL;
unsigned char* psdr_tdo_data = NULL;
unsigned char* psdr_mask_data = NULL;
unsigned char* psdr_smask_data = NULL;
unsigned int sdr_data_size = 0; //current size of arrays
unsigned int sdr_tdi_sz;
unsigned int sdr_tdo_sz;
unsigned int sdr_mask_sz;
unsigned int sdr_smask_sz;

char rbuffer[MAX_STRING_LENGTH];
unsigned char rbytes[MAX_STRING_LENGTH];
char command[MAX_STRING_LENGTH];
int line;

//possible JTAG TAP states
#define TAP_STATE_TLR 0
#define TAP_STATE_RTI 1
#define TAP_STATE_SELECT_DR_SCAN 2
#define TAP_STATE_SELECT_IR_SCAN 3
#define TAP_STATE_CAPTURE_DR 4
#define TAP_STATE_CAPTURE_IR 5
#define TAP_STATE_SHIFT_DR 6
#define TAP_STATE_SHIFT_IR 7
#define TAP_STATE_EXIT_1_DR 8
#define TAP_STATE_EXIT_1_IR 9
#define TAP_STATE_PAUSE_DR 10
#define TAP_STATE_PAUSE_IR 11
#define TAP_STATE_EXIT_2_DR 12
#define TAP_STATE_EXIT_2_IR 13
#define TAP_STATE_UPDATE_DR 14
#define TAP_STATE_UPDATE_IR 15

int g_def_end_state_dr = TAP_STATE_RTI;
int g_def_end_state_ir = TAP_STATE_RTI;
int g_least_runtest_state = TAP_STATE_RTI;
int g_tap_state = TAP_STATE_RTI;

//buffers where we store tdo expected answer to be compared with tdo real answer
#define EXP_BUF_LEN 1024
#define EXP_BUF_THRESH (EXP_BUF_LEN/8)
unsigned char expect_buffer[EXP_BUF_LEN + 16];
unsigned char expect_buffer_mask[EXP_BUF_LEN + 16];
unsigned char compare_buffer[EXP_BUF_LEN + 16];
int exp_buff_ptr = 0;

#define BLOCK_SZ 16 // scan_mgr позволяет отправить по JTAG до 128 битов за раз (16 байтов)

FIL svfFile;

char     cache_buf[MAX_STRING_LENGTH];
int      cache_buf_idx  = MAX_STRING_LENGTH;
uint32_t cache_size = 0;
char *fgets_(char * rbuffer, int n) {
  int i = 0;
  while (true) {
    if (cache_buf_idx < cache_size)
    {
      rbuffer[i] = cache_buf[cache_buf_idx];
      cache_buf_idx++;
      if (rbuffer[i] == '\n') {
        rbuffer[i] = '\0';
        if (i > 0 && rbuffer[i - 1] == '\r')
          rbuffer[i - 1] = '\0';
        break;
      }
      i++;
    } else {
      f_read(&svfFile, cache_buf, sizeof(cache_buf), &cache_size);
      if (cache_size == 0)
        return 0;
      cache_buf_idx = 0;
    }
  }
  //log_printf("%s\n",rbuffer);
  return rbuffer;
}
char *fgets_simple(char * rbuffer, int n, FIL * fil){
  int i=0;
  uint32_t br;
  while(true){
    f_read(fil, &rbuffer[i], 1, &br);
    if(br==0)
      return 0;
    if(rbuffer[i]=='\n'){
        rbuffer[i] = '\0';
        if(i>0 && rbuffer[i-1]=='\r')
          rbuffer[i-1] = '\0';
        break;
    }
    i++;
  }
  return rbuffer;
}

static void init_vars(){
  dwNumBytesToSend = 0; // Index to the output buffer
  dwNumBytesSent = 0; // Count of actual bytes sent - used with FT_Write
  dwNumBytesToRead = 0; // Number of bytes available to read in the driver's input buffer
  dwNumBytesRead = 0; // Count of actual bytes read - used with FT_Read

  wr_buf_ptr = 0;

  sdr_data_size = 0; //current size of arrays
  sdr_tdi_sz=0;
  sdr_tdo_sz=0;
  sdr_mask_sz=0;
  sdr_smask_sz=0;

  g_def_end_state_dr = TAP_STATE_RTI;
  g_def_end_state_ir = TAP_STATE_RTI;
  g_least_runtest_state = TAP_STATE_RTI;
  g_tap_state = TAP_STATE_RTI;

  exp_buff_ptr = 0;
  cache_size = 0;
}

void Sleep(int ms)
{
  //todo
  //usleep(ms*1000);
  for (volatile int i = 0; i < 1000000 * ms; i++)
    ;
}

int FT_Write(
    unsigned char * lpBuffer,
    uint32_t dwBytesToWrite,
    uint32_t* lpBytesWritten
    )
{
  char * buf = (char*) lpBuffer;
  int i=0;
  while( i < dwBytesToWrite)
  {
    int cnt = 1000000;
    uint32_t stat = *(volatile uint32_t*) 0xfff02000; // scanmgr
    uint32_t wfifocnt = (stat >> 28) & 0x7;
    uint32_t rfifocnt = (stat >> 24) & 0x7;
    uint32_t rval = 0;
    while (wfifocnt != 0 && cnt > 0) {
      if (rfifocnt > 0) { //входную очередь надо очищать, иначе не сможем писать.
        rval = alt_read_word(0xfff02010);
        if(dwNumBytesRead < sizeof(byInputBuffer))
          byInputBuffer[dwNumBytesRead] = rval;
        //log_printf("r%X ", rval);
        dwNumBytesRead++;
      }
      stat = *(volatile uint32_t*) 0xfff02000; // scanmgr
      wfifocnt = (stat >> 28) & 0x7;
      rfifocnt = (stat >> 24) & 0x7;
      cnt--;
    }

    if (wfifocnt != 0) // && rfifocnt==0)
      return -1;
    if(dwBytesToWrite - i >= 4){
      *(volatile uint32_t*) 0xfff0201C = *(uint32_t*)&buf[i];
      i += 4;
    }else
    {
      *(volatile uint32_t*) 0xfff02010 = buf[i];
      i += 1;
    }
    //log_printf("%02X ",buf[i]);
  }
  *lpBytesWritten = dwBytesToWrite;
  return 0;
}

//write all accumulated commands
int flush()
{
  uint32_t wr_ptr = 0;
  while (wr_ptr != wr_buf_ptr)
  {
    uint32_t written = 0;
    ftStatus = FT_Write(&wr_buffer[wr_ptr], wr_buf_ptr - wr_ptr, &written);
    if (ftStatus)
      return -1; //error
    wr_ptr += written;
  }
  wr_buf_ptr = 0;
  return 0;
}

//fake write function which stores sending data into accummulating buffer instead of writing
int FT_Write_b(
    unsigned char* lpBuffer,
    uint32_t dwBytesToWrite,
    uint32_t* lpBytesWritten
    )
{
  memcpy(&wr_buffer[wr_buf_ptr], lpBuffer, dwBytesToWrite);
  wr_buf_ptr += dwBytesToWrite;
  if (wr_buf_ptr > WR_BUF_SIZE)
    flush(); //we have accumulated enough to flush
  *lpBytesWritten = dwBytesToWrite;
  return 0;
}

// Navigage TMS to desired state with no TDO read back
// please do 0<num_shifts<8
void tms_command(unsigned char num_shifts, unsigned char pattern, unsigned char least_bit, int target_tap_state)
{
  g_tap_state = target_tap_state;
  dwNumBytesToSend = 0;
  if (num_shifts > 5) {
    byOutputBuffer[dwNumBytesToSend++] = (pattern & 0x1f) | 0x20;
    num_shifts -= 5;
    pattern = pattern >> 5;
    byOutputBuffer[dwNumBytesToSend++] = ((pattern >> 5) & 0x1f) | (1 << num_shifts);
  }
  else
    byOutputBuffer[dwNumBytesToSend++] = (pattern & 0x1f) | (1 << num_shifts);
  ftStatus = FT_Write_b(byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
}

void tap_navigate(int start_state, int end_state)
{
  if (start_state == TAP_STATE_PAUSE_IR && end_state == TAP_STATE_RTI)
    tms_command(3, 3, 0, TAP_STATE_RTI);
  else
  if (start_state == TAP_STATE_PAUSE_DR && end_state == TAP_STATE_RTI)
    tms_command(3, 3, 0, TAP_STATE_RTI);
  else
    log_printf("Not implemented TAP navigate sequence %d %d\n", start_state, end_state);
}

//clear input buffer by reading it
int read_all()
{
  while (dwNumBytesRead < sdr_tdo_sz/2) //два символа на байт
  {
    int cnt = 1000000;
    uint32_t stat = *(volatile uint32_t*) 0xfff02000;
    uint32_t rfifocnt = (stat >> 24) & 0x7;
    uint32_t rval = 0;
    while (rfifocnt == 0 && cnt-- > 0)
      ;
    if (cnt <= 0)
      break;
    rval = alt_read_word(0xfff02010);
    if (dwNumBytesRead < sizeof(byInputBuffer) && dwNumBytesRead < sdr_tdo_sz/2)
      byInputBuffer[dwNumBytesRead] = rval;
    //log_printf("r%X ", rval);
    dwNumBytesRead++;
  }
  if (dwNumBytesRead < sdr_tdo_sz/2)
    return -1;
  return 0;
}

//we have "expect buffer"
//and we read data from ftdi chip
//result should be equal
//but we do this check sometimes only - sending bigger packets should increase performance
int check_answer(int force)
{
  //todo
  return 1;
}

//////////////////////////////////////////////////////////////////////////////
// SVF functions
//////////////////////////////////////////////////////////////////////////////

//go to Test Logic Reset state then to Idle
void goto_tlr()
{
  // Navigage TMS to Test-Logic-Reset (TMS=1 more then 4 times) then Idle (TMS=0 once)
  // Data is shifted LSB first, so: 011111 >>> chip
  tms_command(6, 0x1f, 0, TAP_STATE_TLR);
}

//current state is Idle and stay Idle for some clocks
void Idle()
{
  // Idle (TMS=0 any times)
  // Data is shifted LSB first, so the TMS pattern is 000000 >>> chip
  tms_command(6, 0, 0, TAP_STATE_RTI);
}

//define default end state for DR scan operation
//possible commands are:
//ENDDR IRPAUSE;
//ENDDR IDLE;
//ENDDR RESET;
void do_ENDDR()
{
  //use simplified "one char" command recognition
  //is it OK?
  if (rbuffer[7] == 'R')
  {
    //default is PAUSE
    g_def_end_state_dr = TAP_STATE_PAUSE_DR;
  }
  else
  if (rbuffer[7] == 'D')
  {
    //default is IDLE
    g_def_end_state_dr = TAP_STATE_RTI;
  }
  else
  if (rbuffer[7] == 'E')
  {
    //default is RESET
    g_def_end_state_dr = TAP_STATE_TLR;
  }
  else
  {
    //oops....
    log_printf("unknown ENDDR command parameter in line: %s", rbuffer);
  }
}

//define default end state for IR scan operation
//possible commands are:
//ENDIR IRPAUSE;
//ENDIR IDLE;
//ENDIR RESET;
void do_ENDIR()
{
  //use simplified "one char" command recognition
  //is it OK?
  if (rbuffer[7] == 'R')
  {
    //default is PAUSE
    g_def_end_state_ir = TAP_STATE_PAUSE_IR;
  }
  else
  if (rbuffer[7] == 'D')
  {
    //default is IDLE
    g_def_end_state_ir = TAP_STATE_RTI;
  }
  else
  if (rbuffer[7] == 'E')
  {
    //default is RESET
    g_def_end_state_ir = TAP_STATE_TLR;
  }
  else
  {
    //oops....
    log_printf("unknown ENDIR command parameter in line: %s", rbuffer);
  }
}

int g_num_tdo_errors = 0;
int sir(int nclk, unsigned int val)
{
  if (check_answer(true) == 0)
  {
    log_printf("error on check answer\n");
    g_num_tdo_errors++;

    //strange fact is that working with EPM7032S we get one-bit error in check silicon id
    //let us skip only one error
    if (g_num_tdo_errors > 1)
      return 0;
  }

  if (nclk == 0 || nclk > 32)
  {
    log_printf("error SIR parameters\n");
    return 0;
  }

  //go to idle if not idle now
  if (g_tap_state != TAP_STATE_RTI)
  {
    tap_navigate(g_tap_state, TAP_STATE_RTI);
  }

  //from Idle enter to state where we SHIFT IR
  tms_command(4, 0x03, 0, TAP_STATE_SHIFT_IR);

  dwNumBytesToSend = 0;
  while (nclk > 0)
  {

    if (nclk > 8) {
      byOutputBuffer[dwNumBytesToSend++] = 0x80;    //tdi_tdo1000,tms0,rtdo0,tdi0,utdi0
      byOutputBuffer[dwNumBytesToSend++] = 7;
    } else {
      byOutputBuffer[dwNumBytesToSend++] = 0x88;    //tdi_tdo1000,tms1,rtdo0,tdi0,utdi0
      byOutputBuffer[dwNumBytesToSend++] = nclk - 1;
    }
    byOutputBuffer[dwNumBytesToSend++] = val & 0xff;
    val = val >> 8;
    nclk = nclk - 8;  //remember we had sent 8 bits
  }
  ftStatus = FT_Write_b(byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);

  if (g_def_end_state_ir == TAP_STATE_RTI)
    tms_command(2, 0x01, 0, TAP_STATE_RTI); //go to idle
  else
  if (g_def_end_state_ir == TAP_STATE_PAUSE_IR)
    tms_command(1, 0x00, 0, TAP_STATE_PAUSE_IR); //go to pause
  else
  if (g_def_end_state_ir == TAP_STATE_TLR)
    tms_command(5, 0x1d, 0, TAP_STATE_TLR); //go to reset
  return 1;
}

int runidle(int num)
{
  int num_ = num;
  if (num > 20)
    num_ = 20;
  byOutputBuffer[0] = 0x20; //5 раз TMS0
  while (num_ >= 5)
  {
    ftStatus = FT_Write_b(byOutputBuffer, 1, &dwNumBytesSent);
    num_ = num_ - 5;
  }
  if (num_ > 0)
  {
    byOutputBuffer[0] = 0x01 << num_;
    ftStatus = FT_Write_b(byOutputBuffer, 1, &dwNumBytesSent);
  }
  if (num > 20)
  {
    flush();
    Sleep(100);
  }
  return ftStatus;
}

//process typical strings:
//SIR 10 TDI (203);
int do_SIR()
{
  //get command parameters
  unsigned int sir_arg = 0;
  unsigned int tdi_arg = 0;
  int n = sscanf(rbuffer, "SIR %d TDI (%X);", &sir_arg, &tdi_arg);
  if (n != 2)
  {
    log_printf("error processing SIR\n");
    return 0;
  }
  return sir(sir_arg, tdi_arg);
}

//////////////////////////////////////////////////////////////////////////////
// memory alloc/free functions
//////////////////////////////////////////////////////////////////////////////

//allocate arrays for sdr tdi, tdo and mask
//return 1 if ok or 0 if fail
unsigned int alloc_sdr_data(unsigned int size)
{
  //compare new size with size of already allocated buffers
  if (sdr_data_size >= size)
    return 1; //ok, because already allocated enough

  //we need to allocate memory for arrays
  //but first free previously allocated buffers

  //tdi
  if (psdr_tdi_data)
  {
    free(psdr_tdi_data);
    psdr_tdi_data = NULL;
  }

  //tdo
  if (psdr_tdo_data)
  {
    free(psdr_tdo_data);
    psdr_tdo_data = NULL;
  }

  //mask
  if (psdr_mask_data)
  {
    free(psdr_mask_data);
    psdr_mask_data = NULL;
  }

  //smask
  if (psdr_smask_data)
  {
    free(psdr_smask_data);
    psdr_smask_data = NULL;
  }

  psdr_tdi_data = (unsigned char*) malloc(size);
  if (psdr_tdi_data == NULL)
  {
    log_printf("error allocating sdr tdi buffer\n");
    return (0);
  }

  psdr_tdo_data = (unsigned char*) malloc(size);
  if (psdr_tdo_data == NULL)
  {
    free(psdr_tdi_data);
    psdr_tdi_data = NULL;
    log_printf("error allocating sdr tdo buffer\n");
    return (0);
  }

  psdr_mask_data = (unsigned char*) malloc(size);
  if (psdr_mask_data == NULL)
  {
    free(psdr_tdi_data);
    free(psdr_tdo_data);
    psdr_tdi_data = NULL;
    psdr_tdo_data = NULL;
    log_printf("error allocating sdr mask buffer\n");
    return (0);
  }

  psdr_smask_data = (unsigned char*) malloc(size);
  if (psdr_smask_data == NULL)
  {
    free(psdr_tdi_data);
    free(psdr_tdo_data);
    free(psdr_mask_data);
    psdr_tdi_data = NULL;
    psdr_tdo_data = NULL;
    psdr_mask_data = NULL;
    log_printf("error allocating sdr smask buffer\n");
    return (0);
  }

  //remember that we have allocated some size memory
  sdr_data_size = size;

  //we have successfully allocated buffers for sdr data!
  return 1;
}

//free buffers allocated for sdr tdi, tdo, mask
void free_sdr_data()
{
  if (psdr_tdi_data)
    free(psdr_tdi_data);
  if (psdr_tdo_data)
    free(psdr_tdo_data);
  if (psdr_mask_data)
    free(psdr_mask_data);
  if (psdr_smask_data)
    free(psdr_smask_data);
  sdr_data_size = 0;
}

//send some data to DR
//please num_bytes non zero
//offset points into sdr tdi array and func returns updated offset
unsigned int send_sdr_block(unsigned int num_bytes, unsigned int offset, unsigned int has_tdo, unsigned int has_mask,
    int isLast)
{
  unsigned char block_tdi[BLOCK_SZ + 1];
  unsigned char cmd_mask;
  unsigned char b;
  unsigned int i, j;

  //prepare command modifier which says do we expect answer or not
  if (has_tdo)
    cmd_mask = 0x04;
  else
    cmd_mask = 0x00;

  dwNumBytesToSend = 0;
  if (isLast)
    byOutputBuffer[dwNumBytesToSend++] = 0x88 | cmd_mask; //tdi_tdo1001,tms1,rtdo0,tdi0,utdi0
  else
    byOutputBuffer[dwNumBytesToSend++] = 0x80 | cmd_mask; //tdi_tdo
  if (num_bytes > 16)
    log_printf("ERROR: num_bytes can't be greater than 16\n");
  byOutputBuffer[dwNumBytesToSend++] = num_bytes * 8 - 1;
  ftStatus = FT_Write_b(byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);

  //write data, but firstreverse bytes of array
  j = offset;
  for (i = 0; i < num_bytes; i++)
  {
    //gather byte from two thetrades of sdr array
    b = psdr_tdi_data[j--];
    b = b | (psdr_tdi_data[j--] << 4);
    block_tdi[i] = b; //reverseBits(b);
  }
  ftStatus = FT_Write_b(block_tdi, num_bytes, &dwNumBytesSent);

  //should we expect and compare answer from chip?
  if (has_tdo)
  {
    //we should read from chip TDO answer and compare with expected
    //prepare array for comparation

    j = offset;
    for (i = exp_buff_ptr; i < exp_buff_ptr + num_bytes; i++)
    {
      //save tdo byte
      b = psdr_tdo_data[j];
      b = b | (psdr_tdo_data[j - 1] << 4);
      expect_buffer[i] = b;

      //probably we have also tdo mask
      if (has_mask)
      {
        b = psdr_mask_data[j];
        b = b | (psdr_mask_data[j - 1] << 4);
        expect_buffer_mask[i] = b;
      }
      else
        expect_buffer_mask[i] = 0xff;

      j -= 2;
    }
    exp_buff_ptr += num_bytes;
    check_answer(false);
  }

  //offset is index in tdi array which consists of thetrades, that is why mul 2
  offset -= num_bytes * 2;
  return offset;
}

int sdr_nbits(unsigned int nbits, unsigned int has_tdo, unsigned int has_mask, unsigned int has_smask)
{
  unsigned char val_o, val_i, val_m;
  unsigned char cmd_mask;
  unsigned int num_bytes;
  unsigned int offset;

  if (nbits < 1)
    return 0;

  //go to idle if not idle now
  if (g_tap_state != TAP_STATE_RTI)
  {
    tap_navigate(g_tap_state, TAP_STATE_RTI);
  }

  //from Idle go to state where we can shift DR
  tms_command(3, 0x01, 0, TAP_STATE_SHIFT_DR);

  //calc "offset" - index of least thetrade from array of tdi data
  offset = sdr_tdi_sz - 1;

  int isLast = 0;
  //first send large blocks if we have
  while (nbits >= BLOCK_SZ * 8)
  {
    nbits -= BLOCK_SZ * 8;
    if (nbits == 0)
      isLast = 1;
    else
      isLast = 0;
    offset = send_sdr_block(BLOCK_SZ, offset, has_tdo, has_mask, isLast);
  }

  //send smaller block but leave at least one byte
  if (nbits >= 8)
  {
    num_bytes = nbits / 8;
    nbits -= num_bytes * 8;
    if (nbits == 0)
      isLast = 1;
    else
      isLast = 0;
    offset = send_sdr_block(num_bytes, offset, has_tdo, has_mask, isLast);
  }

  //get last byte from tdi, tdo, mask arrays
  val_i = psdr_tdi_data[offset];
  if (offset)
    val_i |= psdr_tdi_data[offset - 1] << 4;

  val_o = psdr_tdo_data[offset];
  if (offset)
    val_o |= psdr_tdo_data[offset - 1] << 4;

  val_m = psdr_mask_data[offset];
  if (offset)
    val_m |= psdr_mask_data[offset - 1] << 4;

  //define we need read answer or not
  cmd_mask = 0;
  if (has_tdo)
    cmd_mask = 0x04;

  if (nbits > 0)
  {
    dwNumBytesToSend = 0;
    byOutputBuffer[dwNumBytesToSend++] = 0x88 | cmd_mask;    //tdi_tdo1001,tms1,rtdo0,tdi0,utdi0 | rtdo_by_mask
    byOutputBuffer[dwNumBytesToSend++] = nbits - 1;
    byOutputBuffer[dwNumBytesToSend++] = val_i;
    ftStatus = FT_Write_b(byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);

    if (has_tdo)
    {
      //because we had sent nbits we will get answer of same nbits coming LSB first
      expect_buffer[exp_buff_ptr] = (val_o << (8 - nbits));
      expect_buffer_mask[exp_buff_ptr] = (val_m << (8 - nbits));
      exp_buff_ptr++;
    }
  }

  if (has_tdo)
  {
    flush();
    read_all();
    expect_buffer[exp_buff_ptr] = (val_o >> nbits) << 7;
    expect_buffer_mask[exp_buff_ptr] = (val_m >> nbits) << 7;
    exp_buff_ptr++;
    check_answer(false);
  }

  //exit from Exit1-DR state depends on target state (defined by ENDDR command)
  if (g_def_end_state_dr == TAP_STATE_RTI)
    tms_command(2, 0x01, 0, TAP_STATE_RTI);
  else
  if (g_def_end_state_dr == TAP_STATE_PAUSE_DR)
    tms_command(1, 0x00, 0, TAP_STATE_PAUSE_DR);
  else
  if (g_def_end_state_dr == TAP_STATE_TLR)
    tms_command(5, 0x1d, 0, TAP_STATE_TLR);
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//string syntax parcing functions
//////////////////////////////////////////////////////////////////////////////

//return hex number in range 0-0xf or error 0xff if char is not a hexadecimal digit
unsigned char char2hex(unsigned char c)
{
  unsigned char v;
  if (c >= '0' && c <= '9')
    v = c - '0';
  else
  if (c >= 'a' && c <= 'f')
    v = c - 'a' + 10;
  else
  if (c >= 'A' && c <= 'F')
    v = c - 'A' + 10;
  else
    v = 0xff; //not a hex
  return v;
}

//skip leading space or tab chars in string
unsigned char* skip_space(unsigned char* ptr)
{
  while (1)
  {
    unsigned char c = *ptr;
    if (c == ' ' || c == 9)
    {
      ptr++;
      continue;
    }
    else
      break;
  }
  return ptr;
}

//skip from string word which we expect to see
unsigned char* skip_expected_word(unsigned char* ptr, unsigned char* pword, unsigned int* presult)
{
  //assume error result
  *presult = 0;
  ptr = skip_space(ptr);
  while (1)
  {
    unsigned char c1, c2;
    c2 = *pword;
    if (c2 == 0)
    {
      //end of comparing word
      *presult = 1;
      return ptr;
    }
    c1 = *ptr;
    if (c1 == c2)
    {
      //at this point strings are equal, but compare next chars
      ptr++;
      pword++;
      continue;
    }
    else
      break; //string has no expected word
  }
  return ptr;
}

//get word from string
unsigned char* get_and_skip_word(unsigned char* ptr, unsigned char* pword, unsigned int* presult)
{
  int i;
  unsigned char c;
  *presult = 0;

  //assume error result
  *presult = 0;
  ptr = skip_space(ptr);
  for (i = 0; i < 8; i++)
      {
    c = *ptr;
    if (c == ' ' || c == 9 || c == 0 || c == 0xd || c == 0xa)
    {
      //end of getting word
      *pword = 0;
      *presult = 1;
      return ptr;
    }
    *pword++ = c;
    ptr++;
  }
  *pword = 0;
  return ptr;
}

//read decimal integer from string
//returned 0xffffffff mean error
unsigned char* get_dec_integer(unsigned char* ptr, unsigned int* presult)
{
  unsigned int r = 0;
  ptr = skip_space(ptr);
  while (1)
  {
    unsigned char c;
    c = *ptr;
    if (c >= '0' && c <= '9')
    {
      r = r * 10 + (c - '0');
      ptr++;
      continue;
    }
    else
    if (c == ' ' || c == 9)
    {
      *presult = r;
      break;
    }
    else
    {
      //unexpected char
      *presult = 0xffffffff;
      break;
    }
  }
  return ptr;
}

unsigned char* read_hex_array(unsigned char* ptr, unsigned char* pdst, unsigned int num_bits, FILE* f,
    unsigned int* pcount, unsigned int* presult)
{
  unsigned char c, chr;
  unsigned int len, chr_count;
  char* pstr;

  //assume we fail
  *presult = 0;

  chr_count = 0;
  while (num_bits > 0)
  {
    //get char from string and convert to hex digit
    chr = *ptr;
    c = char2hex(chr);
    if (c != 0xFF)
    {
      //hexadecimal digit is correct, save it
      *pdst++ = c;
      //remember that 4 bits we got
      num_bits -= 4;
      //go to next char
      ptr++;
      chr_count++;
    }
    else
    {
      //char from string is not hexadecimal, but what is this?
      if (chr == ')')
      {
        //end of hexadecimal array!!!
        ptr++;
        //return SUCCESS
        *presult = 1;
        *pcount = chr_count;
        return ptr;
      }
      else
      if (chr == 0 || chr == 0xd || chr == 0xa)
      {
        //end of string, we need to continue by reading next string from file
        pstr = fgets_(rbuffer, MAX_STRING_LENGTH - 1);
        if (pstr == NULL)
        {
          //file read fails
          return NULL;
        }
        len = strlen(pstr);
        if (pstr[len - 1] == 0xA || pstr[len - 1] == 0xD)
          pstr[len - 1] = 0;
        ptr = (unsigned char*) rbuffer;
        ptr = skip_space(ptr);
      }
      else
      {
        //unexpected char, error in syntax?
        log_printf("unexpected char %02X\n", chr);
        return NULL;
      }
    }
  }
  //get char from string and convert to hex digit
  chr = *ptr++;
  if (chr == ')')
  {
    //yes, we see final bracket
    *presult = 1;
    *pcount = chr_count;
    return ptr;
  }
  return NULL;
}

/*
 process typical strings:
 SDR 16 TDI (FFFF) TDO (C0C7) MASK (FFFF);
 SDR 16 TDI (FFFF) TDO (2027);
 SDR 13 TDI (0000);
 return 1 on success, 0 is fail
 note that arguments can be very long and take several lines in a source file
 */
int do_SDR(FILE* f)
{
  int has_tdo, has_mask, has_smask;
  unsigned int r;
  unsigned char word[16];
  unsigned int num_bits = 0;
  unsigned int num_bytes;
  unsigned char* pdest;
  unsigned int* pdest_count;
  unsigned char* ptr = (unsigned char*) rbuffer;

  //at begin of string we expect to see word "SDR"
  ptr = skip_expected_word(ptr, (unsigned char*) "SDR", &r);
  if (r == 0)
  {
    log_printf("syntax error for SDR command\n");
    return 0;
  }

  //now we expect to get decimal number of bits for shift
  ptr = get_dec_integer(ptr, &num_bits);
  if (num_bits == 0xffffffff)
  {
    log_printf("syntax error for SDR command, cannot get number of bits\n");
    return 0;
  }

  //how many bytes? calculate space required and allocate
  num_bytes = (num_bits + 7) / 8;
  //each byte takes 2 chars in string, plus we reserve 8 additional bytes for safity
  if (alloc_sdr_data(num_bytes * 2 + 8) == 0)
  {
    log_printf("error on SDR command\n");
    return 0;
  }

  //we expect some words like TDI, TDO, MASK or SMASK here
  //order of words can be different
  has_tdo = 0;
  has_mask = 0;
  has_smask = 0;
  while (1)
  {
    if (ptr == 0 || ptr[0] == '\0')
      ptr = (unsigned char*) fgets_(rbuffer, MAX_STRING_LENGTH - 1);
    ptr = skip_space(ptr);
    ptr = get_and_skip_word(ptr, word, &r);
    if (r == 0)
    {
      log_printf("syntax error for SDR command, cannot fetch parameter word\n");
      return 0;
    }

    //analyze words
    if (strcmp((char*) word, "TDI") == 0)
    {
      pdest = psdr_tdi_data;
      pdest_count = &sdr_tdi_sz;
    }
    else
    if (strcmp((char*) word, "TDO") == 0)
    {
      has_tdo = 1;
      pdest = psdr_tdo_data;
      pdest_count = &sdr_tdo_sz;
    }
    else
    if (strcmp((char*) word, "MASK") == 0)
    {
      has_mask = 1;
      pdest = psdr_mask_data;
      pdest_count = &sdr_mask_sz;
    }
    else
    if (strcmp((char*) word, "SMASK") == 0)
    {
      has_smask = 1;
      pdest = psdr_smask_data;
      pdest_count = &sdr_smask_sz;
    }
    else
    if (strcmp((char*) word, ";") == 0)
    {
      //end of string!
      //send bitstream to jtag
      sdr_nbits(num_bits, has_tdo, has_mask, has_smask);
      break;
    }
    else
    {
      log_printf("syntax error for SDR command, unknown parameter word\n");
      return 0;
    }

    //parameter should be in parentheses
    ptr = skip_expected_word(ptr, (unsigned char*) "(", &r);
    if (r == 0)
    {
      log_printf("syntax error for SDR command, expected char ( after TDI word\n");
      return 0;
    }
    //now expect to read hexadecimal array of tdi data
    ptr = read_hex_array(ptr, pdest, num_bits, f, pdest_count, &r);
  }
  return 1;
}

/*
 process typical strings:
 RUNTEST 53 TCK;
 */
void do_RUNTEST()
{
  int expected_state = -1;

  //get command parameters
  unsigned int tck = 0;
  int n = sscanf(rbuffer, "RUNTEST %d TCK;", &tck);
  if (n == 1)
  {
    //1 param
    expected_state = g_least_runtest_state;
  }
  else
  {
    n = sscanf(rbuffer, "RUNTEST IDLE %d TCK", &tck);
    if (n)
      expected_state = TAP_STATE_RTI;
    else
    {
      n = sscanf(rbuffer, "RUNTEST DRPAUSE %d TCK", &tck);
      if (n)
        expected_state = TAP_STATE_PAUSE_DR;
      else
      {
        n = sscanf(rbuffer, "RUNTEST IRPAUSE %d TCK", &tck);
        if (n)
          expected_state = TAP_STATE_PAUSE_IR;
        else
        {
          log_printf("error processing RUNTEST\n");
        }
      }
    }
  }

  //check current TAP state aganst expected
  if (g_tap_state != expected_state)
  {
    g_least_runtest_state = expected_state;
    tap_navigate(g_tap_state, expected_state);
  }

  if (tck)
    runidle(tck);
}

/*
 process typical strings:
 STATE IDLE;
 */
void do_STATE()
{
  Idle();
}

//reset JTAG
void do_TRST()
{
  goto_tlr();
}

void do_FREQUENCY()
{
  // not available on cyclone v
  return;
}

//////////////////////////////////////////////////////////////////////////////
// SVF PLAYER IS HERE
//////////////////////////////////////////////////////////////////////////////

void prepare_scan_mgr() {
  uint32_t stat = *(volatile uint32_t*) 0xfff02000; // scanmgr
  uint32_t wfifocnt = (stat >> 28) & 0x7;
  uint32_t isactive = (stat >> 31) & 0x1;
  if (isactive == 0 && wfifocnt == 0) {
    *(volatile uint32_t*) 0xfff02004 = 0x80; // enable chain7 jtag to fpga
  }
  else
    log_printf("error init scan mgr!");
  // после переключения отладка не возможна
  *(volatile uint32_t*) 0xffd08030 = 0x1; // sysmgr_ctrl_fpgajtagen scanManager drive jtag signals to fpga
}
void restore_scan_mgr_regs(){
  *(volatile uint32_t*) 0xffd08030 = 0; // disable scanManager jtag signals to fpga
  *(volatile uint32_t*) 0xfff02004 = 0; // disable chain7 jtag to fpga
}
int play_svf(char * fileName)
{
  init_vars();

  prepare_scan_mgr();
  int res = 0;
  if (FR_OK != f_open(&svfFile, fileName, FA_READ))
  {
    log_printf("FAT: Unable to open %s\n", fileName);
    res = -1;
    goto do_exit;
  }

  while (1)
  {
    flush();
    int n, len;
    char* pstr;

    //get string from text file
    pstr = fgets_(rbuffer, MAX_STRING_LENGTH - 1);
    if (pstr == NULL)
      break;
    len = strlen(pstr);

    if (len > 0)
    {
      if (pstr[len - 1] == 0xA || pstr[len - 1] == 0xD)
        pstr[len - 1] = 0;
      if (len > 1)
        if (pstr[len - 2] == 0xA || pstr[len - 2] == 0xD)
          pstr[len - 2] = 0;
    }

    //analyze 1st word
    n = sscanf(pstr, "%s", command);
    if (n == 0)
      break;

    if ( command[0] == '!' /* Altera Quartus marks SVF comment lines with '!' char */
    || (command[0] == '/' && command[1] == '/') /*Xilinx marks comment lines with '//' chars*/
        )
    {
      //line is commented
      if (check_answer(true) == 0)
      {
        log_printf("error on check TDO answer\n");
        res = -1;
        goto do_exit;
      }

      //check important commented lines
      if (
      (strcmp(command, "!CHECKING") == 0) ||
          (strcmp(command, "!BULK") == 0) ||
          (strcmp(command, "!PROGRAM") == 0) ||
          (strcmp(command, "!VERIFY") == 0) ||
          (command[0] == '/' && command[1] == '/')
          )
      {
        log_printf("-----------------------------------\n");
        log_printf("%s\n", rbuffer);
        line = 0;
      }
    }
    else
    {
      //log_printf("%s\n", command);
      line++;
      //real command is here
      if (strcmp(command, "SIR") == 0)
      {
        if (do_SIR() == 0)
        {
          //probably error on check answer
          res = -1;
          goto do_exit;
        }
      }
      else
      if (strcmp(command, "SDR") == 0)
      {
        do_SDR(0);
      }
      else
      if (strcmp(command, "RUNTEST") == 0)
        do_RUNTEST();
      else
      if (strcmp(command, "STATE") == 0)
        do_STATE();
      else
      if (strcmp(command, "TRST") == 0)
        do_TRST();
      else
      if (strcmp(command, "ENDDR") == 0)
        do_ENDDR();
      else
      if (strcmp(command, "ENDIR") == 0)
        do_ENDIR();
      else
      if (strcmp(command, "FREQUENCY") == 0)
        do_FREQUENCY();
      else
      if (strcmp(command, "HDR") == 0)
      {
      }
      else
      if (strcmp(command, "TDR") == 0)
      {
      }
      else
      if (strcmp(command, "HIR") == 0)
      {
      }
      else
      if (strcmp(command, "TIR") == 0)
      {
      }
      else
      {
        log_printf("Unknown command in line: %s\n", rbuffer);
        res = -1;
        goto do_exit;
      }
    }
  }

  flush();
  if (check_answer(true) == 0)
  {
    log_printf("error on check TDO answer\n");
  }

  f_close( &svfFile );
do_exit:
  free_sdr_data();
  restore_scan_mgr_regs();
  return res;
}
