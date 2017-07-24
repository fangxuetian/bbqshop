﻿#include <afxwin.h>
#include "maindlg.h"
#include "AllWindowTitle.h"
#include "TipExtendBtn.h"
#include "ZHSettingRW.h"
#include "ProcessProtocal.h"
#include "zhfunclib.h"
#include "ZhuiHuiMsg.h"
#include "PosPrinterCls.h"
#include "PosPrinterLptCls.h"
#include <QMessageBox>
#include <QTimer>
#include <ScreenCatch.h>
#include "AllExeName.h"
#include "ZBase64.h"
#include <QDir>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	if ((dwStyle & WS_OVERLAPPEDWINDOW) && (dwStyle & WS_VISIBLE))
	{
		// add window to list
		QString className, windowTitle;
		WCHAR tmpName[256];
		CWnd* pWnd = CWnd::FromHandle(hwnd);
		// 窗口类名
		::GetClassName(hwnd, tmpName, 256);
		className = QString::fromWCharArray(tmpName);
		// 窗口标题
		::GetWindowText(pWnd->GetSafeHwnd(), tmpName, 256);
		windowTitle = QString::fromWCharArray(tmpName);

		if (!windowTitle.isEmpty())
		{
			MainDlg *parWidget = (MainDlg *)lParam;
			parWidget->mWinClassNames.push_back(className);
			parWidget->mWinWindowNames.push_back(windowTitle);
		}
	}

	return TRUE;
}


MainDlg::MainDlg(QApplication *pApp, char *account, QWidget *parent)
	: QDialog(parent), parWidget(parent), mainApp(pApp)
{
	setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::Popup|Qt::Tool);
	ui.setupUi(this);
	setWindowTitle(MAINDLGTITLE);

	setStyleSheet("QDialog{background-color:#F9F7F7;border: 1px solid #112D4E;border-width:1px;border-radius:2px}"); 
	setTopBtn();

	urlServer = new BbqUrlServer(this);
	// 保存Account
	codeSetIO::ShopCashdeskInfo &deskInfo = mZHSetting.shopCashdestInfo;
	memcpy(deskInfo.account, account, strlen(account));
	deskInfo.account[strlen(account)] = 0;
	ZHSettingRW settingRW(mZHSetting);
	settingRW.ReadZHSetting();
	// 初始化数据
	initFrame();
	// 按钮控件的信号
	connect(ui.btnGetScreen, SIGNAL(pressed()), this, SLOT(catchScreen()));
	connect(ui.btnCheckPrice, SIGNAL(pressed()), this, SLOT(checkPrice()));
	connect(ui.btnCommit, SIGNAL(pressed()), this, SLOT(commitSlot()));
	connect(ui.btnBind, SIGNAL(pressed()), this, SLOT(bindSlot()));
	connect(ui.pbtSave, SIGNAL(pressed()), this, SLOT(saveSetting()));
	connect(ui.cboCashNo, SIGNAL(currentIndexChanged(int )), this, SLOT(cashNoChanged(int )));
	connect(this, SIGNAL(showBindTipSig(bool )), this, SLOT(showTipSlot(bool )));
	connect(ui.btnCheck, SIGNAL(pressed()), this, SLOT(checkCashSoftCorrect()));
	connect(ui.btnPrinterTest, SIGNAL(pressed()), this, SLOT(printerTest()));
	connect(ui.pbtClear, SIGNAL(pressed()), this, SLOT(clickClear()));
}

MainDlg::~MainDlg()
{

}

//void MainDlg::SendToURLRecord(const char *logLevel, const char *logModule, const char *logMessage, int urlTag)
//{
//	urlServer->SendToURLRecord(logLevel, logModule, logMessage, urlTag);
//}

//void MainDlg::GetDataFromServer(std::string inSecondAddr, std::string inApi, std::string inData, int urlTag)
//{
//	urlServer->GetDataFromServer(inSecondAddr, inApi, inData, urlTag);
//}

//void MainDlg::GetDataFromServer1(std::string inUrl, std::string inSecondAddr, std::string inApi, Json::Value &ioRootVal, int urlTag)
//{
//	urlServer->GetDataFromServer1(inUrl, inSecondAddr, inApi, ioRootVal, urlTag);
//}

//void MainDlg::GetMAC(char *mac)
//{
//	urlServer->GetMAC(mac);
//}

//void MainDlg::TimeFormatRecover(std::string &outStr, std::string inOriTimeStr)
//{
//	urlServer->TimeFormatRecover(outStr, inOriTimeStr);
//}

//std::string MainDlg::GetPayTool(int inType)
//{
//	return urlServer->GetPayTool(inType);
//}

