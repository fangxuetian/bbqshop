#include "ocrprocess.h"
#include <QtWidgets/QApplication>
#include "zhfunclib.h"
#include "AllExeName.h"

int main(int argc, char *argv[])
{
	// �رճ������������������
	ZHFuncLib::TerminateProcessExceptCurrentOne(OCREXE);

	QApplication a(argc, argv);
	OCRProcess w;
	w.show();
	return a.exec();
}
