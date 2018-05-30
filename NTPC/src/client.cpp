/*
 * client.cpp
 *
 *  Created on: 2017年6月22日
 *      Author: pyt64
 */


#include "include.h"

#include "const.h"

/*
 * 功能：整形转字符串
 * 参数：
 * @int n：待转的整形
 * 返回值：
 * 转换后的字符串（string类型）
 */

/* get Timestamp for NTP in LOCAL ENDIAN */
void gettime64(uint32_t ts[])
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	tmpPrint(LOGFILE,"tv.tv_sec = %d\n",tv.tv_sec);
	tmpPrint(LOGFILE,"tv.tv_usec = %d\n",tv.tv_usec);

	ts[0] = (long)tv.tv_sec + UTC_NTP;
	ts[1] = (4294*(tv.tv_usec)) + ((1981*(tv.tv_usec))>>11);

	tmpPrint(LOGFILE,"ts[0] = %ul\n",ts[0]);
	tmpPrint(LOGFILE,"ts[1] = %ul\n",ts[1]);
}

int die(const char *msg)
{
	fputs(msg, stderr);
	exit(-1);
}

int useage(const char *path)
{
	tmpPrint(LOGFILE,"Useage:\n\t%s <server address> or let it empty.\n", path);
	return 1;
}


int open_connect(const char* server)
{
	int s;
	struct addrinfo *saddr;

	/* tmpPrint(LOGFILE,"Connecting to server: %s\n", server); */
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		die("Can not create socket.\n");
	}

	if (0 != getaddrinfo(server, "123", NULL, &saddr)) {
		die("Server address not correct.\n");
	}

	if (connect(s, saddr->ai_addr, saddr->ai_addrlen) != 0) {
		die("Connect error\n");
	}

	freeaddrinfo(saddr);

	return s;
}

void request(int fd)
{
	unsigned char buf[48] = {0};
	uint32_t tts[2]; /* Transmit Timestamp */

	/* LI VN MODE = 00 100 011*/
	buf[0] = 0x23;

	gettime64(tts);
	*((uint32_t *)&buf[40]) = htonl(tts[0]);
	*((uint32_t *)&buf[44])	= htonl(tts[1]);
	if (send(fd, buf, 48, 0) !=48 ) {
		die("Send error\n");
	}
}

void get_reply(int fd)
{
	unsigned char buf[48];
	uint32_t *pt;
	// uint32_t t_last_update[2]; /* Reference Timestamp @ Server */
	uint32_t t1[2]; /* t1 = Originate Timestamp  */
	uint32_t t2[2]; /* t2 = Receive Timestamp @ Server */
	uint32_t t3[2]; /* t3 = Transmit Timestamp @ Server */
	uint32_t t4[2]; /* t4 = Receive Timestamp @ Client */
	double T1, T2, T3, T4;
	double tfrac = 4294967296.0;
	time_t curr_time;
	time_t diff_sec;

	if (recv(fd, buf, 48, 0) < 48) {
		die("Receive error\n");
	}
	gettime64(t4);
	pt = (uint32_t *)&buf[24];

	t1[0] = htonl(*pt++);
	t1[1] = htonl(*pt++);
	tmpPrint(LOGFILE,"t1[0] = %ul\n",t1[0]);
	tmpPrint(LOGFILE,"t1[1] = %ul\n",t1[1]);

	t2[0] = htonl(*pt++);
	t2[1] = htonl(*pt++);
	tmpPrint(LOGFILE,"t2[0] = %ul\n",t2[0]);
	tmpPrint(LOGFILE,"t2[1] = %ul\n",t2[1]);

	t3[0] = htonl(*pt++);
	t3[1] = htonl(*pt++);
	tmpPrint(LOGFILE,"t3[0] = %ul\n",t3[0]);
	tmpPrint(LOGFILE,"t3[1] = %ul\n",t3[1]);

	/* TODO: überprüfen t1 = Transmit Timestamp @ Client */
	/* und andere Überprüfungen
	 * (z.B Version=4, Mode=Server,
	 * 	Stratum = 0-15, etc.)*/

	T1 = t1[0] + t1[1]/tfrac;
	T2 = t2[0] + t2[1]/tfrac;
	T3 = t3[0] + t3[1]/tfrac;
	T4 = t4[0] + t4[1]/tfrac;

	tmpPrint(LOGFILE, "\ndelay = %lf\n"
		"offset = %lf\n\n",
		(T4-T1) - (T2-T3),
		((T2 - T1) + (T3 - T4)) /2
	      );
	/* wenn mit Ganzzahl rechnen, kann sein,
	 * dass die Differenz negativ ist */
	//diff_sec = ((int32_t)(t2[0] - t1[0]) + (int32_t)(t3[0] - t4[0])) /2;
	diff_sec = ((int64_t)(t2[0] - t1[0]) + (int64_t)(t3[0] - t4[0])) /2;
	tmpPrint(LOGFILE,"diff_sec = %d\n",diff_sec);
	curr_time = time(NULL) + diff_sec;
	tmpPrint(LOGFILE,"now_time = %ul\n",time(NULL));
	tmpPrint(LOGFILE,"server_time = %ul\n",curr_time);
	char order[256];
	tmpPrint(LOGFILE,"Current Time at Server:   %s\n", ctime(&curr_time));
	tm *tmp = gmtime(&curr_time);
	sprintf(order,"date -s \"%4d-%02d-%02d %02d:%02d:%02d\"\n", tmp->tm_year+1900,tmp->tm_mon+1,tmp->tm_mday,tmp->tm_hour,tmp->tm_min,tmp->tm_sec);
	tmpPrint(LOGFILE,"%s",order);
	system(order);
}

int client(const char* server)
{
	int fd;

	fd = open_connect(server);
	request(fd);
	get_reply(fd);
	close(fd);

	return 0;
}


int main(int argc, char *argv[], char **env)
{

	if (2 == argc) {
		return client(argv[1]);
	}else if(1 == argc) {
		return client(NETSERVER_ADDR);
	}else
		return useage(argv[0]);
}
