/******************************************************************************
*
* Copyright 2014 Altera Corporation. All Rights Reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. The name of the author may not be used to endorse or promote products
* derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED. IN NO
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
*
******************************************************************************/

#include "alt_mmu.h"
#include <stdio.h>
#include <string.h>
#include "mem_map_defs.h"

// Determine size of an array
#define ARRAY_COUNT(array) (sizeof(array) / sizeof(array[0]))

// MMU Page table - 16KB aligned at 16KB boundary
 static uint32_t __attribute__ ((aligned (0x4000))) alt_pt_storage[4096];

/******************************************************************************/
/*!
 * Allocate MMU page table
 *
 * \return      page table
 */
static void * alt_pt_alloc(const size_t size, void * context)
{
    return context;
}

/******************************************************************************/
/*!
 * Initialize MMU page tables
 *
 * \return      result of the function
 */
ALT_STATUS_CODE alt_pt_init(void)
{
    // Populate the page table with sections (1 MiB regions).
    ALT_MMU_MEM_REGION_t regions[] =
    {
        // Memory area
        {
            .va         = (void *)MEMORY_BASE_ADDR,
            .pa         = (void *)MEMORY_BASE_ADDR,
            .size       = MEMORY_COMMON_END_ADDR - MEMORY_BASE_ADDR,
            .access     = ALT_MMU_AP_PRIV_ACCESS,
            .attributes = ALT_MMU_ATTR_WBA,
            .shareable  = ALT_MMU_TTB_S_NON_SHAREABLE,
            .execute    = ALT_MMU_TTB_XN_DISABLE,
            .security   = ALT_MMU_TTB_NS_SECURE
        },
        // for DMA
        {
            .va         = (void *)MEMORY_DMA_BUF_ADDR,
            .pa         = (void *)MEMORY_DMA_BUF_ADDR,
            .size       = MEMORY_DMA_BUF_SIZE,
            .access     = ALT_MMU_AP_PRIV_ACCESS,
            .attributes = ALT_MMU_ATTR_NC_NC,
            .shareable  = ALT_MMU_TTB_S_SHAREABLE,
            .execute    = ALT_MMU_TTB_XN_DISABLE,
            .security   = ALT_MMU_TTB_NS_SECURE
        },
        // SHARED
        {
            .va         = (void *)MEMORY_SHARED_BUF_ADDR,
            .pa         = (void *)MEMORY_SHARED_BUF_ADDR,
            .size       = MEMORY_SHARED_BUF_SIZE,
            .access     = ALT_MMU_AP_PRIV_ACCESS,
            .attributes = ALT_MMU_ATTR_NC_NC,
            .shareable  = ALT_MMU_TTB_S_SHAREABLE,
            .execute    = ALT_MMU_TTB_XN_ENABLE,
            .security   = ALT_MMU_TTB_NS_SECURE
        },
        // Device area: Everything else
        {
            .va         = (void *)MEMORY_END_ADDR,
            .pa         = (void *)MEMORY_END_ADDR,
            .size       = (0xffffffff - MEMORY_END_ADDR) + 1,
            .access     = ALT_MMU_AP_PRIV_ACCESS,
            .attributes = ALT_MMU_ATTR_DEVICE_NS,
            .shareable  = ALT_MMU_TTB_S_NON_SHAREABLE,
            .execute    = ALT_MMU_TTB_XN_ENABLE,
            .security   = ALT_MMU_TTB_NS_SECURE
        }
    };

    ALT_STATUS_CODE status = ALT_E_SUCCESS;
    uint32_t * ttb1 = NULL;

    if (status == ALT_E_SUCCESS)
    {
        status = alt_mmu_init();
    }

    if (status == ALT_E_SUCCESS)
    {
        size_t reqsize = alt_mmu_va_space_storage_required(regions,
                                                           ARRAY_COUNT(regions));
        if (reqsize > sizeof(alt_pt_storage))
        {
            status = ALT_E_ERROR;
        }
    }

    if (status == ALT_E_SUCCESS)
    {
        status = alt_mmu_va_space_create(&ttb1,
                                         regions, ARRAY_COUNT(regions),
                                         alt_pt_alloc, alt_pt_storage);
    }

    if (status == ALT_E_SUCCESS)
    {
        status = alt_mmu_va_space_enable(ttb1);
    }

    return status;
}

/******************************************************************************/
/*!
 * Cleanup MMU page tables
 *
 * \return      result of the function
 */
ALT_STATUS_CODE alt_pt_uninit(void)
{
    if (alt_mmu_disable() != ALT_E_SUCCESS)
    {
        printf("DEBUG[PT]: Failure on line %d.\n", __LINE__);
    }

    return ALT_E_SUCCESS;
}
