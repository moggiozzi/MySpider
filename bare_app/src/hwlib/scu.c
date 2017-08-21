#include "socal/hps.h"
#include <stdint.h>

#define ALT_MPUSCU_CNTRL_OFFSET           0x0
#define ALT_MPUSCU_CNTRL_ENABLE           0x1
#define ALT_MPUSCU_CNTRL_ADDR_FILTER      0x2
#define ALT_MPUSCU_CNTRL_RAM_PARITY       0x4
#define ALT_MPUSCU_CNTRL_SPEC_LINEFILL    0x8
#define ALT_MPUSCU_CNTRL_PORT0_EN         0x10
#define ALT_MPUSCU_CNTRL_SCU_STANDBY_EN   0x20
#define ALT_MPUSCU_CNTRL_IC_STANDBY_EN    0x40

#define ALT_MPUSCU_INVALIDATE_OFFSET      0xC
#define ALT_MPUSCU_INVALIDATE_ALL         0x0000FFFF

void enable_scu(void)
{
  volatile uint32_t *pCntrl = (uint32_t *)(ALT_MPUSCU_OFST + ALT_MPUSCU_CNTRL_OFFSET);
  volatile uint32_t *pInvalidate = (uint32_t *)(ALT_MPUSCU_OFST + ALT_MPUSCU_INVALIDATE_OFFSET);

  *pCntrl |= ALT_MPUSCU_CNTRL_SPEC_LINEFILL | ALT_MPUSCU_CNTRL_ENABLE;
  *pInvalidate = ALT_MPUSCU_INVALIDATE_ALL;
  
}
