/*
 * constAndinclude.h
 *
 *  Created on: 2017骞�6���22���
 *      Author: pyt64
 */

#ifndef CONSTANDINCLUDE_H_
#define CONSTANDINCLUDE_H_



///////////////////////////////
//complier control
///////////////////////////////
#define CONNECT_TO_MY_SERVER



///////////////////////////////
//const
///////////////////////////////

#ifdef CONNECT_TO_MY_SERVER
#define NETSERVER_ADDR "localhost"
#else
#define NETSERVER_ADDR "192.168.1.10"
#endif


#define UTC_NTP 2208988800U /* 1970 - 1900 */

#define LOGFILE stdout

#define tmpPrint //fprintf

#endif /* CONSTANDINCLUDE_H_ */
