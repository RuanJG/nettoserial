#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
int o2o_debug = 0;
#define log(format, ...) if(o2o_debug==1) printf(format, ## __VA_ARGS__)
#define msleep(x) usleep(x*1000) 

#define O2OIP "61.143.38.63"
#define O2OPORT 80
#define ROUTE_4G_IP "192.168.1.1"
#define ROUTE_4G_PORT 80

#define O2O_USR_MSG_STRING_LEN 50
#define O2O_IP_MSG_STRING_LEN 20
#define O2O_IOL_MSG_STRING_LEN 2
#define O2O_MAC_MSG_STRING_LEN 20
#define O2O_OLT_MSG_STRING_LEN 22

#define UNVAILD_IP "0.0.0.0"
#define UPDATE_INTERVEL_TIME 8000 //ms
typedef struct __o2o_service_data{
	short is_user_msg_update ;
	short is_ip_update ;
	struct timeval last_update_tv;

	char username[O2O_USR_MSG_STRING_LEN];
	char pwd[O2O_USR_MSG_STRING_LEN];
	char id[O2O_USR_MSG_STRING_LEN];
	char ip[O2O_IP_MSG_STRING_LEN];
	int iol;
	char mac[O2O_MAC_MSG_STRING_LEN];
	char olt[O2O_OLT_MSG_STRING_LEN];
}o2o_data_t;

o2o_data_t o2o_g_data;
int o2o_service_need_quite = 0;
pthread_mutex_t mutex;

int find_sub_string_index(char *str, char *substr)
{
	int i;
	for( i = 0; i< strlen(str); i++){
		if( 0 == strncmp(&str[i],substr,strlen(substr)) )
			return i;
	}
	return -1;
}
int is_fd_ready(int fd,int timeout_ms);

