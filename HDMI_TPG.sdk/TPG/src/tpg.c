/*
 * tpg.c
 *
 *  Created on: Jul 2, 2017
 *      Author: Karthik Poduval <kpoduval@gmail.com>
 *
 *
 */
#include "xv_tpg.h"
#include "xaxivdma.h"
#include "xparameters.h"
#include "display_ctrl.h"


#include "xil_types.h"
#include "xil_cache.h"
#include "timer_ps.h"
#include "xuartps_hw.h"

/*
 * Device instance definitions
 */
XAxiVdma AxiVdma;
XV_tpg     tpg;



int tpg_initialize(void)
{
/* initialize TPG */
	int Status=0;

	Status = XV_tpg_Initialize(&tpg, XPAR_V_TPG_0_DEVICE_ID);
	  if(Status != XST_SUCCESS) {
	    xil_printf("ERR:: TPG device not found\r\n");
	    return(XST_FAILURE);
	  } else {

		  xil_printf("KVPZ: TPG initialized Hurray\r\n");
	  }

	  return Status;

}


void tpg_configure(int width, int height, XTpg_PatternId pattern)
{
  //Stop TPG
  XV_tpg_DisableAutoRestart(&tpg);

  XV_tpg_Set_height(&tpg, height);
  XV_tpg_Set_width(&tpg, width );
  XV_tpg_Set_colorFormat(&tpg, 0x0); /** 0x0 RGB video */
  //XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_CHECKER_BOARD);
  XV_tpg_Set_bckgndId(&tpg, pattern);
  XV_tpg_Set_ovrlayId(&tpg, 1);

}

void tpg_start(void)
{
  //Start TPG
  XV_tpg_EnableAutoRestart(&tpg);
  XV_tpg_Start(&tpg);
  xil_printf("INFO: TPG started\r\n");
}


void tpg_vmda_start(UINTPTR WriteFrameAddr1, UINTPTR WriteFrameAddr2, UINTPTR WriteFrameAddr3, int height, int width)
{

	   //S2MM
	   XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0x30, 0x4); //reset
	   //XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0x30, 0x88); //genlock

	   XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0xAC,   WriteFrameAddr1);
	   XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0xAC+4, WriteFrameAddr2);
	   XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0xAC+8, WriteFrameAddr3);
	   XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0xA4, width*3);
	   //XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0xA4, 640);
	   //XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0xA8, 0x01002000);
	   XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0xA8, width*4);
	   XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0x30, 0x3);
	   XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0xA0, height);
}

void tpg_stop(void)
{

	//Stop TPG
	  XV_tpg_DisableAutoRestart(&tpg);
}

void tpg_vmda_stop(void)
{

	XAxiVdma_WriteReg(XPAR_AXI_VDMA_2_BASEADDR, 0x30, 0x0); //stop
}


void tpg_menu_loop(DisplayCtrl *dispPtr, u32 uartAddr)
{

	char userInput = 0;


	tpg_initialize();
	tpg_configure(dispPtr->vMode.width, dispPtr->vMode.height, XTPG_BKGND_TARTAN_COLOR_BARS);
	tpg_start();
	tpg_vmda_start(dispPtr->framePtr[0], dispPtr->framePtr[1],dispPtr->framePtr[2],dispPtr->vMode.width, dispPtr->vMode.height);
	TimerDelay(500000);

	/* Flush UART FIFO */
	while (XUartPs_IsReceiveData(uartAddr))
	{
		XUartPs_ReadReg(uartAddr, XUARTPS_FIFO_OFFSET);
	}

	while (userInput != 'q')
	{
		tpg_demo_print_menu(dispPtr);



		/* Wait for data on UART */
		while (!XUartPs_IsReceiveData(uartAddr))
		{}

		/* Store the first character in the UART recieve FIFO and echo it */
		userInput = XUartPs_ReadReg(uartAddr, XUARTPS_FIFO_OFFSET);
		xil_printf("%c", userInput);
		tpg_stop();

		switch(userInput)
		{

		case '1':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_V_RAMP);
				break;

		case '2':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_TEMPORAL_RAMP);
				break;

		case '3':
			XV_tpg_Set_ovrlayId(&tpg, XTPG_BKGND_SOLID_RED);
				break;
		case '4':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_COLOR_BARS);
				break;
		case '5':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_ZONE_PLATE);
				break;
		case '6':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_TARTAN_COLOR_BARS);
				break;

		case '7':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_CROSS_HATCH);
				break;
		case '8':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_RAINBOW_COLOR);
				break;
		case '9':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_CHECKER_BOARD);
				break;

		case 'a':
				XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_PBRS);
				break;

		case 'q':
			tpg_vmda_stop();
			tpg_stop();
			return;
		default:

		TimerDelay(500000);
		}
		tpg_start();

	}

}

void tpg_demo_print_menu(DisplayCtrl *dispPtr)
{
	xil_printf("\x1B[H"); //Set cursor to top left of terminal
	xil_printf("\x1B[2J"); //Clear terminal
	xil_printf("**************************************************\n\r");
	xil_printf("*           ZYBO TPG User Demo               *\n\r");
	xil_printf("**************************************************\n\r");
	xil_printf("*Port: %42s*\n\r", (dispPtr->fHdmi == DISPLAY_HDMI) ? "HDMI" : "VGA");
	xil_printf("*Current Resolution: %28s*\n\r", dispPtr->vMode.label);
	printf("*Pixel Clock Freq. (MHz): %23.3f*\n\r", (dispPtr->fHdmi == DISPLAY_HDMI) ? (dispPtr->pxlFreq / 5.0) : (dispPtr->pxlFreq));
	xil_printf("**************************************************\n\r");
	xil_printf("\n\r");
	xil_printf("0 - H ramp %s\n\r");
	xil_printf("1 - V ramp %s\n\r");
	xil_printf("2 - temporal RAMP \n\r");
	xil_printf("3 - solid red \n\r");
	xil_printf("4 - color bars \n\r");
	xil_printf("5 - zone plate \n\r");
	xil_printf("6 - tartan color bars \n\r");
	xil_printf("7 - cross hatch \n\r");
	xil_printf("8 - rainbow color \n\r");
	xil_printf("9 - checker board \n\r");
	xil_printf("a - pbrs \n\r");
		xil_printf("q - Quit (go back to software patterns)\n\r");
	xil_printf("\n\r");
	xil_printf("Select a new test pattern:");
}



