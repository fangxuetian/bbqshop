#include "ExcelTestDemo.h"
#include <QAxObject>

ExcelTestDemo::ExcelTestDemo(QString filePath, QObject *parent)
	: QObject(parent)
{
	QAxObject excel("Excel.Application"); 
	excel.setProperty("Visible", false); //false����ʾ����
	QAxObject *work_books = excel.querySubObject("WorkBooks"); 
	work_books->dynamicCall("Open (const QString&)", filePath);
	//QVariant title_value = excel.property("Caption");  //��ȡ����
}

ExcelTestDemo::~ExcelTestDemo()
{

}
