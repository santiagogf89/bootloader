/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "bootloader_common.h"
#include "bootloader/bl_context.h"
#include "fsl_device_registers.h"
#include "qspi/qspi.h"
#include "lpuart/fsl_lpuart.h"
#include "utilities/fsl_assert.h"
#if BL_ENABLE_CRC_CHECK
#include "bootloader/bl_app_crc_check.h"
#endif
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define kIRC_48M 48000000UL

enum
{
    //! @brief Mask for the bit of RCM_MR[BOOTROM] indicating that the BOOTCFG0 pin was asserted.
    kBootedViaPinMask = 1
};

#define PORT_IRQC_INTERRUPT_FALLING_EDGE 0xA
#define PORT_IRQC_INTERRUPT_DISABLE 0

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

/*
 *  This function is called if parameters for QSPI io configuration is invalid
 */
static status_t quadspi_iomux_init_error_handle(void)
{
    // Restore all quadspi related pin mux to default value out of reset.
    for (uint32_t pin = 0; pin < 14; pin++)
    {
        PORTE->PCR[pin] &= (uint32_t)~PORT_PCR_MUX_MASK;
    }
    return kStatus_InvalidArgument;
}

/* This function is called for configurating pinmux for qspi module
 * This function only supports switching default mode on specified pins
 * If the isParallel parameter is used to determine which group of QSPI interface should be configured.
 */
status_t quadspi_iomux_configuration(qspi_config_t *config_base)
{
    bool needInitQuadSPI0A = true;
    bool needInitQuadSPI0B = false;
    bool isBasicConfiguration = false;

    // If the config_base pointer is invalid, only QuadSPI0A will be configured.
    if (config_base == NULL)
    {
        needInitQuadSPI0A = true;
        needInitQuadSPI0B = false;
        isBasicConfiguration = true;
    }
    else // Determine which Ports need to be configured based on parameters in qspi config block
    {
        if (config_base->sflash_type > kQspiFlashPad_Octal)
        {
            return quadspi_iomux_init_error_handle();
        }

        if (config_base->sflash_A1_size > 0)
        {
            needInitQuadSPI0A = true;
        }
        else
        {
            needInitQuadSPI0A = false;
        }

        if (config_base->sflash_B1_size > 0)
        {
            needInitQuadSPI0B = true;
        }
        else
        {
            needInitQuadSPI0B = false;
        }
    }

    if (needInitQuadSPI0A)
    {
        // Configure default GPIOs for basic read
        PORTE->PCR[1] = PORT_PCR_MUX(5); // QSPI0A_SCLK is ALT5 for pin PTE1
        PORTE->PCR[2] = PORT_PCR_MUX(5); // QSPI0A_DATA0 is ALT5 for pin PTE2
        PORTE->PCR[4] = PORT_PCR_MUX(5); // QSPI0A_DATA1 is ALT5 for pin PTE4
        PORTE->PCR[5] = PORT_PCR_MUX(5); // QSPI0A_SS0_B is ALT5 for pin PTE5

        if (isBasicConfiguration)
        {
            return kStatus_Success;
        }

        if (config_base->sflash_A2_size > 0)
        {
            PORTE->PCR[7] = PORT_PCR_MUX(7); // QSPI0A_SS1_B is ALT7 for pin PTE7
        }

        if ((config_base->sflash_type == kQspiFlashPad_Quad) || (config_base->sflash_type == kQspiFlashPad_Octal))
        {
            PORTE->PCR[0] = PORT_PCR_MUX(5); // QSPI0A_DATA3 is ALT5 for pin PTE0
            PORTE->PCR[3] = PORT_PCR_MUX(5); // QSPI0A_DATA2 is ALT5 for pin PTE3
        }
        if (config_base->sflash_type == kQspiFlashPad_Octal)
        {
            PORTE->PCR[8] = PORT_PCR_MUX(5);  // QSPI0B_DQ4 is ALT5 for pin PTE8
            PORTE->PCR[10] = PORT_PCR_MUX(5); // QSPI0B_DQ5 is ALT5 for pin PTE10
            PORTE->PCR[9] = PORT_PCR_MUX(5);  // QSPI0B_DQ6 is ALT5 for pin PTE9
            PORTE->PCR[6] = PORT_PCR_MUX(5);  // QSPI0B_DQ7 is ALT5 for pin PTE6

            if (config_base->differential_clock_pin_enable)
            {
                PORTE->PCR[7] = PORT_PCR_MUX(5); // CK# is ALT5 for pin PTE7
            }
            if (config_base->flash_CK2_clock_pin_enable)
            {
                PORTE->PCR[12] = PORT_PCR_MUX(5); // CK2 is ALT5 for pin PTE12
                if (config_base->differential_clock_pin_enable)
                {
                    PORTE->PCR[13] = PORT_PCR_MUX(5); // CK2# is ALT5 for pin PTE13
                }
            }
        }

        if (config_base->dqs_enable)
        {
            PORTE->PCR[11] = PORT_PCR_MUX(7); // QSPI0A_DQS is ALT7 for pin PTE11
        }
    }
    if (needInitQuadSPI0B)
    {
        PORTE->PCR[7] = PORT_PCR_MUX(5);  // QSPI0B_SCLK is ALT5 for pin PTE7
        PORTE->PCR[8] = PORT_PCR_MUX(5);  // QSPI0B_DATA0 is ALT5 for pin PTE8
        PORTE->PCR[10] = PORT_PCR_MUX(5); // QSPI0B_DATA1 is ALT5 for pin PTE10
        PORTE->PCR[11] = PORT_PCR_MUX(5); // QSPI0B_SS0_B is ALT5 for pin PTE11

        if (config_base->sflash_type == kQspiFlashPad_Quad)
        {
            PORTE->PCR[9] = PORT_PCR_MUX(5); // QSPI0B_DATA2 is ALT5 for pin PTE9
            PORTE->PCR[6] = PORT_PCR_MUX(5); // QSPI0B_DATA3 is ALT5 for pin PTE6
        }

        if (config_base->sflash_B2_size > 0)
        {
            PORTE->PCR[13] = PORT_PCR_MUX(5); // QSPI0B_SS1_B is ALT5 for pin PTE13
        }

        if (config_base->dqs_enable)
        {
            PORTE->PCR[12] = PORT_PCR_MUX(5); // QSPI0B_DQS is ALT5 for pin PTE12
        }
    }

    return kStatus_Success;
}

