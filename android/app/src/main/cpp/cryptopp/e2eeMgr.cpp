#include "e2eeMgr.h"
#include "rsa.h"
#include "randpool.h"
#include "rc4.h"
#include "basecode.h"
#include "publicbase/baseFunc.h"

#define rsakeylen 2048




#define serverPublicKey "MIIBIDANBgkqhkiG9w0BAQEFAAOCAQ0AMIIBCAKCAQEAv2a20gBKu/5PtwrP8FUbDybV519zOM3BZPQFp+PeLL4DOPo0aC3HHt/FpIyQNO92Kfb/lVcHOtQ9uQWT+aGaNhKL21kIrgZyx0R77kP2Ezj7AizL1EF6f6k/ezVX8m1ftyeCjZufE9BNh9ooMHI8yZWcj2GTXqdkfPV+P2XmuGCHsme4aAjZ5nLs1tWlgUmf8PCQOr0/XaoYXXE0c9vD4XknWZXcd1S8h8psCX6y++FdzqDnrZIL58sJ7mSBaaXrNqbM85wcLgbMxWV0k6djfMwneDnnxyRYJ6y1MnNwV+92tazUzYcijnI5eQbm4vYThFuyoYngWy1+NUhIaKrTFQIBEQ"


int g_tempint;
std::string e2eeMgr::m_rsaServerPublicKey = webbase::BaseDecode(serverPublicKey, strlen(serverPublicKey), g_tempint);

e2eeMgr::e2eeMgr()
{

}

e2eeMgr::~e2eeMgr()
{
}

bool e2eeMgr::init()
{
	CryptoPP::RandomPool randomPool;
	srand(rand());
	m_rsaSeed = ntl::getRandStr(20, 30);
	randomPool.IncorporateEntropy((byte*)m_rsaSeed.c_str(), m_rsaSeed.length());

	m_rsaPrivetKey.resize(rsakeylen);
	size_t p_len = m_rsaPrivetKey.length();
	CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(randomPool, rsakeylen);
	CryptoPP::ArraySink decArr((byte*)&m_rsaPrivetKey[0], p_len);

	decryptor.AccessMaterial().Save(decArr);
	decArr.MessageEnd();
	m_rsaPrivetKey.resize(decArr.TotalPutLength());

	CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(decryptor);
	m_rsaPublicKey.resize(rsakeylen);
	p_len = m_rsaPublicKey.length();

	CryptoPP::ArraySink encArr((byte*)&m_rsaPublicKey[0], p_len);
	encryptor.AccessMaterial().Save(encArr);
	encArr.MessageEnd();
	m_rsaPublicKey.resize(encArr.TotalPutLength());
#ifdef _DEBUG
	ntl::writeFile("privetkey.txt", webbase::BaseEncode((unsigned char*)&m_rsaPrivetKey[0], m_rsaPrivetKey.length()));
	ntl::writeFile("publickey.txt", webbase::BaseEncode((unsigned char*)&m_rsaPublicKey[0], m_rsaPublicKey.length()));
	ntl::writeFile("seed.txt", m_rsaSeed);
#endif // _DEBUG

	return true;
}


std::string e2eeMgr::getRandPsw()
{
	if (m_randPsw != "")
	{
		return m_randPsw;
	}
	m_randPsw = ntl::getRandStr(24, 32);
	return m_randPsw;
}

std::string e2eeMgr::rsaEncode(const std::string& data)
{	
	return encode(&data[0],data.length(),m_rsaSeed,m_rsaPublicKey);
}

std::string e2eeMgr::rsaEndcodeToServer(const std::string& data)
{
	return encode(&data[0], data.length(), "", m_rsaServerPublicKey);
}

std::string e2eeMgr::rsaEndcodeToServer(const char* data, int len)
{
	return encode(data, len, "", m_rsaServerPublicKey);
}


