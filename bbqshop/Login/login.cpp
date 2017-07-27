﻿#include "login.h"
#include "zhfunclib.h"
#include "MD5.h"
#include "ProcessProtocal.h"
#include "ZhuiHuiMsg.h"
#include "AllExeName.h"

Login::Login(QApplication *pApp, QWidget *parent)
	: QDialog(parent), mainApp(pApp)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("登录"));
	setFixedSize(size());
	urlServer = new BbqUrlServer(this);
	initFrame();
	getLoginInfo();
	ui.pbtLogin->setFocus();
	ui.pbtLogin->setShortcut( QKeySequence::InsertParagraphSeparator ); //设置快捷键为键盘的“回车”键
	ui.pbtLogin->setShortcut(Qt::Key_Enter); //设置快捷键为enter键
	ui.pbtLogin->setShortcut(Qt::Key_Return); //设置快捷键为小键盘上的enter键	
}

Login::~Login()
{
}

void Login::initFrame()
{
	QRect dlgrect = this->geometry();
	// logo
	QString pngPath = ZHFuncLib::GetWorkPath().c_str();
	pngPath += "/res/logo.png";
	QPixmap img(pngPath);
	ui.labicon->setPixmap(img);

	// 账号框只允许输入数字
	QRegExp rx("^[0-9]{1,11}$");//这个11就是最大长度
	QValidator *validator = new QRegExpValidator(rx,0);
	ui.cboAccount->setValidator(validator);
	QString styl = "font: bold 20px;";
	ui.cboAccount->setStyleSheet(styl);
	//ui.cboAccount->setMa
	// 密码框设置
	ui.ledtPWD->setEchoMode(QLineEdit::Password);
	// version
	ui.labVersion->setText(ZHCLIENTVERSION);

	pngPath = ZHFuncLib::GetWorkPath().c_str();
	pngPath += "/res/pc_btn_g.bmp";
	ui.pbtLogin->setStyleSheet("border-image: url(" + pngPath + ");color: #FFFFFF;font: 12pt \"Arial\";");

	setStyleSheet("QDialog{background-color: #F0F0F0}");
	// 登录按钮
	isLogining = false;
	connect(ui.pbtLogin, SIGNAL(released()), this, SLOT(clickLogin()));
	connect(this, SIGNAL(loginStatus(bool )), this, SLOT(onLoginStatus(bool )));
}

void Login::getLoginInfo()
{
	int isfirstlogin;
	int usrNum = 0;
	int usrLastIndex = 0;
	AccountNumber acount;
	acount.ReadUsrInfo(mUsrs, isfirstlogin, usrNum, usrLastIndex);

	int sz = mUsrs.size();
	for (int i = 0; i < sz; ++i)
	{
		AccountNumber::USERPWD *usrpwd = mUsrs[i];
		char *usraccount = acount.Decode(usrpwd->userAccount);
		ui.cboAccount->addItem(usraccount);
	}
	ui.cboAccount->setCurrentIndex(usrLastIndex);
	if (sz > 0)
	{
		AccountNumber::USERPWD *usrpwd = mUsrs[usrLastIndex];
		char *pwdchar = acount.Decode(usrpwd->pwd);
		ui.ledtPWD->setText(pwdchar);
	}
}

void Login::showTipString(const QString &inTip)
{
	Json::Value mValData;
	mValData[PRO_HEAD] = TO_SHOWTIP;
	mValData[PRO_TIPSTR] = inTip.toStdString().c_str();
	HWND hwnd = ::FindWindowW(NULL, FLOATWINTITLEW);
	ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
}

