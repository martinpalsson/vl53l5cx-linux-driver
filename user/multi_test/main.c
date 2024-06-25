/*******************************************************************************
Copyright (C) 2015, STMicroelectronics International N.V.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of STMicroelectronics nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED.
IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>

#include <stdio.h>
#include <string.h>

#include "vl53l5cx_api.h"


int main(int argc, char ** argv)
{
	int32_t status_i32;
	uint8_t status, loop, isAlive, isReadyLeft, isReadyRight, i;
	VL53L5CX_ResultsData ResultsLeft, ResultsRight;

	VL53L5CX_Configuration 	DevLeft, DevRight;

	/*********************************/
	/*   Power on sensor and init    */
	/*********************************/

	/* Device file e.g. /dev/stmvl53l5cx1 */
	DevLeft.platform.devPath = "/dev/stmvl53l5cx2";
	DevRight.platform.devPath = "/dev/stmvl53l5cx1";
	/* Initialize channel com */
	status_i32 = vl53l5cx_comms_init(&DevLeft.platform);
	if(status_i32)
	{
		printf("VL53L5CX comms init failed\n");
		return -1;
	}
	status_i32 = vl53l5cx_comms_init(&DevRight.platform);
	if(status_i32)
	{
		printf("VL53L5CX comms init failed\n");
		return -1;
	}

	/* (Optional) Check if there is a VL53L5CX sensor connected */
	status = vl53l5cx_is_alive(&DevLeft, &isAlive);
	if(!isAlive || status)
	{
		printf("VL53L5CX not detected at requested address\n");
		return status;
	}
	status = vl53l5cx_is_alive(&DevRight, &isAlive);
	if(!isAlive || status)
	{
		printf("VL53L5CX not detected at requested address\n");
		return status;
	}

	/* (Mandatory) Init VL53L5CX sensor */
	status = vl53l5cx_init(&DevLeft);
	if(status)
	{
		printf("VL53L5CX ULD Loading failed\n");
		return status;
	}

	status = vl53l5cx_init(&DevRight);
	if(status)
	{
		printf("VL53L5CX ULD Loading failed\n");
		return status;
	}

	printf("VL53L5CX ULD ready ! (Version : %s)\n",
			VL53L5CX_API_REVISION);

	/*********************************/
	/*         Ranging loop          */
	/*********************************/

	status = vl53l5cx_start_ranging(&DevLeft);
	status = vl53l5cx_start_ranging(&DevRight);

	loop = 0;
	while(loop < 10)
	{
		/* Use polling function to know when a new measurement is ready.
		 * Another way can be to wait for HW interrupt raised on PIN A3
		 * (GPIO 1) when a new measurement is ready */
 
		isReadyLeft |= wait_for_dataready(&DevLeft.platform);
		isReadyRight |= wait_for_dataready(&DevRight.platform);

		if(isReadyLeft && isReadyRight)
		{
			vl53l5cx_get_ranging_data(&DevLeft, &ResultsLeft);
			vl53l5cx_get_ranging_data(&DevRight, &ResultsRight);

			/* As the sensor is set in 4x4 mode by default, we have a total 
			 * of 16 zones to print. For this example, only the data of first zone are 
			 * print */
			printf("Print data left no : %3u; data right no: %3u\n", DevLeft.streamcount, DevRight.streamcount);
			for(i = 0; i < 16; i++)
			{
				printf("Zone : %3d, Status : %3u, Distance : %4d mm ; Status : %3u, Distance : %4d mm\n",
					i,
					ResultsLeft.target_status[VL53L5CX_NB_TARGET_PER_ZONE*i],
					ResultsLeft.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*i],
					ResultsRight.target_status[VL53L5CX_NB_TARGET_PER_ZONE*i],
					ResultsRight.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*i]
					);
			}
			printf("\n");
			loop++;

			isReadyLeft = isReadyRight = 0;
		}
	}

	status = vl53l5cx_stop_ranging(&DevLeft);

	/*********************************/
	/*        FREE the sensor        */
	/*********************************/
	vl53l5cx_comms_close(&DevLeft.platform);
	vl53l5cx_comms_close(&DevRight.platform);

	return 0;
}
