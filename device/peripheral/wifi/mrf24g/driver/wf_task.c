/*******************************************************************************
 MRF24WG run-time task

  Summary: Called from round-robin to check for WiFi events

  Description: Same as summary
*******************************************************************************/

/* MRF24WG0M Universal Driver
*
* Copyright (c) 2012-2013, Microchip <www.microchip.com>
* Contact Microchip for the latest version.
*
* This program is free software; distributed under the terms of BSD
* license:
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1.    Redistributions of source code must retain the above copyright notice, this
*        list of conditions and the following disclaimer.
* 2.    Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
* 3.    Neither the name(s) of the above-listed copyright holder(s) nor the names
*        of its contributors may be used to endorse or promote products derived
*        from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//==============================================================================
//                                  INCLUDES
//==============================================================================
#include "device/dev_wnic.h"

#include "./inc/shared/wf_universal_driver.h"
#include "./inc/internal/wf_global_includes.h"

//==============================================================================
//                                  LOCAL FUNCTION PROTOTYPES
//==============================================================================

static void EventCheck(DEV_WNIC *mrf24g_wnic);
static void PsPollCheck(DEV_WNIC *mrf24g_wnic);

//==============================================================================
//                                  GLOBAL FUNCTION PROTOTYPES
//==============================================================================
extern void ChipResetStateMachine(void);


void WF_Task(DEV_WNIC *mrf24g_wnic, void *arg)
{
    DEV_WNIC_INFO *mrf24g_info;

    mrf24g_info = &(mrf24g_wnic->wnic_info);

    if (mrf24g_info->power_status != WNIC_POWER_OFF) {
        //arg = arg;  // avoid warning
        ChipResetStateMachine();

        EventCheck(mrf24g_wnic);

        InterruptCheck();

        RxPacketCheck(mrf24g_wnic);

        PsPollCheck(mrf24g_wnic);

    }
}

static void EventCheck(DEV_WNIC *mrf24g_wnic)
{
    t_event event;

    // if event queue empty then nothing to do
    if ( isEventQEmpty() ) {
        return;
    }

    // get next event from queue
    EventDequeue(&event);

    // notify application of event
    WF_ProcessEvent(mrf24g_wnic, event.eventType, event.eventData);
}

static void PsPollCheck(DEV_WNIC *mrf24g_wnic)
{
    DEV_WNIC_INFO *mrf24g_info;

    mrf24g_info = &(mrf24g_wnic->wnic_info);

    if (mrf24g_info->conn_status == WNIC_CONNECTED) {
	    // if PS-Poll was disabled temporarily and needs to be reenabled, and, we are in
	    // a connected state
	    if ((isPsPollNeedReactivate()) && (UdGetConnectionState() == CS_CONNECTED)) {
	        ClearPsPollReactivate();
	        WFConfigureLowPowerMode(WF_LOW_POWER_MODE_ON);
	    }
    }

}