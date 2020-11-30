#pragma once

#include <memory.h>
#include <string>
#define SOCKET int
#define CONNID int
#define BYTE unsigned char

//#define S_HOST {"bjvideoproxy.110route.cn","shvideoproxy.110route.cn","gzvideoproxy.110route.cn"}
//#define S_IP {"117.78.50.23","124.71.172.133","121.37.204.50"}

enum handleType
{
	send_handle,
	not_findnetbar,
	send_close,
	send_fail,
	send_body,
	recv_body,//�յ����ɳ���body
	send_login,
	recv_login,
	send_push_login_one,
	send_push_login_all,
	recv_push_login,//����
	send_push_close,
	send_first_pack,
	recv_first_pack,
	send_login_v2,
	send_ping,
	recv_ping,
	send_update_pin,
	send_login_control,
	send_http_handle,
	send_to_httpid,
	send_login_v3,
};

struct tcpHandle
{
	unsigned char		type;
	char				barid[33];
	uint32_t			tcpId;
	uint64_t			httpId;
	int					checkid;
	tcpHandle()
	{
		memset(this, 0,sizeof(tcpHandle));
	};

};



#define tcp_handel_len sizeof(tcpHandle)

enum FIRST_PACK_NUM
{
	E_OK = 0,
	E_SYSTRM,
	E_VER,
	E_DATA_TOO_LONG,
	E_PARAM,
	E_NOT_MARK,
	E_CONNECT_FAILED

};

std::string ntl_ntoT(int i);
std::string ntl_ntoT(uint64_t i);
std::string ntl_post(const std::string& buff, const char* var);