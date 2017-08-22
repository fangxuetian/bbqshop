#include "excetest.h"
#include <QFileDialog>
#include <QAxObject>

ExceTest::ExceTest(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pbtSelect, SIGNAL(released()), this, SLOT(onFileSelect()));
}

ExceTest::~ExceTest()
{
	applicationExcel->dynamicCall("Quit()");
}

void ExceTest::onFileSelect()
{
	//QString filepath=QFileDialog::getSaveFileName(this,tr("Save orbit"),".",tr("Microsoft Office 2007 (*.xlsx)"));//��ȡ����·��
	//if(filepath.isEmpty())
	//{
	//	return;
	//}
	QString filepath = "D:\\test2.xls";
	applicationExcel = new QAxObject("Excel.Application", this);
	QAxObject *workBooks = applicationExcel->querySubObject("Workbooks");
	createExcel(applicationExcel, filepath);
	QAxObject *workBook = applicationExcel->querySubObject("ActiveWorkBook");
	workSheets = workBook->querySubObject("WorkSheets");
	
	// ��ʼ����
	curSheet = workSheets->querySubObject("Item(const QString&)", "������");
	QAxObject *curCell = curSheet->querySubObject("Cells(int,int)", 0, 0);
	curCell->dynamicCall("SetValue(const QString&)", "ʱ����Ա");
}

bool ExceTest::createExcel(QAxObject *excelAX, QString file)
{
	QDir  dTemp;

	if(dTemp.exists(file))
	{
		//qDebug()<<" QExcel::CreateExcel: exist file"<<file;
		return false;
	}

	//qDebug()<<" QExcel::CreateExcel: succes";

	/**< create new excel sheet file.*/
	QAxObject * workSheet = excelAX->querySubObject("WorkBooks");
	workSheet->dynamicCall("Add");

	/**< save Excel.*/
	QAxObject * workExcel= excelAX->querySubObject("ActiveWorkBook");
	excelAX->setProperty("DisplayAlerts", 1);
	workExcel->dynamicCall("SaveAs (const QString&,int,const QString&,const QString&,bool,bool)",file,56,QString(""),QString(""),false,false);
	excelAX->setProperty("DisplayAlerts", 1);
	workExcel->dynamicCall("Close (Boolean)", false);

	/**< exit Excel.*/
	//excel->dynamicCall("Quit (void)");

	return true;
}