//bool MainDlg::IsImportentOperateNow()
//{
//	return urlServer->IsImportentOperateNow();
//}

inline void MainDlg::setTopBtn()
{
	QRect parRect = this->geometry();
	int xpos = parRect.width() - 30;
	int ypos = 5;

	TipExtendBtn *closeBtn = new TipExtendBtn(this, "/res/close.png", "/res/closered.png");
	closeBtn->setGeometry(QRect(xpos, ypos, 25, 25));
	closeBtn->setToolTip(QString::fromLocal8Bit("关闭"));

	// 关闭
	connect(closeBtn, SIGNAL(pressed()), this, SLOT(closeMainDlg()));
}

void MainDlg::closeMainDlg()
{
	hide();
	mainApp->quit();
}

void MainDlg::hideEvent(QHideEvent * event)
{
	Json::Value mValData;
	mValData[PRO_HEAD] = TO_FLOATWIN_CLOSEMAINDLG;
	mValData[PRO_DLG_STATUS] = 0;
	HWND hwnd = ::FindWindowW(NULL, FLOATWINTITLEW);
	ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
}

bool MainDlg::DealWithJSONFrServer(std::string mRecvJsonStr, int urlTag, std::string urlApi)
{
	Json::Reader reader;
	Json::Value value;
	bool suc = reader.parse(mRecvJsonStr, value);
	if (suc)
	{
		if (value.isMember("return_msgs"))
		{
			const char *msg = value["return_msgs"].asCString();
			Json::Value mValData;
			mValData[PRO_HEAD] = TO_SHOWTIP;
			mValData[PRO_TIPSTR] = msg;

			HWND hwnd = ::FindWindowW(NULL, FLOATWINTITLEW);
			ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
		}
		switch (urlTag)
		{
		case URL_SETTING_DLG_COMMIT:
			urlbackOnCommit(value);
			break;
		case URL_SETTING_DLG_BIND:
			urlbackOnBind(value);
			break;
		default:
			break;
		}
		
		return true;
	}
	return false;
}

inline void MainDlg::initFrame()
{
	codeSetIO::ShopCashdeskInfo &shopInfo = mZHSetting.shopCashdestInfo;
	ui.shopNameTxt->setText((shopInfo.shopName));
	ui.cashNoTxt->setText(shopInfo.cashdeskName);
	ui.shopNumTxt->setText(QString::number(shopInfo.shopid));

	char cashdes[60];
	sprintf(cashdes, "(%s) %s", shopInfo.cashdeskId, shopInfo.cashdeskName);
	ui.cboCashNo->addItem(cashdes);
	ui.cboCashNo->setCurrentIndex(0);
	// 收银软件信息
	codeSetIO::CarishDesk &carishInfo = mZHSetting.carishInfo;
	int mRelativeType = carishInfo.selectRange.relitiveType;
	ui.cboMoneyPos->setCurrentIndex(mRelativeType);

	QString windowName = QString::fromStdWString(ZHFuncLib::StringToWstring(carishInfo.windowName));
	QString exeName = carishInfo.exeName;
	// 是否二值化
	ui.chbBinarization->setChecked(mZHSetting.carishInfo.selectRange.imageBinaryzation != 0);
	// 截图放大倍数
	float imageScale;
	ui.cboImageScale->setCurrentIndex(getImageScaleTag(imageScale));
	// 获取所有的窗口
	mWinClassNames.clear();
	mWinWindowNames.clear();
	mWinClassNames.push_back(QString::fromLocal8Bit("不开启自动识别"));
	mWinWindowNames.push_back(QString::fromLocal8Bit("不开启自动识别"));
	EnumWindows(EnumWindowsProc, (LPARAM)this);//遍历窗口程序
	int sz = mWinWindowNames.size();
	int cursel = 0;
	for (int i = 0; i < sz; ++i)
	{
		ui.cboCashTool->addItem(mWinWindowNames[i]);
		if (mWinWindowNames[i] == windowName)
			cursel = i;
	}
	ui.cboCashTool->setCurrentIndex(cursel);
	connect(ui.cboCashTool, SIGNAL(currentIndexChanged(int)), this, SLOT(cashToolChanged(int)));
	cashToolChanged(cursel != 0);
	// 是否使用支付扫码枪
	ui.chbStartGun->setChecked(shopInfo.isUsePayGun == 1);
	// 获取pos打印机
	QString printerName = carishInfo.printerName;
	int printIndex = -1;
	PosPrinter posP;
	std::vector<std::wstring > allPrinters;
	posP.GetAllPrinters(allPrinters, printerName.toStdWString(), printIndex);
	sz = allPrinters.size();
	for (int i = 0; i < sz; ++i)
	{
		ui.cboPrinter->addItem(QString::fromStdWString(allPrinters[i]));
	}
	// 获取 LPT printer
	char lptlastchar = 0;
	if (printIndex < 0 && printerName.contains("LPT"))
	{
		lptlastchar = printerName.at(3).toLatin1();
		printIndex = sz;
	}
	//else if (printIndex == -1)
	//	printIndex = 0;
	int lptnum = 0;
	for (int i = 1; i <= 9; ++i)
	{
		if (PosPrinterLptCls::CanOpenLPT(i))
		{
			char tmpbuf[50];
			sprintf(tmpbuf, "LPT%d", i);
			ui.cboPrinter->addItem(tmpbuf);
			if (lptlastchar == (i + 48))
			{
				printIndex += lptnum;
			}
			++lptnum;
		}
	}
	if (printIndex < 0)
		printIndex = 0;
	ui.cboPrinter->setCurrentIndex(printIndex);
	connect(ui.cboPrinter, SIGNAL(currentIndexChanged(int)), this, SLOT(printerChanged(int)));
	printerChanged(printIndex);
	// 打印设置
	ui.cboSize->setCurrentIndex(carishInfo.printerType);
	ui.printFont->setValue(carishInfo.commentFontSZ);
	// 是否自动打印
	ui.chbAutioPrint->setChecked(shopInfo.isAutoPrint == 1);

	// 快捷键界面
	QStringList hotkeyLs;
	codeSetIO::HOTKEYS &pHotKeys = mZHSetting.hotKeys;
	int oriAscii[2], curIndexs[2];
	oriAscii[0] = pHotKeys.hPrintHandover.qtkey;
	oriAscii[1] = pHotKeys.hTradeInfo.qtkey;
	asciiIntoIndex(hotkeyLs, 2, oriAscii, curIndexs);
	ui.cboPrintHandover->addItems(hotkeyLs);
	ui.cboPrintHandover->setCurrentIndex(curIndexs[0]);
	ui.cboTradeInfo->addItems(hotkeyLs);
	ui.cboTradeInfo->setCurrentIndex(curIndexs[1]);
}

