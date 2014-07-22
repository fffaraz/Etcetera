#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMessageBox>
#include <QEvent>
#include <QMainWindow>
#include <QProcess>
#include <QSettings>
#include <QString>
#include <QStringList>

#include "frmconfig.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT
public:

	static const int FIREWALL_HOME;
	static const int FIREWALL_PUBLIC;

	MainWindow(QWidget *parent = 0);
	~MainWindow();

	QString executeProgram(QString prog, QStringList args = QStringList(), bool silent = false);
	int getCurrentDefaultFirewall();

public slots:
		void changeDefaultFirewall(int firewall);


protected slots:
		void changeFirewall();
		void openFrmConfig();
		void openFrmAbout();
		void openFrmAboutQt();

protected:
    void changeEvent(QEvent *e);
		void closeEvent(QCloseEvent *event);
		void writeSettings();
		void readSettings();
		QString getUser();
		bool isRoot();
		void widgetEnable(bool enable);
		int getCurrentActiveFirewall();
		void displayCurrentFirewall(int currentFirewall);

private:
    Ui::MainWindow *ui;

		frmConfig *configForm;

		QStringList firewalls;
		int currentDefaultFirewall;
		int currentActiveFirewall;
};

#endif // MAINWINDOW_H
