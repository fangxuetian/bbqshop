#pragma once

#include <string>

#define ONLYLINELPT "---------------------------------\r\n"

#define PrintHeaderLPT(mPrinter, shopInfo, headName) \
	{ \
	mPrinter.PrintString(ONLYLINELPT, PosPrinterLptCls::CENTERALIGN); \
	mPrinter.PrintString(headName, PosPrinterLptCls::TITLE);  \
	mPrinter.PrintString("\r\n", PosPrinterLptCls::CENTERALIGN);  \
	mPrinter.PrintString(ONLYLINELPT, PosPrinterLptCls::CENTERALIGN);  \
	mPrinter.PrintString("\r\n", PosPrinterLptCls::CENTERALIGN);  \
	\
	mPrinter.PrintString("�������ƣ�", PosPrinterLptCls::NORMAL); \
	QString tmpstr = shopInfo.shopName; \
	mPrinter.PrintString(tmpstr.toLocal8Bit().toStdString(), PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString("\r\n", PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString("���̴��룺", PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString(shopInfo.shopCode, PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString("\r\n", PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString("�� �� ̨��", PosPrinterLptCls::NORMAL); \
	tmpstr = shopInfo.cashdeskName; \
	mPrinter.PrintString(tmpstr.toLocal8Bit().toStdString(), PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString("\r\n", PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString("�� �� Ա��", PosPrinterLptCls::NORMAL); \
	tmpstr = shopInfo.userName; \
	mPrinter.PrintString(tmpstr.toLocal8Bit().toStdString(), PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString("\r\n", PosPrinterLptCls::NORMAL); \
	}  \

#define PrintEndLPT(mPrinter) \
	{ \
	mPrinter.PrintString("\r\n��ע��֧������ǲ�����˵�Ψһƾ֤�������Ʊ��� : \r\n", PosPrinterLptCls::NORMAL); \
	mPrinter.PrintString(ONLYLINELPT, PosPrinterLptCls::CENTERALIGN); \
	mPrinter.PrintString("�ǻ�΢����\r\n",PosPrinterLptCls::CENTERALIGN); \
	mPrinter.PrintString(ONLYLINELPT, PosPrinterLptCls::CENTERALIGN); \
	} \

class __declspec(dllexport) PosPrinterLptCls
{
public:
	enum TXTTYPE
	{
		UNKNOWN,
		TITLE,
		CENTERALIGN,
		RIGHTALIGN,
		NORMAL,
	};
	PosPrinterLptCls(void);
	virtual ~PosPrinterLptCls(void);

	int Prepare(int lptNum); // If there is no error, this function will return 0, else return error num.
	int Prepare(const char *lptName);
	void PrintString(std::string inContent, TXTTYPE inType);
	static bool CanOpenLPT(int lptNum);

private:
	HANDLE Hcom;
	TXTTYPE curType;
};