void init_hardware(void)
{
    // Disable the MPU otherwise USB cannot access the bus
    MPU->CESR &= (uint32_t)~MPU_CESR_VLD_MASK;

    // Enable all the ports
    SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK |
                   SIM_SCGC5_PORTE_MASK);

    uint32_t tmp = SIM->SOPT2;
    tmp &= (uint32_t)~SIM_SOPT2_LPUARTSRC_MASK;
    // set PLLFLLSEL to select the IRC48Mhz
    // set LPUART Clock source to MCGFLLCLK or MCGPLLCLK
    tmp |= SIM_SOPT2_PLLFLLSEL_MASK | SIM_SOPT2_LPUARTSRC(1);
    SIM->SOPT2 = tmp;
}

void deinit_hardware(void)
{
    // Restore MPU to default state (enabled)
    MPU->CESR |= MPU_CESR_VLD_MASK;

#if BL_ENABLE_CRC_CHECK && BL_FEATURE_CRC_ASSERT
    restore_crc_check_failure_pin();
#endif

    // Keep enabling PORTE clock since it is needed for QSPI module
    if (is_quadspi_configured())
    {
        SIM->SCGC5 &=
            (uint32_t) ~(SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK);
    }
    else
    {
        SIM->SCGC5 &= (uint32_t) ~(SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK |
                                   SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK);
    }

    // Restore SIM_SOPTx related bits being changed
    SIM->SOPT1 &= (uint32_t) ~(SIM_SOPT1_USBREGEN_MASK);
    SIM->SOPT2 &= (uint32_t) ~(SIM_SOPT2_LPUARTSRC_MASK | SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK);
}

bool usb_clock_init(void)
{
    // Select IRC48M clock
    SIM->SOPT2 |= (SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK);

    // Enable USB-OTG IP clocking
    SIM->SCGC4 |= (SIM_SCGC4_USBOTG_MASK);

    // Configure enable USB regulator for device
    SIM->SOPT1 |= SIM_SOPT1_USBREGEN_MASK;

    USB0->CLK_RECOVER_CTRL |= USB_CLK_RECOVER_CTRL_CLOCK_RECOVER_EN_MASK;
    USB0->CLK_RECOVER_IRC_EN |= USB_CLK_RECOVER_IRC_EN_IRC_EN_MASK;
    return true;
}

uint32_t get_bus_clock(void)
{
    uint32_t coreClockDivider = ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> SIM_CLKDIV1_OUTDIV1_SHIFT) + 1;
    uint32_t busClockDivider = ((SIM->CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> SIM_CLKDIV1_OUTDIV2_SHIFT) + 1;
    uint32_t mcgOutClock = SystemCoreClock * coreClockDivider;

    return (mcgOutClock / busClockDivider);
}

//! @brief Return uart clock frequency according to instance
//! @note: This function is updated according the "K80_256 RM Rev0 draft a", section 6.7.9
uint32_t get_uart_clock(uint32_t instance)
{
    uint32_t lpuartClock;
    uint32_t pllFllFrac = ((SIM->CLKDIV3 & SIM_CLKDIV3_PLLFLLFRAC_MASK) >> SIM_CLKDIV3_PLLFLLFRAC_SHIFT) + 1;
    uint32_t pllFllDiv = ((SIM->CLKDIV3 & SIM_CLKDIV3_PLLFLLDIV_MASK) >> SIM_CLKDIV3_PLLFLLDIV_SHIFT) + 1;
    lpuartClock = kIRC_48M * pllFllFrac / pllFllDiv;

    return lpuartClock;
}

/*
uint32_t read_autobaud_pin( uint32_t instance )
{
    switch(instance)
    {
        case 0:
            return (GPIO_RD_PDIR(GPIOB) >> UART0_RX_GPIO_PIN_NUM) & 1;
        case 1:
            return (GPIO_RD_PDIR(GPIOC) >> UART1_RX_GPIO_PIN_NUM) & 1;
        case 2:
            return (GPIO_RD_PDIR(GPIOD) >> UART2_RX_GPIO_PIN_NUM) & 1;
        default:
            return 0;
    }
}
*/
bool is_boot_pin_asserted(void)
{
    // Boot pin for Flash only target
    return false;
}

void dummy_byte_callback(uint8_t byte)
{
    (void)byte;
}

void debug_init(void)
{
}

#if __ICCARM__

size_t __write(int handle, const unsigned char *buf, size_t size)
{
    return size;
}

#endif // __ICCARM__

void update_available_peripherals()
{
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
