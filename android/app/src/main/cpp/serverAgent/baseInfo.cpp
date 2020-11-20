#include "stdafx.h"
#include "baseInfo.h"

#define TASK_DATA_SIZE 360
#define TASK_HANDLE_SIZE 16
//#define ISDEBUG
//#define INFO_TEST 1



std::string ntl_ntoT(int i)
{
	char p_ret[20] = {};
	sprintf(p_ret, "%d", i);
	return p_ret;
}

std::string ntl_ntoT(uint64_t i)
{
	char p_ret[64] = {};
	sprintf(p_ret, "%lld", i);
	return p_ret;

}

std::string ntl_post(const std::string& buff, const char* var)
{
	std::string p = var;
	p += "=";
	int i = buff.find(p);
	if (i == -1)
	{
		return "";
	}
	i += p.length();
	long buffLen = strlen(buff.c_str());
	int i2 = buff.find("&", i);
	if (i2 == -1)
	{
		i2 = buffLen;
	}
	return buff.substr(i, i2 - i);
}
