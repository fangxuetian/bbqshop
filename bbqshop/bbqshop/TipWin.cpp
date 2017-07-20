#include "TipWin.h"
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QApplication>

TipWin::TipWin(QString inTip, QString inTitle, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	// ��ȥ��������λ��
	QRect rectExceptTast = QApplication::desktop()->availableGeometry();
	// λ������
	QRect tmpRect = this->geometry();
	int iWidth = tmpRect.width();
	int iHeight = tmpRect.height();
	QDesktopWidget *desktop = QApplication::desktop();
	QRect screen = desktop->screenGeometry();
	int tarskHeight = screen.height() - rectExceptTast.height();
	int xpos = screen.width() - iWidth - 10;
	int ypos = screen.height() - iHeight - tarskHeight;

	this->setGeometry(xpos, ypos, iWidth, iHeight);

	setWindowTitle(inTitle);
	ui.labTip->setText(inTip);

	QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
	animation->setDuration(1000);
	animation->setStartValue(0);
	animation->setEndValue(1);
	animation->start();

	QTimer::singleShot(3000,this,SLOT(close())); 
}

TipWin::~TipWin()
{
	emit closeTipWin();
}