int get_client_socket(char * ip, int port)
{
	int sockfd;
        struct sockaddr_in servaddr;
 
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
                log("创建网络连接失败,本线程即将终止---socket error!\n");
		return -1;
        };
 
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0 ){
                log("set socket addr error -inet_pton error!\n");
                return -1;
        };
 
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
                log("连接到服务器失败,connect error!\n");
		return -1;
        }
        //log("与远端建立了连接\n");

	return sockfd;
}
int send_o2oc_http_cmd(int sockfd,char * cmd)
{
	char str1[4096];
	int ret;

        memset(str1, 0, 4096);
        strcat(str1, cmd);
        strcat(str1, "Host: ");
	strcat(str1, O2OIP);
	strcat(str1, "\r\n");
        //strcat(str1, "Host: 61.143.38.63\r\n");
        //strcat(str1, "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:31.0) Gecko/20100101 Firefox/31.0\r\n");
        strcat(str1, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
        //strcat(str1, "Accept-Language: en-US,en;q=0.5\r\n");
        //strcat(str1, "Accept-Encoding: gzip, deflate\r\n");
        strcat(str1, "Cache-Control: no-cache\r\n");
	strcat(str1,"Connection: close\r\n");
        strcat(str1, "\r\n");

        log("%s\n",str1);
 
        ret = write(sockfd,str1,strlen(str1));
        if (ret < 0) {
                log("发送失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));
		return  -1;
        }
	return 0;
}
int recive_socket_block(int sockfd,char * buffer, int buff_len, int max_ms_delay)
{//-1 error , > 0 has data ; =0 time out
	int ret;
	int len=0;
	int total_len=0;
	int ms = 0;

        memset(buffer, 0, buff_len);
        do{
		ret = is_fd_ready(sockfd,300);
		ms+=300;
 
                if (ret < 0) {
                        log("在读取数据报文时SELECT检测到异常，该异常导致线程终止！\n");
                        return -1;
                };
 
                if (ret > 0){
                        len= read(sockfd, &buffer[total_len], buff_len-total_len);
                        if (len<=0){
                                //log("读取数据报文时发现远端关闭！\n");
                                return total_len;
                        }else{
				total_len += len;
				if( total_len > buff_len ){
					log("recive buff is too small\n");
					return -1;
				}
				;//break;
			}
                }
        }while(ms<max_ms_delay);

	if( ms >= max_ms_delay){
		log("recive_socket_block timeout \n");
		return 0;
	}
	return total_len;
}
int get_wan_ip(char *wip)
{
	char str1[4096];
	char buff[4096];
	int ret,i,j;

        int sockfd;
        struct sockaddr_in servaddr;

	sockfd = get_client_socket(ROUTE_4G_IP,ROUTE_4G_PORT);
	if( sockfd < 0 ) return -1;

        memset(str1, 0, 4096);
        strcat(str1, "POST /api/user/login HTTP/1.1\r\n");
        strcat(str1, "Host: 192.168.1.1\r\n");
        strcat(str1, "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n");
        strcat(str1, "X-Requested-With: XMLHttpRequest\r\n");
        strcat(str1, "Referer: http://192.168.1.1/html/home.html\r\n");
        strcat(str1, "Accept: */*\r\n");
        strcat(str1, "Content-Length: 112\r\n");
        strcat(str1, "Connection:close\r\n");
        strcat(str1, "Pragma: no-cache\r\n");
        strcat(str1, "\r\n");
        strcat(str1, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><request><Username>admin</Username><Password>YWRtaW4=</Password></request>");
        ret = write(sockfd,str1,strlen(str1));
        if (ret < 0) {
                log("发送失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));
                goto err_out;
        }

	ret = recive_socket_block(sockfd,buff,4096,10000);
	if( 0 >= ret){
                log("recevie data error\n");
		goto err_out;
		
	}
	//log("%s\n",buff);
/*
HTTP/1.1 200 OK
Date: Thu, 01 Jan 1970 00:00:00 GMT
Server: mini_httpd/1.19 19dec2003
Connection: close
Cache-Control: no-cache
Content-Type: text/html
Content-Length: 61

<?xml version="1.0" encoding="UTF-8"?><response>OK</response>
*/
	i = find_sub_string_index(buff,"<response>OK</response>");
	if( i < 0 ) goto err_out;
	log("login=%s\n",&buff[i]);

	log("login ok \n");
	close(sockfd);


	sockfd = get_client_socket(ROUTE_4G_IP,ROUTE_4G_PORT);
	if( sockfd < 0 ) return -1;

        memset(str1, 0, 4096);
        strcat(str1, "GET /api/monitoring/status HTTP/1.1\r\n");
        strcat(str1, "Host: 192.168.1.1\r\n");
        strcat(str1, "Accept: */*\r\n");
        strcat(str1, "Cache-Control: no-cache\r\n");
        strcat(str1, "X-Requested-With: XMLHttpRequest\r\n");
        strcat(str1, "Referer: http://192.168.1.1/html/deviceinformation.html\r\n");
        strcat(str1, "Connection: close\r\n");
        strcat(str1, "\r\n");
        ret = write(sockfd,str1,strlen(str1));
        if (ret < 0) {
                log("发送失败！错误代码是%d，错误信息是'%s'\n",errno, strerror(errno));
		goto err_out;
        }
	ret = recive_socket_block(sockfd,buff,4096,10000);
	if( 0 >= ret){
                log("recevie data error\n");
		goto err_out;
		
	}	
	//log("%s\n",buff);
/*

HTTP/1.1 200 OK
Date: Thu, 01 Jan 1970 00:00:00 GMT
Server: mini_httpd/1.19 19dec2003
Connection: close
Cache-Control: no-cache
Content-Type: text/html
Content-Length: 913

<?xml version="1.0" encoding="UTF-8"?>
<response>
<ConnectionStatus>901</ConnectionStatus>
<WifiConnectionStatus></WifiConnectionStatus>
<SignalStrength>39</SignalStrength>
<SignalIcon>2</SignalIcon>
<CurrentNetworkType>19</CurrentNetworkType>
<CurrentServiceDomain>2</CurrentServiceDomain>
<RoamingStatus>0</RoamingStatus>
<BatteryStatus></BatteryStatus>
<BatteryLevel></BatteryLevel>
<BatteryPercent></BatteryPercent>
<simlockStatus>0</simlockStatus>
<WanIPAddress>100.80.212.39</WanIPAddress>
<WanIPv6Address></WanIPv6Address>
<PrimaryDns>0.0.0.0</PrimaryDns>
<SecondaryDns>0.0.0.0</SecondaryDns>
<PrimaryIPv6Dns></PrimaryIPv6Dns>
<SecondaryIPv6Dns></SecondaryIPv6Dns>
<CurrentWifiUser>1</CurrentWifiUser>
<TotalWifiUser>10</TotalWifiUser>
<ServiceStatus>2</ServiceStatus>
<SimStatus>1</SimStatus>
<WifiStatus>1</WifiStatus>
<msisdn></msisdn>
<classify>wingle</classify>
</response>
*/
	//<WanIPAddress>100.83.192.99</WanIPAddress>
	i = find_sub_string_index(buff,"<WanIPAddress>");
	if( i < 0 ){
		log("no ip find\n");
		goto err_out;
	}
	i = i + strlen("<WanIPAddress>");
	j = 0;
	for( i; i< ret; i++){
		if( buff[i] == '<' )
			break;
		wip[j++] = buff[i];
	}
	log("ip=%s\n",wip);



	close(sockfd);
	return 0;
err_out:
	close(sockfd);
	return -1;
	
}
void clear_o2o_data()
{
	memset(o2o_g_data.username,0,O2O_USR_MSG_STRING_LEN);
	memset(o2o_g_data.pwd,0,O2O_USR_MSG_STRING_LEN);
	memset(o2o_g_data.id,0,O2O_USR_MSG_STRING_LEN);

	memset(o2o_g_data.ip,0,O2O_IP_MSG_STRING_LEN);
	memset(o2o_g_data.mac,0,O2O_MAC_MSG_STRING_LEN);
	memset(o2o_g_data.olt,0,O2O_OLT_MSG_STRING_LEN);
	//memset(o2o_g_data.iol,0,O2O_IOL_MSG_STRING_LEN);
	o2o_g_data.iol = 1;//1 =initing 
	o2o_g_data.is_user_msg_update = 0;
	o2o_g_data.is_ip_update = 0;
}
int get_o2o_user_msg()
{
	memset(o2o_g_data.username,0,O2O_USR_MSG_STRING_LEN);
	memset(o2o_g_data.pwd,0,O2O_USR_MSG_STRING_LEN);
	memset(o2o_g_data.id,0,O2O_USR_MSG_STRING_LEN);

//expand function read from file
	strcat(o2o_g_data.username,"RuanJG");
	strcat(o2o_g_data.pwd,"123");
	strcat(o2o_g_data.id,"te350_4G");
	o2o_g_data.is_user_msg_update = 0;
	return 0;
}
int get_4G_ip()
{
	int ret;
	memset(o2o_g_data.ip,0,O2O_IP_MSG_STRING_LEN);
	ret = get_wan_ip(o2o_g_data.ip);
	if( ret < 0 ){
		log("get 4G router ip error \n");
		return -1;
	}
	o2o_g_data.is_ip_update = 0;
	return 0;
}
int get_mac_addr()
{
	memset(o2o_g_data.mac,0,O2O_MAC_MSG_STRING_LEN);
	strcpy(o2o_g_data.mac,"11:22:33:44:55:66");
	return 0;
}
int get_olt()
{
	//2015-10-20%2012:12:12
	time_t timep;
	struct tm *p;
	char str[1024];

	memset(o2o_g_data.olt,0,O2O_OLT_MSG_STRING_LEN);
	time(&timep);
	p=localtime(&timep);
	sprintf(o2o_g_data.olt,"%d-%2d-%2d%s%2d:%2d:%2d",1900+p->tm_year , 1+p->tm_mon, p->tm_mday,"%20", p->tm_hour, p->tm_min, p->tm_sec );
}
void reflash_update_time()
{
	gettimeofday(&o2o_g_data.last_update_tv, NULL);
}
int is_time_to_update()
{
	struct timeval tv;
	unsigned int ms;
	unsigned int s;
	unsigned int diff_ms;

	gettimeofday(&tv, NULL);

	s = tv.tv_sec - o2o_g_data.last_update_tv.tv_sec;
	diff_ms = s*1000;
	if( diff_ms >= UPDATE_INTERVEL_TIME ) return 1;

	ms = (unsigned int)(tv.tv_usec/1000);
	diff_ms += (ms - o2o_g_data.last_update_tv.tv_usec/1000);
	log("ms=%d\n",diff_ms);
	if( diff_ms > UPDATE_INTERVEL_TIME )
		return 1;
	else 
		return 0;

}
int check_o2o_cmd_result(char * buff,int len, char *checkstr)
{
/*
HTTP/1.1 200 OK
Server: nginx/1.7.6
Date: Thu, 22 Oct 2015 10:03:51 GMT
Content-Type: text/html;charset=UTF-8
Content-Length: 2
Connection: close
Set-Cookie: JSESSIONID=2A2C4E0288ED71A384C5C71DF6933CB5; Path=/equipment/; HttpOnly

ok
*/	
	int ret,index;
	ret = find_sub_string_index(buff,"HttpOnly");
	if( ret < 0 ) return -1;
	index = ret;

	ret = find_sub_string_index(&buff[index],checkstr);
	if( ret < 0 ) return -1;
	index += ret;

	return index;
}
int update_o2oservice_status()
{
	char cmd[1024];
	int sockfd;
	int ret;
	char buff[4096];


	sockfd = get_client_socket(O2OIP,O2OPORT);
	if( sockfd <0 ){
		log("sockfd cread error \n");
		return -1;
	}

	//pthread_mutex_lock(&mutex);
	sprintf(cmd,"GET /UAV/%s/%s?id=%s&ip=%s&iol=%d&mac=%s&olt=%s HTTP/1.1\r\n",o2o_g_data.username,o2o_g_data.pwd, o2o_g_data.id, o2o_g_data.ip, o2o_g_data.iol, o2o_g_data.mac, o2o_g_data.olt);
	//pthread_mutex_unlock(&mutex);

	if( 0 > send_o2oc_http_cmd(sockfd,cmd) ){
		log("send_o2oc_http_cmd error \n");
		goto error_out;
	}
	
	ret = recive_socket_block(sockfd,buff,4096,10000);
	if( ret <= 0 ){
		log("recive_socket_block error \n");
		goto error_out;
	}
	ret = check_o2o_cmd_result(buff,ret,"ok");
	log("update_o2oservice_status result %s\n", &buff[ret]);
	if( ret < 0 ){
		log("get o2o data reult error \n");
		goto error_out;
	}

	close(sockfd);
	return 0;

error_out:
	close(sockfd);
	return -1;
}

int loop()
{
	int ret;

	if(o2o_g_data.is_user_msg_update)
		get_o2o_user_msg();
	if( o2o_g_data.is_ip_update )
		get_4G_ip();

	get_olt();
	if( is_time_to_update() ){
		ret  = update_o2oservice_status();
		reflash_update_time();
	}
	sleep(1); 
	return 0;
}

void setup()
{
	pthread_mutex_init(&mutex,NULL);


	//pthread_mutex_lock(&mutex);
	clear_o2o_data();

	get_olt();
	get_mac_addr();
	o2o_g_data.iol = 1; //initing
	strcpy(o2o_g_data.ip,UNVAILD_IP);

	//these data cannot update so much
	o2o_g_data.is_user_msg_update = 1;
	get_o2o_user_msg();
	//pthread_mutex_unlock(&mutex);

	o2o_g_data.is_ip_update = 1;
	get_4G_ip(); 

	update_o2oservice_status();
	reflash_update_time();
	
}
int is_need_quit()
{
	//pthread_mutex_lock(&mutex);
	return o2o_service_need_quite;
	//pthread_mutex_unlock(&mutex);
}
/*
void quit_handler( int sig )
{
	int i;

	printf("\n");
	printf("TERMINATING AT USER REQUEST\n");
	printf("\n");


	printf("Ruan: exit by ctrl-c\n");
	exit(0);
}
*/
void* o2o_service_main( void *argv)
{
	int ret;

	//signal(SIGINT,quit_handler);
	setup();
	while(!is_need_quit()){
		ret = loop();
		if( ret < 0 ) setup();
	}
}

void call_o2o_service_quit()
{
	//pthread_mutex_lock(&mutex);
	o2o_service_need_quite = 1;
	//pthread_mutex_unlock(&mutex);
}

void call_update_o2o_online_status(int status)
{
	//pthread_mutex_lock(&mutex);
	if( 0 <= status && status <= 99 )
		o2o_g_data.iol = status;
	//pthread_mutex_unlock(&mutex);
}
void call_update_o2o_4G_ip()
{
	int ret;
	o2o_g_data.is_ip_update = 1;
}

