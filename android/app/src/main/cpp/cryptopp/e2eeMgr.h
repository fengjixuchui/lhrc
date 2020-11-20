#pragma once
#include <string>

class e2eeMgr

{
public:
	e2eeMgr();
	virtual ~e2eeMgr();
	bool init();
	std::string getRandPsw();
	std::string rsaEncode(const std::string& data);
	std::string rsaEndcodeToServer(const std::string& data);
	std::string rsaEndcodeToServer(const char* data,int len);

	std::string rsaDecode(const std::string& data);
	std::string getPublicKey();
	std::string encode(const char* data, int datalen, const std::string& seed, const std::string& publicKey);
	bool		rc4Encode(const char* data, int len);
private:



	std::string m_rsaSeed;
	std::string m_rsaPrivetKey;
	std::string m_rsaPublicKey;
	std::string m_randPsw;
	
	static std::string m_rsaServerPublicKey;

};

