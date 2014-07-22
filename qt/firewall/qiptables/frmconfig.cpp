#include "frmconfig.h"
#include "ui_frmconfig.h"

#include "mainwindow.h"

#include <QApplication>

frmConfig::frmConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmConfig)
{
	ui->setupUi(this);
	readSettings();

	mainForm = (MainWindow *) parent;
}

frmConfig::~frmConfig()
{
	writeSettings();
	delete ui;
}

void frmConfig::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void frmConfig::openFileDialog()
{
	QString caller = QApplication::focusWidget()->objectName();
	QFileDialog fd(this, tr("cdn caption"),
								"/etc/iptables");

	if (caller == "btnShellHome" || caller == "btnShellPublic")
	{
		fd.setNameFilter("Shell Files to build Ip Tables Configuration Files (*.sh)");
	} else 	if (caller == "btnIpTablesHome" ||
							caller == "btnIpTablesPublic" ||
							caller == "btnIpTablesDefault")
	{
		fd.setNameFilter("Ip Tables Configuration Files (*.txt)");
	} else 	if (caller == "btnInitIpTables")
	{
		fd.setDirectory ("/etc/init.d");
		fd.setNameFilter("Ip Tables Configuration Files (*.sh)");
	}

	if (fd.exec())
	{
		QStringList sl = fd.selectedFiles();

		if (caller == "btnShellHome")
		{
			ui->edtShellHome->setText(sl.at(0));
		}
		else if (caller == "btnShellPublic")
		{
			ui->edtShellPublic->setText(sl.at(0));
		}
		else if (caller == "btnIpTablesHome")
		{
			ui->edtIpTablesHome->setText(sl.at(0));
		}
		else if (caller == "btnIpTablesPublic")
		{
			ui->edtIpTablesPublic->setText(sl.at(0));
		}
		else if (caller == "btnIpTablesDefault")
		{
			ui->edtIpTablesDefault->setText(sl.at(0));
		}
		else if (caller == "btnInitIpTables")
		{
			ui->edtInitIpTables->setText(sl.at(0));
		}

	}



	/****************
	frmConfig::openFileDialog() Caller [btnShellHome]
	frmConfig::openFileDialog() Caller [btnShellPublic]
	frmConfig::openFileDialog() Caller [btnIpTablesHome]
	frmConfig::openFileDialog() Caller [btnIpTablesPublic]
	frmConfig::openFileDialog() Caller [btnInitIpTables]
	frmConfig::openFileDialog() Caller [btnShellHome]
	******************/

}


void frmConfig::writeSettings()
{
		QSettings settings("chris-newey.homeip.net", "qiptables");

		settings.beginGroup("frmconfig");
		settings.setValue("size", size());
		settings.setValue("pos", pos());

		settings.setValue("shellHome", ui->edtShellHome->text());
		settings.setValue("shellPublic", ui->edtShellPublic->text());
		settings.setValue("ipTablesHome", ui->edtIpTablesHome->text());
		settings.setValue("ipTablesPublic", ui->edtIpTablesPublic->text());
		settings.setValue("ipTablesDefault", ui->edtIpTablesDefault->text());
		settings.setValue("initIpTables", ui->edtInitIpTables->text());

		settings.endGroup();
}

void frmConfig::readSettings()
{
		QSettings settings("chris-newey.homeip.net", "qiptables");

		settings.beginGroup("frmconfig");
		resize(settings.value("size", QSize(400, 400)).toSize());
		move(settings.value("pos", QPoint(200, 200)).toPoint());

		ui->edtShellHome->setText(settings.value("shellHome", "").toString());
		ui->edtShellPublic->setText(settings.value("shellPublic", "").toString());
		ui->edtIpTablesHome->setText(settings.value("ipTablesHome", "").toString());
		ui->edtIpTablesPublic->setText(settings.value("ipTablesPublic", "").toString());
		ui->edtIpTablesDefault->setText(settings.value("ipTablesDefault", "").toString());
		ui->edtInitIpTables->setText(settings.value("initIpTables", "").toString());

		settings.endGroup();
}


