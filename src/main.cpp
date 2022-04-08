

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
//Std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//RP2040
#include "hardware/watchdog.h"
#include "hardware/rtc.h"


//W5000 and FreeRTOS
extern "C" {
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "dhcp.h"
#include "dns.h"

#include "timer.h"
#include "socket.h"
}


//JD Specials
#include <WatchdogBlinkAgent.h>
#include <RGBLEDAgent.h>
#include "OledDisplay.h"

//TwinThing
#include "EthHelper.h"
#include "MQTTAgent.h"
#include "MQTTRouterPing.h"
#include "MQTTRouterTwin.h"

// Fan Control Classes
#include "FanState.h"
#include "RGBLEDMgr.h"
#include "FanController.h"
#include "DisplayAgent.h"
#include "RotEncAgent.h"
#include "RotEncListener.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Definitions
 * ----------------------------------------------------------------------------------------------------
 */
/* Task */
#define DHCP_TASK_STACK_SIZE 2048
#define DHCP_TASK_PRIORITY 8

#define DNS_TASK_STACK_SIZE 512
#define DNS_TASK_PRIORITY 10

/* Clock */
#define PLL_SYS_KHZ (133 * 1000)


/* Socket */
#define SOCKET_DHCP 0
#define SOCKET_DNS 3

/* Retry count */
#define DHCP_RETRY_COUNT 5
#define DNS_RETRY_COUNT 5


#ifndef MQTTHOST
#define MQTTHOST "mqtt.home.com"
#define MQTTPORT 1883
#define MQTTUSER "MAC"
#define MQTTPASSWD "MAC"
#endif


/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/*NTP Servers*/
static const char *sntpHosts[5] = {
		"mqtt.home.com",
		"uk.pool.ntp.org",
		"time.cloudflare.com",
		"ntp2a.mcc.ac.uk",
		"ntp2b.mcc.ac.uk"};

TaskHandle_t initHandle = NULL;

//Global Twin Objects
EthHelper gEth;
MQTTAgent mqttAgent = MQTTAgent(3, &gEth);
MQTTRouterTwin mqttRouter;

FanState state;
TwinTask xTwin;
MQTTPingTask xPing;

RGBLEDAgent ledAgent = RGBLEDAgent(5,3,2);
RGBLEDMgr   ledMgr = RGBLEDMgr(&ledAgent);

OledDisplay display = OledDisplay(14, 15);
DisplayAgent dispAgent = DisplayAgent(&display, &state, &gEth);

FanController fanControl = FanController(&state, 0);

RotEncAgent rotEncAgent = RotEncAgent(12, 11, 10, 20);
RotEncListener rotEncListener;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* Clock */
static void set_clock_khz(void);


/***
 * Print status and stack usage of task
 * @param name
 * @param task
 */
void debugTask(char * name, TaskHandle_t task){
	TaskStatus_t xTaskStatus;
	char ready[]="eReady";
	char run[]="eRunning";
	char block[]="eBlocked";
	char sus[]="eSuspended";
	char del[]="eDeleted";
	char other[]="Unknown";
	char *state = other;


	vTaskGetInfo( task,
	              &xTaskStatus,
				  pdTRUE,
				  (eTaskState)eInvalid);

	switch(xTaskStatus.eCurrentState){
	case eReady:
		state = ready;
		break;
	case eRunning:
		state = run;
		break;
	case eBlocked:
		state = block;
		break;
	case eSuspended:
		state = sus;
		break;
	case eDeleted:
		state = del;
		break;
	default:
		state = other;
	}

	printf("%s: Priority: %d/%d Run: %d Min: %d eState: %s\n",
			name,
			xTaskStatus.uxCurrentPriority,
			xTaskStatus.uxBasePriority,
			xTaskStatus.ulRunTimeCounter,
			xTaskStatus.usStackHighWaterMark,
			state
			);
}


/***
 * Start up the MQTT components
 * @param mqttTarget - Host of MQTT Hub
 * @param mqttPort - Port number
 * @param mqttUser - User
 * @param mqttPwd - Password
 */