std::string e2eeMgr::rsaDecode(const std::string& data)
{
	if (m_rsaPrivetKey.empty() || data.empty())
	{
		return "";
	}
	std::string p_ret;
	if (data.length() < rsakeylen)
	{
		p_ret.resize(rsakeylen);
	}
	else
	{
		p_ret.resize(data.length());
	}
	uint64_t putLen = 0;
	try
	{
		CryptoPP::ArraySource keyArr((byte*)&m_rsaPrivetKey[0], m_rsaPrivetKey.length(), true);
		CryptoPP::RSAES_OAEP_SHA_Decryptor dec;
		dec.AccessKey().Load(keyArr);

		CryptoPP::RandomPool randomPool;
		randomPool.IncorporateEntropy((byte*)&m_rsaSeed[0], m_rsaSeed.length());

		uint64_t fixedLen = dec.FixedCiphertextLength();
		if (data.length() % fixedLen != 0)
		{
			return "";
		}
		size_t p_datalen = data.length();
		size_t p_retLen = p_ret.length();
		byte* p_retAddr = (byte*)&p_ret[0];
		byte* p_dataAddr = (byte*)&data[0];
		size_t p_len = 0;
		int p_packNum = 0;

		for (uint64_t i = 0; i < p_datalen; i += fixedLen)
		{
			p_len = fixedLen < (p_datalen - i) ? fixedLen : (p_datalen - i);
			CryptoPP::ArraySink* dstArr = new CryptoPP::ArraySink(p_retAddr + putLen, (size_t)(p_retLen - putLen));
			CryptoPP::ArraySource source(p_dataAddr + i, p_len, true, new CryptoPP::PK_DecryptorFilter(randomPool, dec, dstArr));
			putLen += dstArr->TotalPutLength();
		}
	}
	catch (const std::exception&)
	{
		return "";
	}

	p_ret.resize(putLen);
	return p_ret;
}


std::string e2eeMgr::getPublicKey()
{
	std::string p_ret = "seed=" + m_rsaSeed;
	p_ret += "&publickey=" + m_rsaPublicKey;
	return p_ret;
}

bool e2eeMgr::rc4Encode(const char* data, int len)
{
	if (m_randPsw == "")
	{
		return false;
	}
	rc4::encrypt(data, len, m_randPsw.c_str(), m_randPsw.length());
	return true;
}

std::string e2eeMgr::encode(const char* data,int datalen, const std::string& seed, const std::string& publicKey)
{
	std::string p_ret;
	if (datalen < rsakeylen)
	{
		p_ret.resize(rsakeylen);
	}
	else
	{
		p_ret.resize(datalen * 2);
	}

	uint64_t putLen = 0;
	try
	{
		CryptoPP::RSAES_OAEP_SHA_Encryptor enc;
		CryptoPP::RandomPool randomPool;
		CryptoPP::ArraySource keyArr((byte*)&publicKey[0], publicKey.length(), true);
		enc.AccessKey().Load(keyArr);
		randomPool.IncorporateEntropy((byte*)&seed[0], seed.length());
		uint64_t fixedLen = enc.FixedMaxPlaintextLength();
		size_t p_datalen = datalen;
		size_t p_retLen = p_ret.length();
		byte* p_retAddr = (byte*)&p_ret[0];
		byte* p_dataAddr = (byte*)data;
		uint64_t p_len = 0;
		for (uint64_t i = 0; i < p_datalen; i += fixedLen)
		{
			p_len = fixedLen < (p_datalen - i) ? fixedLen : (p_datalen - i);
			CryptoPP::ArraySink* dstArr = new CryptoPP::ArraySink(p_retAddr + putLen, p_retLen - putLen);
			CryptoPP::ArraySource source(p_dataAddr + i, p_len, true, new CryptoPP::PK_EncryptorFilter(randomPool, enc, dstArr));
			putLen += dstArr->TotalPutLength();
		}
	}
	catch (const std::exception&)
	{
		return "";
	}
	p_ret.resize(putLen);
	return p_ret;
}