void frmConfig::editFile()
{
	QString caller = QApplication::focusWidget()->objectName();

	if (caller == "btnEdtShellHome")
	{
		ui->edtCurrentFile->setText(ui->edtShellHome->text());
		ui->txtFile->setReadOnly(false);
		ui->lblEdit->setText("File being Edited");
	}
	else if (caller == "btnEdtShellPublic")
	{
		ui->edtCurrentFile->setText(ui->edtShellPublic->text());
		ui->txtFile->setReadOnly(false);
		ui->lblEdit->setText("File being Edited");
	}
	else if (caller == "btnEdtIpTablesHome")
	{
		ui->edtCurrentFile->setText(ui->edtIpTablesHome->text());
		ui->txtFile->setReadOnly(true);
		ui->lblEdit->setText("File being Viewed");
	}
	else if (caller == "btnEdtIpTablesPublic")
	{
		ui->edtCurrentFile->setText(ui->edtIpTablesPublic->text());
		ui->txtFile->setReadOnly(true);
		ui->lblEdit->setText("File being Viewed");
	}
	else if (caller == "btnEdtIpTablesDefault")
	{
		ui->edtCurrentFile->setText(ui->edtIpTablesDefault->text());
		ui->txtFile->setReadOnly(true);
		ui->lblEdit->setText("File being Viewed");
	}
	else if (caller == "btnEdtInitIpTables")
	{
		ui->edtCurrentFile->setText(ui->edtInitIpTables->text());
		ui->txtFile->setReadOnly(false);
		ui->lblEdit->setText("File being Edited");
	}
	else
	{
		ui->edtCurrentFile->setText("");
		ui->txtFile->setReadOnly(true);
		ui->lblEdit->setText("File being Viewed");
	}


	openFileForEdit(ui->edtCurrentFile->text());

}


void frmConfig::openFileForEdit(QString filename)
{
	ui->txtFile->clear();

	if (filename.isEmpty() || (! QFile(filename).exists()) )
	{
		QMessageBox msgBox(this);

		msgBox.setWindowTitle("File Open");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setText("Cannot Open file");
		if (filename.isEmpty())
		{
			msgBox.setInformativeText("Empty Filename");
		} else if (! QFile(filename).exists() )
		{
			msgBox.setInformativeText(QString("File [%1] does not exist").arg(filename));
		}

		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);

		msgBox.exec();

		ui->edtCurrentFile->clear();
		ui->btnSave->setEnabled(false);
		return;
	}

	ui->txtFile->setPlainText(readFile(filename));
	ui->btnSave->setEnabled(false);

}

QString frmConfig::readFile(QString filename)
{
	QString fileContents("");

	QFile data(filename);
	if (data.open(QFile::ReadOnly))
	{
		QTextStream in(&data);
		fileContents = in.readAll();
		data.close();
	}
	return fileContents;
}


void frmConfig::btnSaveEnable()
{
	ui->btnSave->setEnabled(true);
}


void frmConfig::saveText()
{
	// make backup
	QString backupFile = ui->edtCurrentFile->text() + "~";

	QFile data(backupFile);
	if (data.exists())
	{
		data.remove();
	}
	data.close();

	data.setFileName(ui->edtCurrentFile->text());
	data.copy(backupFile);

	// write changed content
	if (data.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QTextStream in(&data);
		in << ui->txtFile->toPlainText();
		ui->btnSave->setEnabled(false);
		data.close();

		QMessageBox msgBox(this);

		msgBox.setWindowTitle("File Save");
		msgBox.setIcon(QMessageBox::Information);
		msgBox.setText(QString("File [%1] Saved").arg(ui->edtCurrentFile->text()));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);

		msgBox.exec();

	}

}


QString frmConfig::getCurrentDefaultFirewallName()
{
	return ui->edtIpTablesDefault->text();
}

int frmConfig::getCurrentDefaultFirewallType()
{
	int result = -1;
	QString fileContents = readFile(getCurrentDefaultFirewallName());
	if (fileContents.contains("PUBLIC-NETWORK", Qt::CaseInsensitive))
	{
		result = MainWindow::FIREWALL_PUBLIC;
	} else 	if (fileContents.contains("HOME-NETWORK", Qt::CaseInsensitive))
	{
		result = MainWindow::FIREWALL_HOME;
	}

	return result;
}


void frmConfig::rebuildFirewall()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QString caller = QApplication::focusWidget()->objectName();

	ui->txtFile->clear();

	if (caller == "btnRebuildHome")
	{
		ui->txtFile->append("Running " + ui->edtShellHome->text() + " to rebuild Home Firewall Rules");
		mainForm->executeProgram(ui->edtShellHome->text(),QStringList(),false);
		if (mainForm->getCurrentDefaultFirewall() == MainWindow::FIREWALL_HOME)
		{
			ui->txtFile->append("Home firewall rules are the default - Updating default rules with new Home Rules");
			mainForm->changeDefaultFirewall(MainWindow::FIREWALL_HOME);
		}
	}

	if (caller == "btnRebuildPublic")
	{
		ui->txtFile->append("Running " + ui->edtShellPublic->text() + " to rebuild Public Firewall Rules");
		mainForm->executeProgram(ui->edtShellPublic->text(),QStringList(),false);
		if (mainForm->getCurrentDefaultFirewall() == MainWindow::FIREWALL_PUBLIC)
		{
			ui->txtFile->append("Public firewall rules are the default - Updating default rules with new Public Rules");
			mainForm->changeDefaultFirewall(MainWindow::FIREWALL_PUBLIC);
		}
	}

	ui->txtFile->append("Completed");
	ui->btnSave->setEnabled(false);
	QApplication::restoreOverrideCursor();
}
