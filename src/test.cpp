/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include "hardware/rtc.h"

#include "EthHelper.h"
#include <WatchdogBlinkAgent.h>


#include "MQTTAgent.h"

#include "MQTTRouterPing.h"
#include "MQTTRouterTwin.h"

#include "StateExample.h"
#include "ExampleAgentObserver.h"


#include "TCPTransport.h"

#include "hardware/watchdog.h"

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

EthHelper gEth;

static const char *sntpHosts[5] = {
		"nas3.local.jondurrant.com",
		"0.uk.pool.ntp.org",
		"1.uk.pool.ntp.org",
		"ntp2a.mcc.ac.uk",
		"ntp2b.mcc.ac.uk"};


char strBuf[2048];

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Task */
void dhcp_task(void *argument);
void dns_task(void *argument);

/* Clock */
static void set_clock_khz(void);

/* DHCP */
static void wizchip_dhcp_init(void);
static void wizchip_dhcp_assign(void);
static void wizchip_dhcp_conflict(void);

/* Timer  */
static void repeating_timer_callback(void);

MQTTAgent mqttAgent = MQTTAgent(3, &gEth);
MQTTRouterTwin mqttRouter;
//MQTTRouterPing mqttRouter;
StateExample state;
ExampleAgentObserver agentObs;
TwinTask xTwin;
MQTTPingTask xPing;

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



void doTCPTransport(){
	uint8_t sockNum = 0;
	int8_t res;
	uint8_t ip[4];
	int32_t dataSend, dataOut, dataIn;
	char httpGet[] = "GET / HTTP/1.1\nHost: piudev2.local.jondurrant.com\n\n";
	uint16_t port = 1080;
	uint8_t step=0, count=0;
	bool dataReady = false;

	TCPTransport trans(sockNum, &gEth);
	NetworkContext_t net;
	net.tcpTransport = (void *)&trans;

	if (trans.transConnect((char *)g_mqtt_domain, port)){
		if (trans.transSend(&net, httpGet, strlen(httpGet))<= 0){
			printf("Send failed\n");
			return;
		}
		for (uint8_t i=0; i < 30; i++){
			if (dataReady){
				printf("Read MAX: ");
				dataIn = trans.transRead(&net, strBuf, ETHERNET_BUF_MAX_SIZE);
				dataReady = false;
			} else {
				printf("Read 1: ");
				dataIn = trans.transRead(&net, strBuf, 1);
				if (dataIn == 1){
					dataReady = true;
				}
			}
			if (dataIn >=0){
				printf("Step: %d dataIn: %d\n",i, dataIn);
				strBuf[dataIn]=0;
				printf("->%s\n", strBuf);
			} else {
				printf("READ ERROR %d\n", dataIn);
			}
			vTaskDelay(10);

		}
		trans.transClose();
	}

}


