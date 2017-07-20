#pragma once
#include <vector>

#define NAMEPOS 0.02
#define CONTENTPOS 0.35
#define SPACE1 35

#define ONLYLINE L"-------------------------------------"

#define PrintHeader(mPrinter, shopInfo, pWnd, headName) \
	{ \
	mPrinter->SetDeviceWidth(pWnd->GetPrinterDeviceWidth());\
	mPrinter->AddNewLine(0.5, ONLYLINE, SPACE1, 50, L"����", 0); \
	mPrinter->AddNewLine(0.5, headName, SPACE1, pWnd->GetCommentFontSZ(), L"", 0);  \
	mPrinter->AddNewLine(0.5, ONLYLINE, SPACE1, 50, L"", 0);  \
	mPrinter->AddNewLine(NAMEPOS, L"", SPACE1, pWnd->GetCommentFontSZ());  \
	\
	mPrinter->AddNewLine(NAMEPOS, L"�������ƣ�", SPACE1, pWnd->GetCommentFontSZ(), L"����"); \
	mPrinter->AppendLineString(CONTENTPOS, pWnd->StrToWStr(shopInfo.shopName)); \
	mPrinter->AddNewLine(NAMEPOS, L"���̴��룺", SPACE1); \
	mPrinter->AppendLineString(CONTENTPOS, pWnd->StrToWStr(shopInfo.shopCode)); \
	mPrinter->AddNewLine(NAMEPOS, L"�� �� ̨��", SPACE1); \
	mPrinter->AppendLineString(CONTENTPOS, pWnd->StrToWStr(shopInfo.cashdeskName)); \
	mPrinter->AddNewLine(NAMEPOS, L"�� �� Ա��", SPACE1); \
	mPrinter->AppendLineString(CONTENTPOS, pWnd->StrToWStr(shopInfo.userName)); \
	}  \

#define PrintEnd(mPrinter) \
	{ \
	mPrinter->AddNewLine(NAMEPOS, L""); \
	mPrinter->AddNewLine(NAMEPOS, L"��ע��֧������ǲ�����˵�Ψһƾ֤�������Ʊ��� : ",SPACE1); \
	mPrinter->AddNewLine(NAMEPOS, ONLYLINE); \
	mPrinter->AddNewLine(NAMEPOS, L"�ǻ�΢���� ",SPACE1); \
	mPrinter->AddNewLine(NAMEPOS, L"",30); \
	mPrinter->AddNewLine(0.5, ONLYLINE, 0, 0, L"", 0); \
	} \

#undef NAMEPOS
#undef CONTENTPOS
#undef SPACE1

class __declspec(dllexport) PosPrinter
{
public:
	struct LINESTRING 
	{
		unsigned int lineNum;  // �к�
		float x;  // ռ��һ�еİٷֱ�  С��
		int lineSpace;  // �±߾�
		std::wstring lineContent;
		int fontsz;
		std::wstring fontName;
		int trimType;  // 0���У�1���
	};

	PosPrinter();
	virtual ~PosPrinter(void);

	//friend CFont* printLineString(PosPrinter *pp, CDC *inPrintDC, CFont *oriFont, PosPrinter::LINESTRING &inLine);//��Ԫ���� 
//private:
	std::wstring docName;
	int deviceWidth;
	int posY;  // ��ӡλ��Y

	std::vector<LINESTRING > allLines;
	LINESTRING *lastLine;

	BOOL getPrinterDevice(LPTSTR pszPrinterName, HGLOBAL* phDevNames, HGLOBAL* phDevMode);
	inline void getCurrentLineInfo(int &outLineNum, int &outLineSpace);
	inline void addLineByLineNum(int lineNum, float xPos, std::wstring inContent, int lineSpace ,int fontsz, std::wstring fontname, int trimType);
	//inline bool createNewFont(CDC *inPrintDC, CFont &outFont, int fontsz, std::wstring fontname, bool isOnlyLine = false);
	//inline CFont* printLineString(CDC *inPrintDC, CFont *oriFont, LINESTRING &inLine);
	inline float pixStrPercentInDC(std::wstring inContent, int textcx, int &outTextlen); // ������ռ��������
	inline int getNumInLine(std::wstring inDes, float pixch, float pixen, float allPix); 
protected:
	void init();
public:
	bool PreparePrinter(std::wstring inName);
	void AddNewLine(float xPos, std::wstring inContent, int lineSpace = 0, int fontsz = 0, std::wstring fontname = L"", int trimType = 1); // ���fontszΪ0������Ĭ�����������֮ǰ���õ�����
	void AppendLineString(float xPos, std::wstring inContent, int fontsz = 0, std::wstring fontname = L"", int trimType = 1);
	int GetDeviceWidth();
	void SetDeviceWidth(int inWidth);
	void CallPrinter();
	void GetAllPrinters(std::vector<std::wstring > &outPrinters, const std::wstring &oriPrinter, int &outOriIndex);
};

