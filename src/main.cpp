

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

//TwinThing
#include "EthHelper.h"
#include "MQTTAgent.h"
#include "MQTTRouterPing.h"
#include "MQTTRouterTwin.h"
//#include "TCPTransport.h"

// Fan Control Classes
#include "StateExample.h"
#include "ExampleAgentObserver.h"
#include "FanState.h"


/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Task */
#define DHCP_TASK_STACK_SIZE 2048
#define DHCP_TASK_PRIORITY 8

#define DNS_TASK_STACK_SIZE 512
#define DNS_TASK_PRIORITY 10

/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_DHCP 0
#define SOCKET_DNS 3

/* Retry count */
#define DHCP_RETRY_COUNT 5
#define DNS_RETRY_COUNT 5



#ifndef MQTTHOST
#define MQTTHOST "piudev2.local.jondurrant.com"
#define MQTTPORT 1883
#define MQTTUSER "nob"
#define MQTTPASSWD "nob"
#endif


/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .dhcp = NETINFO_DHCP                         // DHCP enable/disable
};
static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/* DHCP */
static uint8_t g_dhcp_get_ip_flag = 0;

/* DNS */
static uint8_t g_dns_target_domain[] = "www.wiznet.io";
static uint8_t g_mqtt_domain[] = "piudev2.local.jondurrant.com";
static uint8_t g_dns_target_ip[4] = {
    0,
};
static uint8_t g_dns_get_ip_flag = 0;

/* Semaphore */
//static xSemaphoreHandle dns_sem = NULL;
static SemaphoreHandle_t dns_sem = NULL;

/* Timer  */
static volatile uint32_t g_msec_cnt = 0;



static const char *sntpHosts[5] = {
		"nas3.local.jondurrant.com",
		"0.uk.pool.ntp.org",
		"1.uk.pool.ntp.org",
		"ntp2a.mcc.ac.uk",
		"ntp2b.mcc.ac.uk"};


char strBuf[2048];


//Global Twin Objects
EthHelper gEth;
MQTTAgent mqttAgent = MQTTAgent(3, &gEth);
MQTTRouterTwin mqttRouter;
//MQTTRouterPing mqttRouter;
//StateExample state;
FanState state;
ExampleAgentObserver agentObs;
TwinTask xTwin;
MQTTPingTask xPing;



/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* Clock */
static void set_clock_khz(void);

/* DHCP */
static void wizchip_dhcp_init(void);
static void wizchip_dhcp_assign(void);
static void wizchip_dhcp_conflict(void);

/* Timer  */
static void repeating_timer_callback(void);

void doMQTT(){
	//MQTTAgent agent(0, &gEth);
	mqttAgent.credentials("nob", "nob", "nob");
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
	mqttAgent.setObserver(&agentObs);
	mqttAgent.setRouter(&mqttRouter);

	mqttAgent.connect("piudev2.local.jondurrant.com", 3881, true, false);
	mqttAgent.start(tskIDLE_PRIORITY+1);//DHCP_TASK_PRIORITY);
}


void
init_thread(void* pvParameters) {
	char mqttTarget[] = MQTTHOST;
	uint16_t mqttPort = MQTTPORT;
	char mqttUser[] = MQTTUSER;
	char mqttPwd[] = MQTTPASSWD;
	uint8_t ip[4];
	uint8_t sntpSvr[] = {192, 168, 1, 20};//{80, 86, 38, 193};
	bool retval;
	char host[] = "nas3.local.jondurrant.com";

	WatchdogBlinkAgent watchdog;
	watchdog.start(tskIDLE_PRIORITY+1);


	gEth.dhcpClient();

	retval=gEth.syncRTCwithSNTP(sntpSvr);
	printf("SNTP IP: %s\n", retval?"Ok":"Fail");

	doMQTT();

	uint16_t count=0;
	datetime_t xDate;
    for (;;){
    	vTaskDelay(1000);

    	if (!gEth.isJoined()){
    		//mqttAgent.stop();
    		gEth.dhcpClient();
    		//mqttAgent.start(tskIDLE_PRIORITY+1);//DHCP_TASK_PRIORITY);
    	} else {

			if (count >= 300){
				count = 0;
				if (gEth.dnsClient(ip, mqttTarget)) {
					printf("-----------------------------------\n");
					printf(" JON DNS success\r\n");
					printf(" Target domain : %s\n", mqttTarget);
					printf(" IP of target domain : %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
					printf("-----------------------------------\n");
				} else {
					printf("DNS Failed\n");
				}

				//retval = gEth.syncRTCwithSNTP(host);
				retval = gEth.syncRTCwithSNTP(sntpHosts, 5);
				printf("SNTP HOST: %s\n", retval?"Ok":"Fail");
			}
    	}
    	count ++;

    	if (count % 10 == 0){
			rtc_get_datetime (&xDate);
			printf("RTC: %d-%d-%d, %d:%d:%d\n", xDate.year, xDate.month, xDate.day, xDate.hour, xDate.min, xDate.sec);
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


    if (watchdog_caused_reboot()){
    	printf("WATCHDOG REBOOTED\n");
    	sleep_ms(5000);

    }

    gEth.init(g_ethernet_buf);

    TaskHandle_t atHandle = NULL;

	xTaskCreate(
		init_thread,
		"Init task",
		DHCP_TASK_STACK_SIZE,
		( void * ) 1,
		DHCP_TASK_PRIORITY,//tskIDLE_PRIORITY+1,
		&atHandle );

    vTaskStartScheduler();

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
