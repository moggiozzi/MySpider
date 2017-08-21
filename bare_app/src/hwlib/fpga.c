#include "fpga.h"
#include "alt_fpga_manager.h"

ALT_STATUS_CODE init_fpga(bool isWaitReady) {
  volatile uint32_t val = 0;
  *(uint32_t*) 0xffd05020 = val; // Enable misc modules; Reset manager miscmodrst (deassert misc module reset)
  *(uint32_t*) 0xffd0501c = val; // Enable hps2fpga, lwhps2fpga, fpga2hps; Reset manager brgmodrst.rstmgr (deassert bridges reset)
  val = 0x3fff;
  *(uint32_t*) 0xffc25080 = val; // Enable fpga2sdram; SDRAM controller fpgaportrst.sdr (enable all fpga fabric ports)
  val = 0x19;
  *(uint32_t*) 0xff800000 = val; // Do bridges associated address range visible for L3 (NIC-301); remap
  if (isWaitReady)
    return wait_fpga_ready();
  return ALT_E_SUCCESS;
}

ALT_STATUS_CODE wait_fpga_ready() {
  ALT_FPGA_STATE_t fpga_state;
  uint32_t cnt = 0;
  do{
    fpga_state = alt_fpga_state_get();
  }while( fpga_state != ALT_FPGA_STATE_USER_MODE && cnt++ < 10000000);
  if (fpga_state != ALT_FPGA_STATE_USER_MODE) {
    return ALT_E_FPGA_NOT_USER_MODE;
  }
  return ALT_E_SUCCESS;
}

ALT_STATUS_CODE restart_fpga(bool isWaitReady) {
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
  if (status == ALT_E_SUCCESS)
    status = alt_fpga_control_enable();
  if (status == ALT_E_SUCCESS)
    status = alt_fpga_reset_assert();
  if (status == ALT_E_SUCCESS)
    status = alt_fpga_reset_deassert();
  if (status == ALT_E_SUCCESS && isWaitReady)
    status = wait_fpga_ready();
  alt_fpga_control_disable();
  return status;
}
