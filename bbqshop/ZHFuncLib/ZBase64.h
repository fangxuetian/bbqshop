#ifndef Z_BASE_64_H  
#define Z_BASE_64_H  

#include <iostream>
class __declspec(dllexport) ZBase64
{
public:
	ZBase64(void);
	~ZBase64(void);

	/*����
    DataByte
        [in]��������ݳ���,���ֽ�Ϊ��λ
    */
    std::string Encode(const char* Data,int DataByte);
    /*����
    DataByte
        [in]��������ݳ���,���ֽ�Ϊ��λ
    OutByte
        [out]��������ݳ���,���ֽ�Ϊ��λ,�벻Ҫͨ������ֵ����
        ������ݵĳ���
    */
    std::string Decode(const char* Data,int DataByte,int& OutByte);
};

#endif  