int MainDlg::getImageScaleTag(float &outScale)
{
	switch (mZHSetting.carishInfo.selectRange.priceImageScaleTag)
	{
	case 0:  // 表示不放大
		outScale = 0;
		break;
	case 1:
		outScale = 1.2f;
		break;
	case 2:
		outScale = 1.3f;
		break;
	case 3:
		outScale = 1.5f;
		break;
	case 4:
		outScale = 2.0f;
		break;
	}
	return mZHSetting.carishInfo.selectRange.priceImageScaleTag;
}

void MainDlg::cashToolChanged(int newIndex)
{
	bool hasCashTool = (newIndex != 0);
	ui.btnCheck->setEnabled(hasCashTool);
	ui.gbMoneyPos->setEnabled(hasCashTool);
}

void MainDlg::printerChanged(int newIndex)
{
	bool userPrinter = (newIndex != 0);
	ui.gbPrinter->setEnabled(userPrinter);
	ui.widPrinter2->setEnabled(userPrinter);
}

inline void MainDlg::asciiIntoIndex(QStringList &ioHotkeyLs, int tabNum, int *inASCII, int *outIndex)
{
	ioHotkeyLs << QString::fromLocal8Bit("无"); // -1 index 0
	for (char i = 'A'; i <= 'Z'; ++i)  // ascii 65~~90 index 1~~26
	{
		ioHotkeyLs << QString(i);
	}
	for (char i = '0'; i <= '9'; ++i) // ascii 48~~57  index 27~~36
	{
		ioHotkeyLs << QString(i);
	}
	ioHotkeyLs << "Left" << "Up" << "Right" << "Down"; // ascii 37~~40 index 37~~40

	for (int i = 0; i < tabNum; ++i)
	{
		if (inASCII[i] == -1)
			outIndex[i] = 0;
		else if (inASCII[i] >= 65 && inASCII[i] <= 90)
			outIndex[i] = inASCII[i] - 64;
		else if (inASCII[i] >= 48 && inASCII[i] <= 57)
			outIndex[i] = inASCII[i] - 21;
		else if (inASCII[i] >= 37 && inASCII[i] <= 40)
			outIndex[i] = inASCII[i];
	}
}

inline void MainDlg::indexIntoAscii(int tabNum, int *inIdex, int *outASCII)
{
	for (int i = 0; i < tabNum; ++i)
	{
		if (inIdex[i] == 0)
			outASCII[i] = -1;
		else if (inIdex[i] >= 1 && inIdex[i] <= 26)
			outASCII[i] = inIdex[i] + 64;
		else if (inIdex[i] >= 27 && inIdex[i] <= 36)
			outASCII[i] = inIdex[i] + 21;
		else if (inIdex[i] >= 37 && inIdex[i] <= 40)
			outASCII[i] = inIdex[i];
	}
}