void Login::clickLogin()
{
	if (isLogining)
	{
		// 提示
		showTipString(QString::fromLocal8Bit("正在登录中"));
		return;
	}
	Json::Value mValData;
	QString tmp = ui.cboAccount->currentText();
	tmp = tmp.trimmed();
	if (tmp.isEmpty())
	{
		showTipString(QString::fromLocal8Bit("账号不能为空"));
		return;
	}
	ui.cboAccount->setCurrentText(tmp);
	mValData["username"] = tmp.toStdString();
	tmp = ui.ledtPWD->text();
	if (tmp.isEmpty())
	{
		//parPay->ShowTipDialogOK(QMessageBox::Warning, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("密码不能为空"), this);
		showTipString(QString::fromLocal8Bit("密码不能为空"));
		return;
	}
	mValData["password"] = md5(tmp.toStdString());

	std::string itemVal = mValData.toStyledString();
	std::string::size_type rePos;
	while ((rePos = itemVal.find(" ")) != -1) {
		itemVal.replace(rePos, 1, "");
	}
	isLogining = true;
	ui.pbtLogin->setText(QString::fromLocal8Bit("正在登录"));
	urlServer->GetDataFromServer("api/app/v1", USERLOGINAPI, itemVal, URL_LOGIN_DLG);
}

void Login::onLoginStatus(bool isLogined)
{
	if (isLogined)
	{
		AccountNumber acount;
		QString username = ui.cboAccount->currentText();
		AccountNumber::USERPWD usrpwd;
		acount.Incode(usrpwd.userAccount, username.toStdString().c_str());
		QString pwd = ui.ledtPWD->text();
		acount.Incode(usrpwd.pwd, pwd.toStdString().c_str());

		int lastUsr = 0;
		acount.AddOrModifyUsr(mUsrs, const_cast<AccountNumber::USERPWD *>(&usrpwd), lastUsr);
		acount.WriteUsrInfo(mUsrs, 1, lastUsr, -1);
		acount.ClearUsersVector(mUsrs);
		accept();
		mainApp->quit();
	}
}

bool Login::DealWithJSONFrServer(std::string mRecvJsonStr, int urlTag, std::string urlApi)
{
	isLogining = false;
	Json::Reader reader;
	Json::Value value;
	bool suc = reader.parse(mRecvJsonStr, value);
	if (suc)
	{
		ui.pbtLogin->setText(QString::fromLocal8Bit("登录"));
		if (urlTag == URL_LOGIN_DLG)
		{
			std::string retCode = value["return_code"].asString();
			std::string resCode = value["result_code"].asString();
			std::string retmsg = value["return_msgs"].asString();
			if (retCode == "FAIL" || resCode == "FAIL" || retCode == "fail" || resCode == "fail")
			{
				const char *msg = value["return_msgs"].asCString();
				showTipString(msg);
			}else
			{
				Json::Value data = value["data"];
				loginInfoStore(data);
				emit loginStatus(true);
			}
		}
	}
	return suc;
}

void Login::CurlError(std::string url, int res, int urlTag)
{
	ui.pbtLogin->setText(QString::fromLocal8Bit("登录"));
	isLogining = false;
	showTipString(QString::fromLocal8Bit("网络异常，请检查网络！"));
}

void Login::loginInfoStore(const Json::Value &value)
{
	Json::Value mValData = value;
	mValData[PRO_HEAD] = TO_FLOATWIN_LOGININFO;
	HWND hwnd = ::FindWindowW(NULL, FLOATWINTITLEW);
	ZHFuncLib::SendProcessMessage((HWND)this->winId(), hwnd, ZHIHUI_CODE_MSG, mValData.toStyledString());
	urlServer->RecordMemoryInfo("Login success", LOG_DEBUG, LOG_LOGIN, URL_RECORE_LOGIN_MEMORY);
}

void Login::closeEvent(QCloseEvent * e) // 直接点的关闭按钮，此时未点登陆按钮
{
	ZHFuncLib::TerminateProcessExceptCurrentOne(BBQSHOPEXE);
	ZHFuncLib::TerminateProcessExceptCurrentOne(MAINDLGEXE);
	ZHFuncLib::TerminateProcessExceptCurrentOne(OCREXE);
	ZHFuncLib::TerminateProcessExceptCurrentOne(UPGRADEEXE);
	ZHFuncLib::TerminateProcessExceptCurrentOne(UPGRADECLIENTICONEXE);

	mainApp->quit();
}