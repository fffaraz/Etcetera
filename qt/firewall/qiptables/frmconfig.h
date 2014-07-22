#ifndef FRMCONFIG_H
#define FRMCONFIG_H

#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>

class MainWindow;

namespace Ui {
    class frmConfig;
}

class frmConfig : public QDialog {
    Q_OBJECT
public:
    frmConfig(QWidget *parent = 0);
    ~frmConfig();

		QString getCurrentDefaultFirewallName();
		int getCurrentDefaultFirewallType();

protected slots:
	void openFileDialog();
	void editFile();
	void btnSaveEnable();
	void saveText();
	void rebuildFirewall();

protected:
    void changeEvent(QEvent *e);
		void writeSettings();
		void readSettings();
		void openFileForEdit(QString filename);
		QString readFile(QString filename);

private:
    Ui::frmConfig *ui;
		MainWindow *mainForm;
};

#endif // FRMCONFIG_H