void MainDlg::catchScreen()
{
	if (!checkSoft())
		return;

	codeSetIO::CarishDesk &carishInfo = mZHSetting.carishInfo;
	carishInfo.selectRange.relitiveType = ui.cboMoneyPos->currentIndex();
	ScreenCatch dlg(this);
	connect(&dlg, SIGNAL(selectRectsig(const QRect &)), this, SLOT(catchScreen(const QRect &)));
	dlg.exec();
}

void MainDlg::catchScreen(const QRect &inselect)
{
	int nIndex = ui.cboCashTool->currentIndex();
	QString str = mWinWindowNames[nIndex];
	const wchar_t * encodedName = reinterpret_cast<const wchar_t *>(str.utf16());   
	HWND hwnd = ::FindWindow(NULL, encodedName);
	if (hwnd == NULL)
	{
		QString str1 = mWinClassNames[nIndex];
		encodedName = reinterpret_cast<const wchar_t *>(str1.utf16());  
		hwnd = ::FindWindow(encodedName, NULL);
	}
	RECT mCarishtRect;
	::GetWindowRect(hwnd, &mCarishtRect);

	if (inselect.width() <= 0 || inselect.height() <= 0)
		return;
	codeSetIO::SelectRange &imageSel = mZHSetting.carishInfo.selectRange;
	int selectCenterPosX = inselect.x() + inselect.width() * 0.5;
	int selectCenterPosY = inselect.y() + inselect.height() * 0.5;
	switch (imageSel.relitiveType)  // 选框右上角相对于收银软件的位置
	{
	case 0:
		imageSel.xCenterDistance = selectCenterPosX - mCarishtRect.left;
		imageSel.yCenterDistance = selectCenterPosY - mCarishtRect.top;
		break;
	case 1:
		imageSel.xCenterDistance = mCarishtRect.right - selectCenterPosX;
		imageSel.yCenterDistance = mCarishtRect.bottom - selectCenterPosY;
		break;
	case 2:
		imageSel.xCenterDistance = selectCenterPosX -mCarishtRect.left;
		imageSel.yCenterDistance = mCarishtRect.bottom - selectCenterPosY;
		break;
	case 3:
		imageSel.xCenterDistance = mCarishtRect.right - selectCenterPosX;
		imageSel.yCenterDistance = mCarishtRect.bottom - selectCenterPosY;
		break;
	case 4:
		imageSel.xCenterDistance = selectCenterPosX - (mCarishtRect.right + mCarishtRect.left) * 0.5;
		imageSel.yCenterDistance = selectCenterPosY - (mCarishtRect.bottom + mCarishtRect.top) * 0.5;
		break;
	default:
		break;
	}

	imageSel.widImage = inselect.width();
	imageSel.heightImage = inselect.height();

	std::vector<int > ids;
	ZHFuncLib::GetTargetProcessIds(OCREXE, ids);
	if (ids.size() == 0)
	{
		Json::Value mValData;
		mValData[PRO_HEAD] = TO_FLOATWIN_STARTOCR;

		HWND hwnd = ::FindWindowW(NULL, FLOATWINTITLEW);
		ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
	}
}

bool MainDlg::checkSoft()
{
	int nIndex = ui.cboCashTool->currentIndex();
	setActualTimeGetPrice(nIndex != 0);
	if (mZHSetting.shopCashdestInfo.isGetPriceActualTime == 1)
	{
		QString strWinName = mWinWindowNames[nIndex];
		const wchar_t * encodedName = reinterpret_cast<const wchar_t *>(strWinName.utf16());   
		HWND hwnd = ::FindWindow(NULL, encodedName);
		if (hwnd == NULL)
		{
			QString str1 = mWinClassNames[nIndex];
			encodedName = reinterpret_cast<const wchar_t *>(str1.utf16());   
			hwnd = ::FindWindow(encodedName, NULL);
		}
		if (hwnd == NULL) 
		{
			//QMessageBox::about(this, QString::fromLocal8Bit("警告"), strWinName + QString::fromLocal8Bit("未打开"));
			showTipDialogOK(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("未打开"));

			return false;
		}
		if (::IsWindowVisible(hwnd) == FALSE)
		{
			//QMessageBox::about(this, QString::fromLocal8Bit("警告"), strWinName + QString::fromLocal8Bit("窗口隐藏"));
			showTipDialogOK(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("窗口隐藏"));
			return false;
		}
		ui.labSoftTag->setText(QString::fromLocal8Bit("正确"));
		ui.labSoftTag->show();
		codeSetIO::CarishDesk &deskInfo = mZHSetting.carishInfo;

		std::string namcchar = ZHFuncLib::WstringToString(strWinName.toStdWString());
		int namelen = namcchar.length();
		memcpy(deskInfo.windowName, namcchar.c_str(), namelen);
		deskInfo.windowName[namelen] = 0;
		QTimer::singleShot(3000, ui.labSoftTag, SLOT(hide()) ); 
	}

	return true;
}

