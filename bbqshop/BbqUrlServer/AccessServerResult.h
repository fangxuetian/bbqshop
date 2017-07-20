#ifndef ACCESS_SERVER_RESULT_H
#define ACCESS_SERVER_RESULT_H

#include <string>

class __declspec(dllexport) AccessServerResult
{
public:
	AccessServerResult(void);
	virtual ~AccessServerResult(void);

	virtual bool DealWithJSONFrServer(std::string mRecvJsonStr, int urlTag, std::string urlApi);  // ����ֵ �ṹ�Ƿ���ȷ
	virtual void CurlError(std::string url, int res, int urlTag);
};

#endif 