#include "mainwindow.h"
#include "ui_mainwindow.h"

 #include <QCoreApplication>



const int MainWindow::FIREWALL_HOME = 0;
const int MainWindow::FIREWALL_PUBLIC = 1;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

		configForm = new frmConfig(this);

		firewalls << "Home" << "Public"; // This stringlist has to be kept in step with FIREWALL_HOME and FIREWALL_PUBLIC

		ui->cbxFirewall->addItems(firewalls);
		ui->cbxDefaultFirewall->addItems(firewalls);

		readSettings();

		ui->cbxDefaultFirewall->setCurrentIndex(currentDefaultFirewall);
		ui->cbxFirewall->setCurrentIndex(currentActiveFirewall);

		displayCurrentFirewall(currentActiveFirewall);

		ui->txtEdit->clear();

		if (isRoot())
		{
			widgetEnable(true);
		}
		else
		{
			widgetEnable(false);
			ui->btnClose->setEnabled(true); // Need the form close button available
			ui->txtEdit->append("<h2>This application must be run as root.</h2>");
			ui->txtEdit->append(QString("<h2>Current User [%1]").arg(getUser()) );
		}

}

MainWindow::~MainWindow()
{
		writeSettings();
		if (configForm)
		{
			delete configForm;
			configForm = 0;
		}

    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void MainWindow::writeSettings()
{
		QSettings settings("chris-newey.homeip.net", "qiptables");

		settings.beginGroup("MainWindow");
		settings.setValue("size", size());
		settings.setValue("pos", pos());

		// CDN 2/3/10  Current default firewall now found by by inspecting the file itself
		//settings.setValue("defaultFirewall", ui->cbxDefaultFirewall->currentIndex() );
		//settings.setValue("activeFirewall", ui->cbxFirewall->currentIndex() );

		settings.endGroup();
}

void MainWindow::readSettings()
{
		QSettings settings("chris-newey.homeip.net", "qiptables");

		settings.beginGroup("MainWindow");
		resize(settings.value("size", QSize(400, 400)).toSize());
		move(settings.value("pos", QPoint(200, 200)).toPoint());

		// CDN 2/3/10 Detect current default firewall by inspecting the file itself
		// currentDefaultFirewall = settings.value("defaultFirewall", MainWindow::FIREWALL_PUBLIC).toInt();
		currentDefaultFirewall = configForm->getCurrentDefaultFirewallType();

		// CDN 2/3/10 Detect current firewall by inspecting the dump from "/sbin/iptables -L"
		//currentActiveFirewall  = settings.value("activeFirewall",  MainWindow::FIREWALL_PUBLIC).toInt();
		currentActiveFirewall = getCurrentActiveFirewall();



		settings.endGroup();


}

int MainWindow::getCurrentActiveFirewall()
{
	int result = -1;
	QString ipTablesDump = executeProgram("/sbin/iptables", QStringList("-L"), true);
	if (ipTablesDump.contains("PUBLIC-NETWORK", Qt::CaseInsensitive))
	{

		result = MainWindow::FIREWALL_PUBLIC;
	} else 	if (ipTablesDump.contains("HOME-NETWORK", Qt::CaseInsensitive))
	{
		result = MainWindow::FIREWALL_HOME;
	}

	return result;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	QMessageBox msgBox;
	msgBox.setText("Close the application?");
	//msgBox.setInformativeText("Close the Application?");
	msgBox.setStandardButtons(QMessageBox::Close | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Close);

	if (QMessageBox::Close == msgBox.exec())
	{
		writeSettings();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}


void MainWindow::changeFirewall()
{
	ui->txtEdit->clear();

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if (ui->cbxFirewall->currentIndex() == MainWindow::FIREWALL_HOME)
	{
		executeProgram("/etc/init.d/iptables.sh", QStringList() << "home");
		currentActiveFirewall = getCurrentActiveFirewall();
		ui->cbxFirewall->setCurrentIndex(currentActiveFirewall);
		displayCurrentFirewall(currentActiveFirewall);
	}
	else if (ui->cbxFirewall->currentIndex() == MainWindow::FIREWALL_PUBLIC)
	{
		executeProgram("/etc/init.d/iptables.sh", QStringList() << "public");
		currentActiveFirewall = getCurrentActiveFirewall();
		ui->cbxFirewall->setCurrentIndex(currentActiveFirewall);
		displayCurrentFirewall(currentActiveFirewall);
	}

	ui->txtEdit->append(
			QString("<h4>Firewall configuration changed to [%1]</h4>").
			arg(ui->cbxFirewall->currentText()));

	QApplication::restoreOverrideCursor();
}

void MainWindow::changeDefaultFirewall(int firewall)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	ui->txtEdit->clear();
	ui->txtEdit->append(
			QString("<h4>Default Firewall changed to [%1]</h4>").
			arg(ui->cbxDefaultFirewall->currentText()));

	if (firewall == MainWindow::FIREWALL_HOME)
	{
		executeProgram("cp",
				QStringList() << "/etc/iptables/firewall-home.txt"
											<< "/etc/iptables/firewall-default.txt");
		currentDefaultFirewall = MainWindow::FIREWALL_HOME;
	}
	else if (firewall == MainWindow::FIREWALL_PUBLIC)
	{
		executeProgram("cp",
				QStringList() << "/etc/iptables/firewall-public.txt"
											<< "/etc/iptables/firewall-default.txt");
		currentDefaultFirewall = MainWindow::FIREWALL_PUBLIC;
	}

	QApplication::restoreOverrideCursor();
}


QString MainWindow::executeProgram(QString prog, QStringList args, bool silent)
{
	QProcess proc;
	QString result("");

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	widgetEnable(false);

	proc.setProcessChannelMode(QProcess::MergedChannels);

	if (args.count() > 0)
	{
		proc.start(prog, args);
	}
	else
	{
		proc.start(prog);
	}
	if (!proc.waitForStarted())
		return "";

	proc.closeWriteChannel();

	if (!proc.waitForFinished())
		return "";

	result = proc.readAll();

	if (! silent)
	{

		ui->txtEdit->append(result);
		ui->txtEdit->append(QString("Exit Code [ %1 ]").arg(proc.exitCode()));
		ui->txtEdit->append("========================================");
	}

	widgetEnable(true);
	QApplication::restoreOverrideCursor();

	return result;
}


QString MainWindow::getUser()
{
	QStringList env = QProcess::systemEnvironment();
	QString user = "User not found";

	for (int i = 0; i <env.count(); i++)
	{
		if (env.at(i).left(5) == "USER=")
		{
			user = env.at(i).mid(5);
		}
	}

	return user;
}

bool MainWindow::isRoot()
{
	bool result = false;
	if (getUser().toUpper() == "ROOT")
	{
		result = true;
	}

	return result;
}


void MainWindow::widgetEnable(bool enable)
{
	if (enable)
	{
		ui->cbxFirewall->setEnabled(true);
		ui->cbxDefaultFirewall->setEnabled(true);
		ui->btnClose->setEnabled(true);
	}
	else
	{
		ui->cbxFirewall->setEnabled(false);
		ui->cbxDefaultFirewall->setEnabled(false);
		ui->btnClose->setEnabled(false);
	}
}


 void MainWindow::openFrmConfig()
 {
	 configForm->exec();
 }


void MainWindow::openFrmAbout()
{
	QMessageBox msgBox(this);

	msgBox.setWindowTitle("About");
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setText("qiptables");
	msgBox.setInformativeText("QT Based GUI frontend to switch firewall configurations.");
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);

	msgBox.exec();
}


void MainWindow::openFrmAboutQt()
{
	QMessageBox::aboutQt(this);
}

void MainWindow::displayCurrentFirewall(int currentFirewall)
{
	if ( (currentFirewall == MainWindow::FIREWALL_PUBLIC) ||
			 (currentFirewall == MainWindow::FIREWALL_HOME) )
	{
		ui->edtCurrentFirewall->setText(firewalls.at(currentFirewall));
	}
	else
	{
		ui->edtCurrentFirewall->setText("");
	}
}

int MainWindow::getCurrentDefaultFirewall()
{
	return currentDefaultFirewall;
}