void MainDlg::setActualTimeGetPrice(bool isActualTime)
{
	if (!isActualTime){
		codeSetIO::CarishDesk &deskInfo = mZHSetting.carishInfo;
		mZHSetting.shopCashdestInfo.isGetPriceActualTime = 0;
		deskInfo.windowName[0] = 0;
	}
	else{
		mZHSetting.shopCashdestInfo.isGetPriceActualTime = 1;
	}
}

void MainDlg::showTipDialogOK(int icon, const QString &inTitle, const QString &inTxt)
{
	QMessageBox box((QMessageBox::Icon)icon, inTitle, inTxt);
	box.setStandardButtons (QMessageBox::Ok);
	box.setButtonText (QMessageBox::Ok,QString::fromLocal8Bit("确 定"));
	box.setWindowFlags(Qt::WindowStaysOnTopHint);
	box.setWindowFlags(box.windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowMinimizeButtonHint);
	box.setDefaultButton(QMessageBox::Ok);

	QRect thisRect = this->geometry();
	int iWidth = thisRect.width();
	int iHeight = thisRect.height();
	QRect boxRect = box.geometry();
	int posx = x() + iWidth - boxRect.width() * 0.6;
	int posy = y() + iHeight - boxRect.height() * 0.6;
	box.setGeometry(posx, posy, box.width(), box.height());

	int typeBtn = box.exec();
}

void MainDlg::checkPrice()
{
	catchScreenInfo();
	Json::Value mValData;
	mValData[PRO_HEAD] = TO_OCR_CASHINFO;
	mValData[PRO_OCR_FROMDLG] = PRO_OCR_FROM_MAINDLG;
	codeSetIO::CarishDesk &carishInfo = mZHSetting.carishInfo;
	codeSetIO::SelectRange &curSelRange = carishInfo.selectRange;
	ZBase64 base64;
	mValData[PRO_OCR_CASHNAME] = base64.Encode(carishInfo.windowName, strlen(carishInfo.windowName));
	mValData[PRO_OCR_REALITIVETYPE] = curSelRange.relitiveType;
	mValData[PRO_OCR_SELX] = curSelRange.xCenterDistance;
	mValData[PRO_OCR_SELY] = curSelRange.yCenterDistance;
	mValData[PRO_OCR_SELW] = curSelRange.widImage;
	mValData[PRO_OCR_SELH] = curSelRange.heightImage;
	mValData[PRO_OCR_SCALETAG] = curSelRange.priceImageScaleTag;
	mValData[PRO_OCR_BINA] = curSelRange.imageBinaryzation;

	HWND hwnd = ::FindWindowW(NULL, OCRDLGTITLEW);
	ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
}

void MainDlg::catchScreenInfo()
{
	codeSetIO::SelectRange &selectRange = mZHSetting.carishInfo.selectRange;
	selectRange.imageBinaryzation = ui.chbBinarization->checkState();
	selectRange.priceImageScaleTag = ui.cboImageScale->currentIndex();
}

bool MainDlg::nativeEvent(const QByteArray & eventType, void * message, long * result)
{
	MSG *param = static_cast<MSG *>(message);

	switch (param->message)
	{
	case WM_COPYDATA:
		{
			COPYDATASTRUCT *cds = reinterpret_cast<COPYDATASTRUCT*>(param->lParam);
			if (cds->dwData == ZHIHUI_CODE_MSG)
			{
				QString strMessage = QString::fromUtf8(reinterpret_cast<char*>(cds->lpData), cds->cbData);
				parseProcessJsonData(strMessage);
				*result = 1;
				return true;
			}
		}
	}

	return QWidget::nativeEvent(eventType, message, result);
}

inline void MainDlg::parseProcessJsonData(QString inJson)
{
	Json::Reader reader;
	Json::Value value;
	bool suc = reader.parse(inJson.toStdString(), value);
	if (!suc)
		return;
	int header = value[PRO_HEAD].asInt();
	switch (header)
	{
	case RETURN_PRICE:
		showPrice(value);
		break;
	default:
		break;
	}
}

inline void MainDlg::showPrice(const Json::Value &inJson)
{
	QString pricestr = inJson[PRO_OCR_PRICE].asCString();
	ui.txtPrice->setText(pricestr);
}

