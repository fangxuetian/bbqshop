#include "login.h"
#include <QtWidgets/QApplication>
#include "zhfunclib.h"
#include "AllExeName.h"

int main(int argc, char *argv[])
{
	// �رճ������������������
	ZHFuncLib::TerminateProcessExceptCurrentOne(LOGINEXE);
	QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false);
	Login w;
	w.show();
	return a.exec();
}