void doTCPRead(){
	uint8_t sock = 0;
	int8_t res;
	uint8_t ip[4];
	uint32_t dataSend, dataOut, dataIn;
	char httpGet[] = "GET / HTTP/1.1\nHost: piudev2.local.jondurrant.com\n\n";
	uint16_t port = 1080;
	uint8_t step=0, count=0;


	res = socket(sock, Sn_MR_TCP, 1080, SF_TCP_NODELAY);//SF_TCP_NODELAY | SF_IO_NONBLOCK  );
	if (res != sock){
		printf("Socket failed %d\n",res);
		switch(res){
		case SOCKERR_SOCKNUM:{
			printf("Socket SOCKERR_SOCKNUM %d\n",res);
			break;
		}
		case SOCKERR_SOCKMODE:{
			printf("Socket SOCKERR_SOCKMODE %d\n",res);
			break;
		}
		case SOCKERR_SOCKFLAG:{
			printf("Socket SOCKERR_SOCKFLAG %d\n",res);
			break;
		}
		default: {
			printf("Socket failed %d\n",res);
		}
		}
		return;
	}

	//res = ctlsocket(sock ,CS_SET_IOMODE, SOCK_IO_NONBLOCK);

	if (!gEth.dnsClient(ip, (char *)g_mqtt_domain)){
		printf("Socket DNS failed \n");
		return;
	}

	res = connect(sock, ip, port);

	if (res != SOCK_OK){
		printf("Socket connect to %d.%d.%d.%d:%d failed %d\n",ip[0],ip[1], ip[2], ip[3], port, res);
		return;
	}


	dataSend = strlen(httpGet);
	dataOut = send(sock, (uint8_t *)httpGet, dataSend);
	if (dataOut != dataSend){
		printf("Send failed %d\n", dataOut);
	}

	uint8_t status;
	uint16_t remaining;
	while(1){
		getsockopt(sock,SO_STATUS, &status);
		getsockopt(sock,SO_REMAINSIZE, &remaining);
		printf("SOCKET STATUS %d, Remaining %d\n", status, remaining);
		switch(status){
		case SOCK_CLOSED:{
			printf("SOCK_CLOSED\n");
			break;
		}
		case SOCK_INIT:{
			printf("SOCK_INIT\n");
			break;
		}
		case SOCK_LISTEN:{
			printf("SOCK_INIT\n");
			break;
		}
		case SOCK_ESTABLISHED:{
			printf("SOCK_ESTABLISHED\n");
			break;
		}
		case SOCK_CLOSE_WAIT:{
			printf("SOCK_CLOSE_WAIT\n");
			break;
		}
		case SOCK_UDP:{
			printf("SOCK_UDP\n");
			break;
		}
		case SOCK_MACRAW:{
			printf("SOCK_MACRAW\n");
			break;
		}
		case SOCK_SYNSENT:{
			printf("SOCK_SYNSENT\n");
			break;
		}
		case SOCK_SYNRECV:{
			printf("SOCK_SYNRECV\n");
			break;
		}
		case SOCK_FIN_WAIT:{
			printf("SOCK_FIN_WAIT\n");
			break;
		}
		case SOCK_CLOSING:{
			printf("SOCK_CLOSING\n");
			break;
		}
		case SOCK_TIME_WAIT:{
			printf("SOCK_TIME_WAIT\n");
			break;
		}
		case SOCK_LAST_ACK:{
			printf("SOCK_LAST_ACK\n");
			break;
		}

		}


		switch(step){
		case 0:{
			printf("Attempt read 1\n");
			dataIn = recv(sock, g_ethernet_buf, 1);
			break;
		}
		case 1:{
			printf("Attempt read MAX\n");
			dataIn = recv(sock, g_ethernet_buf, ETHERNET_BUF_MAX_SIZE);
			break;
		}
		case 2:{
			printf("Rerequest \n");
			dataOut = send(sock, (uint8_t *)httpGet, dataSend);
			if (dataOut != dataSend){
				printf("Send failed %d\n", dataOut);
			}
			dataIn = 0;
			step = 0;
		}
		}

		switch(dataIn){
		case SOCKERR_SOCKSTATUS:{
			printf("SOCKERR_SOCKSTATUS\n");
			disconnect(sock);
			return;
			break;
		}
		case SOCKERR_SOCKMODE:{
			printf("SOCKERR_SOCKMODE\n");
			disconnect(sock);
			return;
			break;

		}
		case SOCKERR_SOCKNUM:{
			printf("SOCKERR_SOCKNUM\n");
			disconnect(sock);
			return;
			break;
		}
		case SOCKERR_DATALEN:{
			printf("SOCKERR_DATALEN\n");
			break;
		}
		case SOCK_BUSY:{
			printf("SOCK_BUSY\n");
			break;
		}
		default:{
			if (dataIn > 0){
				step++;
				printf("SockRead %d\n", dataIn);
				g_ethernet_buf[dataIn]=0;
				printf("SockRead %s\n", (char*)g_ethernet_buf);

				count ++;
				if (count > 5){
					disconnect(sock);
					return;
				}
			} else {
				printf("Socket Error %d\n", dataIn);
			}
		}

		}

		vTaskDelay(10);
	}

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


	/*
	MQTTRouterTwin mqttRouter;
	StateExample state;
	ExampleAgentObserver agentObs;

	MQTTAgent mqttAgent(512, 512);
	TwinTask xTwin;
	MQTTPingTask xPing;
	*/

	WatchdogBlinkAgent watchdog;
	watchdog.start(tskIDLE_PRIORITY+1);


	gEth.dhcpClient();


	/*
	//Set up the credentials so we have an ID for our thing
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
	mqttAgent.setObserver(&agentObs);
	mqttAgent.setRouter(&mqttRouter);
	mqttAgent.connect(mqttTarget, mqttPort, true, true);
	mqttAgent.start(tskIDLE_PRIORITY+1);
	*/


	retval=gEth.syncRTCwithSNTP(sntpSvr);
	printf("SNTP IP: %s\n", retval?"Ok":"Fail");


	//doTCPRead();
	//doTCPTransport();
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


/*
    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();
*/
    //wizchip_dhcp_init();

    gEth.init(g_ethernet_buf);

    //wizchip_1ms_timer_initialize(repeating_timer_callback);

    //xTaskCreate(dhcp_task, "DHCP_Task", DHCP_TASK_STACK_SIZE, NULL, DHCP_TASK_PRIORITY, NULL);
    //xTaskCreate(dns_task, "DNS_Task", DNS_TASK_STACK_SIZE, NULL, DNS_TASK_PRIORITY, NULL);

    dns_sem = xSemaphoreCreateCounting((unsigned portBASE_TYPE)0x7fffffff, (unsigned portBASE_TYPE)0);



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

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
void doDNS(){
	 DNS_init(SOCKET_DNS, g_ethernet_buf);
	 if (DNS_run(g_net_info.dns, g_mqtt_domain, g_dns_target_ip) > 0)
	 {
		 printf(" JON DNS success\r\n");
		 printf(" Target domain : %s\n", g_mqtt_domain);
		 printf(" IP of target domain : %d.%d.%d.%d\n", g_dns_target_ip[0], g_dns_target_ip[1], g_dns_target_ip[2], g_dns_target_ip[3]);
		 printf("-----------------------------------\n");
	 }
}



/* Task */
void dhcp_task(void *argument)
{
    int retval = 0;
    uint8_t link;
    uint16_t len = 0;
    uint32_t dhcp_retry = 0;

    if (g_net_info.dhcp == NETINFO_DHCP) // DHCP
    {
        wizchip_dhcp_init();
    }
    else // static
    {
        network_initialize(g_net_info);

        /* Get network information */
        print_network_information(g_net_info);

        while (1)
        {
            vTaskDelay(1000 * 1000);
        }
    }

    while (1)
    {
        link = wizphy_getphylink();

        if (link == PHY_LINK_OFF)
        {
            printf("PHY_LINK_OFF\r\n");

            DHCP_stop();

            while (1)
            {
                link = wizphy_getphylink();

                if (link == PHY_LINK_ON)
                {
                    wizchip_dhcp_init();

                    dhcp_retry = 0;

                    break;
                }

                vTaskDelay(1000);
            }
        }

        retval = DHCP_run();

        if (retval == DHCP_IP_LEASED)
        {
            if (g_dhcp_get_ip_flag == 0)
            {
                dhcp_retry = 0;

                printf(" DHCP success\n");

                g_dhcp_get_ip_flag = 1;

                doDNS();


                /*
                TaskHandle_t atHandle = NULL;
				xTaskCreate(
					init_thread,
					"Init task",
					512,
					( void * ) 1,
					tskIDLE_PRIORITY+1,
					&atHandle );
				*/

                xSemaphoreGive(dns_sem);
            }
        }
        else if (retval == DHCP_FAILED)
        {
            g_dhcp_get_ip_flag = 0;
            dhcp_retry++;

            if (dhcp_retry <= DHCP_RETRY_COUNT)
            {
                printf(" DHCP timeout occurred and retry %d\n", dhcp_retry);
            }
        }

        if (dhcp_retry > DHCP_RETRY_COUNT)
        {
            printf(" DHCP failed\n");

            DHCP_stop();

            while (1)
            {
                vTaskDelay(1000 * 1000);
            }
        }

        vTaskDelay(10);
    }
}



void dns_task(void *argument)
{
    uint8_t dns_retry;

    while (1)
    {
        xSemaphoreTake(dns_sem, portMAX_DELAY);
        DNS_init(SOCKET_DNS, g_ethernet_buf);

        dns_retry = 0;

        while (1)
        {
            if (DNS_run(g_net_info.dns, g_dns_target_domain, g_dns_target_ip) > 0)
            {
                printf(" DNS success\r\n");
                printf(" Target domain : %s\n", g_dns_target_domain);
                printf(" IP of target domain : %d.%d.%d.%d\n", g_dns_target_ip[0], g_dns_target_ip[1], g_dns_target_ip[2], g_dns_target_ip[3]);

                break;
            }
            else
            {
                dns_retry++;

                if (dns_retry <= DNS_RETRY_COUNT)
                {
                    printf(" DNS timeout occurred and retry %d\r\n", dns_retry);
                }
            }

            if (dns_retry > DNS_RETRY_COUNT)
            {
                printf(" DNS failed\r\n");

                break;
            }

            vTaskDelay(10);
        }
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

/* DHCP */
static void wizchip_dhcp_init(void)
{
    printf(" DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);

    g_dhcp_get_ip_flag = 0;
}

static void wizchip_dhcp_assign(void)
{
    getIPfromDHCP(g_net_info.ip);
    getGWfromDHCP(g_net_info.gw);
    getSNfromDHCP(g_net_info.sn);
    getDNSfromDHCP(g_net_info.dns);

    g_net_info.dhcp = NETINFO_DHCP;

    /* Network initialize */
    network_initialize(g_net_info); // apply from DHCP

    print_network_information(g_net_info);
    printf(" DHCP leased time : %ld seconds\n", getDHCPLeasetime());
}

static void wizchip_dhcp_conflict(void)
{
    printf(" Conflict IP from DHCP\n");

    // halt or reset or any...
    while (1)
    {
        vTaskDelay(1000 * 1000);
    }
}

/* Timer */
static void repeating_timer_callback(void)
{
    g_msec_cnt++;

    if (g_msec_cnt >= 1000 - 1)
    {
        g_msec_cnt = 0;

        DHCP_time_handler();
        DNS_time_handler();
    }
}