void doMQTT(char * mqttTarget, uint16_t mqttPort, char * mqttUser, char * mqttPwd){

	mqttAgent.credentials(mqttUser, mqttPwd);
	mqttRouter.init(mqttAgent.getId(), &mqttAgent);

	//Twin agent to manage the state
	xTwin.setStateObject(&state);
	xTwin.setMQTTInterface(&mqttAgent);
	xTwin.start(tskIDLE_PRIORITY+1);

	//Start up a Ping agent to mange ping requests
	xPing.setInterface(&mqttAgent);
	xPing.start(tskIDLE_PRIORITY+1);

	//Give the router the twin and ping agents
	mqttRouter.setTwin(&xTwin);
	mqttRouter.setPingTask(&xPing);

	//Setup and start the mqttAgent
	mqttAgent.setObserver(&ledMgr);
	mqttAgent.setRouter(&mqttRouter);

	mqttAgent.connect(mqttTarget, 1883, true, false);
	mqttAgent.start(tskIDLE_PRIORITY+2);//DHCP_TASK_PRIORITY);
}


/***
 * Init thread to get everything going and check on status of network
 * @param pvParameters
 */
void
init_thread(void* pvParameters) {
	char mqttTarget[] = MQTTHOST;
	uint16_t mqttPort = MQTTPORT;
	char mqttUser[] = MQTTUSER;
	char mqttPwd[] = MQTTPASSWD;
	uint8_t ip[4];
	bool retval;

	WatchdogBlinkAgent watchdog;
	watchdog.start(tskIDLE_PRIORITY+3);


	if (!ledAgent.start(tskIDLE_PRIORITY+1)){
		LogError(("Led Agent failed to start"));
	} else {
		LogInfo(("Led Agent Started"));
	}
	ledAgent.set(RGBModeOn,0xFF,0x0,0x0);

	ledMgr.setDisplayAgent(&dispAgent);
	dispAgent.start(tskIDLE_PRIORITY+1);

	rotEncAgent.setListener(&dispAgent);
	rotEncAgent.start(tskIDLE_PRIORITY+1);

	gEth.enableMutex();
	gEth.dhcpClient();
	gEth.getIPAddress(ip);
	dispAgent.showIP(ip);
	ledAgent.set(RGBModeOn,0xFF,0xCE,0x42);

	gEth.setSNTPServers(sntpHosts, 5);
	retval = gEth.syncRTCwithSNTP(sntpHosts, 5);
	printf("SNTP Res: %s\n", retval?"Ok":"Fail");

	doMQTT(mqttTarget, mqttPort, mqttUser, mqttPwd);

	uint32_t count=0;
	datetime_t xDate;
    for (;;){
    	vTaskDelay(1000);

    	/*
    	debugTask("ledAgent", ledAgent.getTask());
    	debugTask("watchdog", watchdog.getTask());
    	debugTask("mqttAgent", mqttAgent.getTask());
    	debugTask("xTwin", xTwin.getTask());
    	debugTask("xPing", xPing.getTask());
    	debugTask("init", initHandle);
    	debugTask("dispAgent", dispAgent.getTask());
    	debugTask("rotEncAgent", rotEncAgent.getTask());
    	*/

    	//Check if network has failed
    	if (!gEth.isJoined()){
    		dispAgent.noIP();
    		ledAgent.set(RGBModeOn,0xFF,0x0,0x0);
    		gEth.dhcpClient();
    		if (gEth.isJoined()){
				ledAgent.set(RGBModeOn,0xFF,0xCE,0x42);
				gEth.getIPAddress(ip);
				dispAgent.showIP(ip);
    		}
    	} else {

    		//Once a day renew the DHCP lease
			if (count >= 60*60*24){
				count = 0;

				if (gEth.dhcpClient()){
					printf("DHCP Renewed\n");
				} else {
					printf("DHCP FAILED Renew\n");
				}

				//And update RTC
				retval = gEth.syncRTCwithSNTP(sntpHosts, 5);
				printf("SNTP: %s\n", retval?"Ok":"Fail");
			}
    	}
    	count ++;

    	if (count % 10 == 0){
			state.updateClock();
    	}
    }
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    set_clock_khz();

    stdio_init_all();


    //3second wait to let serial stabalise for debug messages
    sleep_ms(3000);

    if (watchdog_caused_reboot()){
    	printf("WATCHDOG REBOOTED\n");
    	sleep_ms(5000);

    }

    //Init ethernet before running up tasks
    gEth.rtcInit();
    gEth.init(g_ethernet_buf);

    //Nice message on display why waiting for network
    display.displayString("Hello","",2);


    //Launch Init task
    xTaskCreate(
		init_thread,
		"Init task",
		DHCP_TASK_STACK_SIZE,
		( void * ) 1,
		DHCP_TASK_PRIORITY,//tskIDLE_PRIORITY+1,
		&initHandle );

    vTaskStartScheduler();

    //Should never get here
    while (1)
    {
        ;
    }
}


/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}
