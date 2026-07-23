/******************** (C) COPYRIGHT 2014 SONiX *******************************
* COMPANY:			SONiX
* DATE:					2014/05
* AUTHOR:				SA1
* IC:				SN32F240/230/220
* DESCRIPTION:	SPI0 related functions.
*____________________________________________________________________________
* REVISION	Date				User		Description
* 1.0				2013/12/17	SA1			1. First release
*	1.1				2014/05/23	SA1			1. Add __SSP0_DATA_FETCH_HIGH_SPEED macro
*
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include "SPI.h"

#define SN_SSP0 SN_SPI0

/*_____ D E C L A R A T I O N S ____________________________________________*/


/*_____ D E F I N I T I O N S ______________________________________________*/


/*_____ M A C R O S ________________________________________________________*/


/*_____ F U N C T I O N S __________________________________________________*/

void SPI0_Flush(void)
{
    while (SN_SSP0->STAT_b.BUSY);
}

void SPI0_Read3(unsigned char b1, unsigned char b2, unsigned char *b3)
{
    /* write first 2 bytes: header and address */
    while (!SN_SSP0->STAT_b.TX_EMPTY);
    SN_SSP0->DATA_b.Data = b1;
    SN_SSP0->DATA_b.Data = b2;

    /* read 1 byte data */
    while (SN_SSP0->STAT_b.BUSY);
    while (SN_SSP0->STAT_b.RX_EMPTY);
    *b3 = SN_SSP0->DATA_b.Data;
         
    while (SN_SSP0->STAT_b.BUSY);
}
