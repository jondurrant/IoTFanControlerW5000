/*
 * ExampleAgentObserver.h
 *
 *  Created on: 23 Nov 2021
 *      Author: jondurrant
 */

#ifndef EXAMPLEAGENTOBSERVER_H_
#define EXAMPLEAGENTOBSERVER_H_

#include "MQTTAgentObserver.h"


class ExampleAgentObserver : public  MQTTAgentObserver{
public:
	ExampleAgentObserver();
	virtual ~ExampleAgentObserver();

	virtual void MQTTOffline();

	virtual void MQTTOnline();

	virtual void MQTTSend();

	virtual void MQTTRecv();
};

#endif /* EXAMPLEAGENTOBSERVER_H_ */
