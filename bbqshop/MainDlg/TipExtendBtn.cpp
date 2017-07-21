#include "TipExtendBtn.h"
#include "zhfunclib.h"
//#include <QToolTip>

TipExtendBtn::TipExtendBtn(QWidget *parent, const QString &normalIcon, const QString &hoverIcon)
	: QPushButton(parent)
{
	thisNormalIcon = normalIcon;
	thisHoverIcon = hoverIcon;
	QString pngpath = ZHFuncLib::GetWorkPath().c_str();
	pngpath += normalIcon;
	QString leftBtnStyle = "QPushButton {";
	leftBtnStyle += "qproperty-icon: url(" + pngpath + ");";
	leftBtnStyle += "qproperty-iconSize: 25px 25px;";
	leftBtnStyle += "background-color:transparent;";
	leftBtnStyle += "}";
	setStyleSheet(leftBtnStyle);
	
}

TipExtendBtn::~TipExtendBtn()
{
}

void TipExtendBtn::leaveEvent(QEvent *event)    
{    
	//����뿪����ʱ����ͨ��ָ��    
	setCursor(Qt::ArrowCursor);
	QString pngpath = ZHFuncLib::GetWorkPath().c_str();
	pngpath += thisNormalIcon;
	QString leftBtnStyle = "QPushButton {";
	leftBtnStyle += "qproperty-icon: url(" + pngpath + ");";
	leftBtnStyle += "qproperty-iconSize: 25px 25px;";
	leftBtnStyle += "background-color:transparent;";
	leftBtnStyle += "}";
	setStyleSheet(leftBtnStyle);
}    

void TipExtendBtn::enterEvent(QEvent *event)    
{    
	//������ڴ�����ʱ��һ����ָ    
	setCursor(Qt::PointingHandCursor);
	QString pngpath = ZHFuncLib::GetWorkPath().c_str();
	pngpath += thisHoverIcon;
	QString leftBtnStyle = "QPushButton {";
	leftBtnStyle += "qproperty-icon: url(" + pngpath + ");";
	leftBtnStyle += "qproperty-iconSize: 25px 25px;";
	leftBtnStyle += "background-color:transparent;";
	leftBtnStyle += "}";
	setStyleSheet(leftBtnStyle);
}