void MainDlg::commitSlot()
{
	codeSetIO::ShopCashdeskInfo &shopInfo = mZHSetting.shopCashdestInfo;

	// 获得输入的门牌号
	Json::Value item;
	item["shopid"] = ui.shopNumTxt->text().toStdString();

	std::string itemVal = item.toStyledString();
	std::string::size_type rePos;
	while ((rePos = itemVal.find(" ")) != -1) {
		itemVal.replace(rePos, 1, "");
	}

	urlServer->GetDataFromServer("api/app/v1", SETTINGCOMMITAPI, itemVal, URL_SETTING_DLG_COMMIT);
}

void MainDlg::bindSlot()
{
	if (mCashNos.size() < 1)
	{
		showTipDialogOK(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先提交"));
		return;
	}
	emit showBindTipSig(true);

	// shop id
	QString shopID = ui.shopNumTxt->text();
	Json::Value root;
	root["shop_id"] = shopID.toStdString();
	char str[25];
	int cashNo = mCashNos[ui.cboCashNo->currentIndex()];
	itoa(cashNo, str, 10);
	root["cashdesk_id"] = str;
	root["android_type"] = "1";
	char  mac[MACADDRLEN];  
	ZHFuncLib::NativeLog("", "SettingWidget::bindSlot", "a");
	urlServer->GetMAC(mac);
	root["android_no"] = mac;
	codeSetIO::ShopCashdeskInfo &deskInfo = mZHSetting.shopCashdestInfo;
	root["cashier_account"] = deskInfo.account;
	urlServer->GetDataFromServer1(URLCLOUND, SETTINGBINDAPI, "", root, URL_SETTING_DLG_BIND);

}

bool MainDlg::isReturnSuccessFromeServer(const Json::Value &pjsonVal)
{
	std::string retCode = pjsonVal["return_code"].asString();
	std::string resCode = pjsonVal["result_code"].asString();
	bool isReturnSuc = !(retCode == "FAIL" || resCode == "FAIL" || retCode == "fail" || resCode == "fail" );
	return isReturnSuc;
}

inline void MainDlg::urlbackOnCommit(const Json::Value &inVal)
{
	bool isReturnSuc = isReturnSuccessFromeServer(inVal);
	bool hasReturnMsg = inVal.isMember("return_msgs");
	if (isReturnSuc)
	{
		setCashInfo(inVal["data"]);
	}
	if (!hasReturnMsg)
	{
		Json::Value mValData;
		mValData[PRO_HEAD] = TO_SHOWTIP;
		mValData[PRO_TIPSTR] = (isReturnSuc ? QString::fromLocal8Bit("提交成功。").toStdString() : QString::fromLocal8Bit("提交失败。").toStdString());
		HWND hwnd = ::FindWindowW(NULL, FLOATWINTITLEW);
		ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
	}
}

inline void MainDlg::urlbackOnBind(const Json::Value &inVal)
{
	bool isReturnSuc = isReturnSuccessFromeServer(inVal);
	std::string dcdev_mac = inVal["dcdev_mac"].asString();
	std::string dcdev_no = inVal["dcdev_no"].asString();
	if (isReturnSuc)
	{
		emit showBindTipSig(false);
		Json::Value mValData;
		mValData[PRO_HEAD] = TO_FLOATWIN_RECONNECT_SOCKET;
		HWND hwnd = ::FindWindowW(NULL, FLOATWINTITLEW);
		ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
	}
	else
	{
		ZHFuncLib::NativeLog("", inVal.toStyledString().c_str(), "a");
		return;
	}
	//CString tmpstr;
	//tmpstr.Format(L"dcdev mac: %s     ,dcdev no: %s ", pWnd->CharToWChar(dcdev_mac.c_str()), pWnd->CharToWChar(dcdev_no.c_str()));
	//ZHMainDlg::InitInstance(false)->SetStatus(tmpstr);

	codeSetIO::ShopCashdeskInfo &shopInfo = mZHSetting.shopCashdestInfo;
	const char *tmpChar = inVal["shop_code"].asCString();
	memcpy(shopInfo.shopCode, tmpChar, strlen(tmpChar));
	shopInfo.shopCode[strlen(tmpChar)] = 0;
	tmpChar = inVal["shop_name"].asCString();
	memcpy(shopInfo.shopName, tmpChar, strlen(tmpChar));
	shopInfo.shopName[strlen(tmpChar)] = 0;
	tmpChar = inVal["cashdesk_id"].asCString();
	memcpy(shopInfo.cashdeskId, tmpChar, strlen(tmpChar));
	shopInfo.cashdeskId[strlen(tmpChar)] = 0;
	tmpChar = inVal["cashdesk_name"].asCString();
	memcpy(shopInfo.cashdeskName, tmpChar, strlen(tmpChar));
	shopInfo.cashdeskName[strlen(tmpChar)] = 0;
	tmpChar = inVal["dcdev_no"].asCString();
	memcpy(shopInfo.dcdevNo, tmpChar, strlen(tmpChar));
	shopInfo.dcdevNo[strlen(tmpChar)] = 0;
	tmpChar = inVal["dcdev_mac"].asCString();
	memcpy(shopInfo.dcdevMac, tmpChar, strlen(tmpChar));
	shopInfo.dcdevMac[strlen(tmpChar)] = 0;
	shopInfo.shopid = ui.shopNumTxt->text().toInt();
	shopInfo.isBind = isReturnSuc;

	ui.cashNoTxt->setText(shopInfo.cashdeskName);
}

void MainDlg::setCashInfo(const Json::Value &inData)
{
	mCashNos.clear();
	mCashNames.clear();
	disconnect(ui.cboCashNo, SIGNAL(currentIndexChanged(int)), this, SLOT(cashNoChanged(int)));
	ui.cboCashNo->clear();
	connect(ui.cboCashNo, SIGNAL(currentIndexChanged(int)), this, SLOT(cashNoChanged(int)));
	ui.cashNoTxt->setText("");

	const char *shopname = inData["SHOP_NAME"].asCString();
	ui.shopNameTxt->setText(shopname);

	Json::Value cashObj = inData["CASH_LIST"];
	int sz = cashObj.size();
	for (unsigned int i = 0; i < sz; i++)
	{
		Json::Value noInfo = cashObj[i];
		int id = noInfo["ID"].asInt();
		const char *cashNoName = noInfo["CASH_NAME"].asCString();
		mCashNos.push_back(id);
		mCashNames.push_back(cashNoName);
		char string[255];
		sprintf(string, "(%d)%s", id, cashNoName);
		ui.cboCashNo->addItem(string);
	}
	if (sz != 0)
	{
		ui.cboCashNo->setCurrentIndex(0);
		cashNoChanged(0);
	}
}

void MainDlg::cashNoChanged(int newIndex)
{
	codeSetIO::ShopCashdeskInfo &shopInfo = mZHSetting.shopCashdestInfo;
	if (newIndex < 0)
		return;
	sprintf(shopInfo.cashdeskId, "%d", mCashNos[newIndex]);
	memcpy(shopInfo.cashdeskName, mCashNames[newIndex].c_str(), mCashNames[newIndex].length());
}

void MainDlg::showTipSlot(bool isShow)
{
	if (isShow)
		ui.labBindTip->setText(QString::fromLocal8Bit("未绑定"));
	else
		ui.labBindTip->hide();
}

void MainDlg::saveSetting()
{
	checkSoft();
	memeryPrintName();
	catchScreenInfo();
	SavePrintFont(ui.cboSize->currentIndex(), ui.printFont->value());
	codeSetIO::ShopCashdeskInfo &cashdesk = mZHSetting.shopCashdestInfo;
	// 是否使用支付扫码枪
	cashdesk.isUsePayGun = ui.chbStartGun->isChecked() ? 1 : 0;
	// 是否自动打印
	cashdesk.isAutoPrint = ui.chbAutioPrint->isChecked() ? 1 : 0;

	// 保存快捷键
	int indexs[2], asciis[2];
	indexs[0] = ui.cboPrintHandover->currentIndex();
	indexs[1] = ui.cboTradeInfo->currentIndex();
	indexIntoAscii(2, indexs, asciis);
	codeSetIO::HOTKEYS &hotKeys = mZHSetting.hotKeys;
	hotKeys.hPrintHandover.qtkey = asciis[0];
	hotKeys.hTradeInfo.qtkey = asciis[1];

	SaveAllSetting();
}

void MainDlg::memeryPrintName()
{
	codeSetIO::CarishDesk &carishInfo = mZHSetting.carishInfo;
	if (ui.cboPrinter->currentIndex() > 0)
	{
		QString printerName = ui.cboPrinter->currentText();
		memcpy(carishInfo.printerName, printerName.toStdString().c_str(), printerName.length());
		carishInfo.printerName[printerName.length()] = 0;
	}
	else
	{
		carishInfo.printerName[0] = 0;
	}
}

void MainDlg::SavePrintFont(int printerType, int printerFont)
{
	codeSetIO::CarishDesk &carishInfo = mZHSetting.carishInfo;
	carishInfo.printerType = printerType;
	carishInfo.commentFontSZ = printerFont;
}

void MainDlg::SaveAllSetting()
{
	ZHSettingRW settingRW(mZHSetting);
	settingRW.WriteZHSetting();

	Json::Value mValData;
	mValData[PRO_HEAD] = TO_FLOATWIN_REWRITESETTING;
	mValData[PRO_DLG_STATUS] = 0;
	HWND hwnd =::FindWindowW(NULL, FLOATWINTITLEW);
	ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
}

void MainDlg::checkCashSoftCorrect()
{
	checkSoft();
}

void MainDlg::printerTest()
{
	memeryPrintName();

	codeSetIO::CarishDesk &carishInfo = mZHSetting.carishInfo;
	QString printerDevice = carishInfo.printerName;
	if (printerDevice.contains("LPT"))
	{
		char tmpchar = printerDevice.at(3).toLatin1();
		if (tmpchar >= '1' && tmpchar <= '3')
		{
			PosPrinterLptCls mPrinter;
			mPrinter.Prepare(printerDevice.toStdString().c_str());
			mPrinter.PrintString("测试您的小票打印机，如果您的打印机能够打印这些信息，则表明您的打印机已经可以正常使用！\r\n", PosPrinterLptCls::NORMAL);
			mPrinter.PrintString(" --------智慧微生活\r\n", PosPrinterLptCls::RIGHTALIGN);
			return;
		}
	}

	PosPrinter *mPrinter = new PosPrinter();
#define NAMEPOS 0.02
#define CONTENTPOS 0.45
#define SPACE1 10
	//USES_CONVERSION;
	//CString printerName = A2W(pWnd->mZHSetting.carishInfo.printerName);
	if (mPrinter->PreparePrinter(printerDevice.toStdWString()))
	{
		int commentFontSZ = mZHSetting.carishInfo.commentFontSZ;
		mPrinter->SetDeviceWidth(GetPrinterDeviceWidth());
		mPrinter->AddNewLine(NAMEPOS, L"测试您的小票打印机，如果您的打印机能够打印这些信息，则表明您的打印机已经可以正常使用！",SPACE1, commentFontSZ);
		mPrinter->AddNewLine(NAMEPOS, L"         --------智慧微生活", commentFontSZ);
		mPrinter->AddNewLine(0.5, ONLYLINE, SPACE1, 50, L"", 0);
#undef NAMEPOS
#undef CONTENTPOS
#undef SPACE1

		mPrinter->CallPrinter();
	}
	delete mPrinter;
}

int MainDlg::GetPrinterDeviceWidth()
{
	codeSetIO::CarishDesk &printer = mZHSetting.carishInfo;
	int deviceWidth = -1;
	switch (printer.printerType)
	{
	case 0:
		deviceWidth = printer.deviceWidth58;
		break;
	case 1:
		deviceWidth = printer.deviceWidth80;
		break;
	default:
		break;
	}
	if (deviceWidth > 1000)
		return -1;
	if (deviceWidth < 100)
		return -1;
	return deviceWidth;
}

void MainDlg::clickClear()
{
	QString strDirName = ZHFuncLib::GetWorkPath().c_str();
	QDir dir(strDirName);
	if (!dir.exists())
		return;
	// clear log and moc info
	dir.setFilter(QDir::Dirs|QDir::Files);
	dir.setSorting(QDir::DirsFirst);
	QFileInfoList list = dir.entryInfoList();
	int i = 0;
	//QStringList fileWillRemove;
	do{
		QFileInfo fileInfo = list.at(i);
		++i;
		QString fName = fileInfo.fileName();
		if(fName == "." || fName == ".." || fileInfo.isDir())
			continue;
		if (fName.right(3) == "exe")
			continue;
		if (fName.contains("logdata"))
		{
			//fileWillRemove.push_back(fName);
			dir.remove(fName);
		}
	}while(i < list.size());

	//for (int i = 0; i < fileWillRemove.size(); ++i)
	//{
	//	dir.remove(fileWillRemove.at(i));
	//}
	// clear install exe
	int searchRes = -1;

	QString bbqDir = urlServer->SearchBBQDownloadDir(searchRes).c_str();
	if (searchRes == 0)
	{
		dir.setPath(bbqDir);
		QFileInfoList list = dir.entryInfoList();
		int i = 0;
		do{
			QFileInfo fileInfo = list.at(i);
			++i;
			QString fName = fileInfo.fileName();
			if(fName == "." || fName == ".." || fileInfo.isDir())
				continue;
			dir.remove(fName);
		}while(i < list.size());
		dir.rmdir(bbqDir);
	}

	Json::Value mValData;
	mValData[PRO_HEAD] = TO_SHOWTIP;
	mValData[PRO_TIPSTR] = QString::fromLocal8Bit("清理缓存完成。").toStdString();
	HWND hwnd = ::FindWindowW(NULL, FLOATWINTITLEW);
	ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
}
