#include "ScreenCatch.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QBitmap>
#include "zhfunclib.h"

ScreenCatch::ScreenCatch(QWidget *parent)
	: QDialog(parent)
{
	setWindowState(Qt::WindowFullScreen);

	isPressed = false;
	isMoved = false;
	rubberBand = NULL;
	_pixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
}

ScreenCatch::~ScreenCatch()
{

}

void ScreenCatch::paintEvent(QPaintEvent *event)  
{  
	QPainter painter(this);  
	_pixmap = _pixmap.scaled(width(),height(),Qt::KeepAspectRatio);  

	//pixmap_û�� alphaͨ�� ���ͨ��  
	QPixmap temp(_pixmap.size());  
	temp.fill(Qt::transparent);  

	QPainter p(&temp);  
	p.setCompositionMode(QPainter::CompositionMode_Source);  
	p.drawPixmap(0, 0, _pixmap);  
	p.setCompositionMode(QPainter::CompositionMode_DestinationIn);  
	//p.fillRect(temp.rect(), QColor(50, 50, 50, 100)); //��ͼƬ�� �� ����ʾ��ͼȫ��  

	//ˮӡ
	painter.drawPixmap(0,0,temp);  
}  

void ScreenCatch::mousePressEvent(QMouseEvent *event)  
{  
	isPressed = true;
	//��ȡ����
	pStart_ = event->pos();

	if (!rubberBand)
	{
		rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
	}
	rubberBand->setGeometry(QRect(pStart_,QSize()));
	rubberBand->show();
}  

void ScreenCatch::mouseMoveEvent(QMouseEvent *event)
{  
	if (rubberBand)
	{
		isMoved = true;
		rubberBand->setGeometry(QRect(pStart_, event->pos()).normalized());
	}
}

void ScreenCatch::mouseReleaseEvent(QMouseEvent *event)  
{  
	isPressed = false;
	if (rubberBand)
	{
		//��ȡ��Ƥ������ֹ����
		if (isMoved)
		{
			pEnd = event->pos();
			selectedRect = CoordinateCorrecte(pStart_, pEnd);
			isMoved = false;
		}
	}
}

// ������������
QRect ScreenCatch::CoordinateCorrecte(QPoint startPoint, QPoint endPoint)
{
	QRect * rect;
	// ���һ������ �� ����
	if (startPoint.x()<endPoint.x() && startPoint.y()<endPoint.y())
	{
		rect = new QRect(startPoint.x(), startPoint.y(), endPoint.x()-startPoint.x(), endPoint.y()-startPoint.y());
	}

	// ����������� �� ����
	if (startPoint.x()>endPoint.x() && startPoint.y()>endPoint.y())
	{
		rect = new QRect(endPoint.x(), endPoint.y(), startPoint.x()-endPoint.x(), startPoint.y()-endPoint.y());
	}

	// ����������� �� ����
	if (startPoint.x()<endPoint.x() && startPoint.y()>endPoint.y())
	{
		rect = new QRect(startPoint.x(), endPoint.y(), endPoint.x()-startPoint.x(), startPoint.y()-endPoint.y());
	}

	// ����ģ����� �� ����
	if (startPoint.x()>endPoint.x() && startPoint.y()<endPoint.y())
	{
		rect = new QRect(endPoint.x(), startPoint.y(), startPoint.x()-endPoint.x(), endPoint.y()-startPoint.y());
	}

	return * rect;
}

void ScreenCatch::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		rubberBand->hide();
		emit selectRectsig(selectedRect);
		// QScreen ����
		//QScreen * screen = QGuiApplication::primaryScreen();
		//// ��ͼ
		//QPixmap img = screen->grabWindow(0,selectedRect.x(),selectedRect.y(),selectedRect.width(),selectedRect.height());
		//QString fllpath = ZHFuncLib::GetWorkPath().c_str();
		//fllpath += "/test.png";
		//// ����
		//img.save(fllpath);
		accept();
	}
}
