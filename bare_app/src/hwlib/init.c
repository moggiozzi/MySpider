#include "init.h"
#include "logger.h"
#include "fpga.h"
#include "interrupts.h"
#include "alt_pt.h"
#include "alt_cache.h"
#include "alt_globaltmr.h"
#include "pmu.h"
#include "timer.h"
#include "qspi.h"
#include "alt_qspi.h"
#include "alt_interrupt.h"
#include "alt_watchdog.h"
#include "uart.h"
#include "dma.h"
#include "scu.h"

extern void enableCahceCoherency(void);

ALT_STATUS_CODE system_init(void) {
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
  int status_err = ERR_SYSTEM_INIT_OK;
  //log_printf("INFO: System Initialization.\n");

  // todo WD
  alt_wdog_stop(ALT_WDOG0);

  // FPGA
  if (status == ALT_E_SUCCESS) {
    status = init_fpga(false);
     if (status != ALT_E_SUCCESS)
     {
       status_err = ERR_SYSTEM_INIT_FPGA;
       //log_printf("ERROR: FPGA NOT INIT! %d", status);
     }
  }

  // global timer (должен инициализироваться до инициализации прерываний)
  if (status == ALT_E_SUCCESS) {
    //log_printf("INFO: Setting up Global Timer.\n");
    if (!alt_globaltmr_int_is_enabled()) {
      status = alt_globaltmr_init();
    }
    if (status != ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_GLOBAL_TIMER;
      //log_printf("ERROR: GLOBAL TIMER NOT INIT! %d", status);
    }
  }

  // interrupts
  if (status == ALT_E_SUCCESS) {
    //log_printf("INFO: Configure interrupt system.\n");
    status = init_interrupts();
    if (status != ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_INTERRUPTS;
      //log_printf("ERROR: INTERRUPTS NOT INIT! %d", status);
    }
  }

// not tested
//  enableCahceCoherency();
//  enable_scu();

  // Populating page table and enabling MMU
  if (status == ALT_E_SUCCESS) {
    //log_printf("INFO: Populating page table and enabling MMU.\n");
    status = alt_pt_init();
    if (status != ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_MMU;
      //log_printf("ERROR: MMU NOT INIT! %d", status);
    }
  }

  // Enabling caches
  if (status == ALT_E_SUCCESS) {
    //log_printf("INFO: Enabling caches.\n");
    status = alt_cache_system_enable();
    if (status != ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_CACHES;
      //log_printf("ERROR: CACHES NOT INIT! %d", status);
    }
  }

  // Performance Monitoring Unit
  if (status == ALT_E_SUCCESS) {
    //log_printf("INFO: Enabling Performance Monitoring Unit.\n");
    status = init_pmu(1, 0);
    if(status!= ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_PMU;
      //log_printf("ERROR: PMU NOT INIT! %d", status);
    }
  }

  // Timers
  if (status == ALT_E_SUCCESS) {
    status = init_timer();
    if (status != ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_TIMER;
      //log_printf("ERROR: TIMER NOT INIT! %d", status);
    }
  }

  // QSPI
  if (status == ALT_E_SUCCESS) {
    status = init_qspi();
    if (status != ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_QSPI;
      //log_printf("ERROR: QSPI NOT INIT! %d", status);
    }
  }

#ifdef USE_SERIAL_PORT
  if (status == ALT_E_SUCCESS)
  {
    status = init_uart();
    if (status != ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_UART;
    }
  }
#endif

  // DMA
  if (status == ALT_E_SUCCESS) {
    status = init_dma();
    if (status != ALT_E_SUCCESS)
    {
      status_err = ERR_SYSTEM_INIT_DMA;
      //log_printf("ERROR: DMA NOT INIT! %d", status);
    }
  }

//  // Initialize random number generator
//  if (status == ALT_E_SUCCESS) {
//    int random_seed = alt_globaltmr_counter_get_low32();
//    log_printf("INFO: Using random seed = 0x%04x.\n", random_seed);
//    srand(random_seed);
//  }
  //wait_fpga_ready();

  if (status_err != ERR_SYSTEM_INIT_UART)
  {
    if (status_err)
    {
      log_printf("INFO: System init error code= %d status= %d\n", status_err, status);
    }
    else
    log_printf("INFO: system_init done ok\n");
  }
  return status;
}

ALT_STATUS_CODE system_uninit(void)
{
    printf("INFO: System shutdown.\n");

    //alt_dma_channel_free(Dma_Channel);
    //alt_dma_uninit();
    alt_qspi_disable();
    alt_qspi_uninit();
    alt_cache_system_disable();
    alt_pt_uninit();
    alt_int_cpu_uninit();

    printf("\n");

    return ALT_E_SUCCESS;
}
