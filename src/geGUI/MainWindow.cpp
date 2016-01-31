#include "MainWindow.h"
#include "ToolsDialog.h"
#include "Settings.h"
#include "Helper.h"
#include "TDX.h"
#include "GUIHelper.h"
#include <QMessageBox>
#include <QFormLayout>
#include <QFileDialog>
#include <QSpinBox>
#include <QDebug>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , process_(this)
{
  ui.setupUi(this);
  setWindowTitle(QApplication::applicationName());
  ui.output_dock->hide();
  ui.call_dock->hide();

  QGridLayout* layout = new QGridLayout();
  ui.box->setLayout(layout);

  //tools
  connect(ui.tools, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateToolForm(QString)));
  updateToolsList();

  //buttons
  ui.terminate->hide();
  ui.busy->hide();
  connect(ui.execute, SIGNAL(clicked()), this, SLOT(executeTool()));
  connect(ui.terminate, SIGNAL(clicked()), this, SLOT(terminate()));

  //process
  process_file_ = Helper::tempFileName(".txt", 16);
  process_.setProcessChannelMode(QProcess::MergedChannels);
  process_.setStandardOutputFile(process_file_);
  connect(&process_, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(executeFinished(int,QProcess::ExitStatus)));
  connect(&process_, SIGNAL(error(QProcess::ProcessError)), this, SLOT(executeError(QProcess::ProcessError)));

  process_timer_.setInterval(100);
  process_timer_.setSingleShot(false);
  connect(&process_timer_, SIGNAL(timeout()), this, SLOT(printOutput()));

  //settings
  ui.actionShowCall->setChecked(Settings::boolean("show_call_in_output", true));
  ui.actionShowTime->setChecked(Settings::boolean("show_time_in_output", true));
  ui.actionHelp->setChecked(Settings::boolean("show_help", true));

  //process CLI parameters
  processCLI();
}


void MainWindow::on_actionAbout_triggered()
{
  QMessageBox::about(this, "About " + QApplication::applicationName(), QApplication::applicationName() + " " + QApplication::applicationVersion() +"\n\nThis program is free software.\n\nThis program is provided as is with no warranty of any kind, including the warranty of design, merchantability and fitness for a particular purpose.");
}

void MainWindow::on_actionManage_triggered()
{
    ToolsDialog* dialog = new ToolsDialog(this);
    dialog->exec();

    if (dialog->toolsChanged())
    {
        updateToolsList();
    }

    delete dialog;
}

void MainWindow::on_actionOutput_triggered()
{
    ui.output_dock->setVisible(!ui.output_dock->isVisible());
}

void MainWindow::on_actionHelp_triggered()
{
    Settings::setBoolean("show_help", ui.actionHelp->isChecked());
}

void MainWindow::on_actionShowCall_triggered()
{
    Settings::setBoolean("show_call_in_output", ui.actionShowCall->isChecked());
}

void MainWindow::on_actionShowTime_triggered()
{
    Settings::setBoolean("show_time_in_output", ui.actionShowTime->isChecked());
}

void MainWindow::on_actionCall_triggered()
{
    ui.call_dock->setVisible(!ui.call_dock->isVisible());
}

void MainWindow::processCLI()
{
    QStringList args = QApplication::arguments();
    args.removeFirst();
    if (args.count()==0) return;

    //parse tool name
    QString tool = args[0];

    //find matching tools
    QStringList matches;
    for (int i=0; i<ui.tools->count(); ++i)
    {
        QString item_text = ui.tools->itemText(i);
        if (item_text.startsWith(tool))
        {
            matches.append(item_text);
        }
    }
    if (matches.size()==0)
    {
		GUIHelper::showMessage("Error", "Command-line tool name '" + tool + "' not found!");
        return;
    }
    if (matches.size()>=2)
    {
		QMap<QString, QString> info;
        int i=1;
        foreach(QString match, matches)
        {
            info.insert("tool" + i, match);
            ++i;
        }
		GUIHelper::showMessage("Error", "Command-line tool name '" + tool + "' matches several tools!", info);
        return;
    }
    ui.tools->setCurrentText(matches.first());
    args.removeFirst();

    //parse parameters
    QMap<QString, QStringList> parameters;
    QString key = "";
    foreach(QString arg, args)
    {
        if (arg.startsWith("-"))
        {
            key = arg;
            parameters.insert(key, QStringList());
        }
        else
        {
            parameters[key].append(arg);
        }
    }

    //assign parameters
    TDX::populateForm(ui.box, parameters);
}

void MainWindow::updateToolsList()
{
  //store selection
  QString selected = ui.tools->currentText();

  //update list
  ui.tools->clear();
  ui.tools->addItem("<select tool>");
  QMap<QString, QVariant> tools = Settings::map("tools");
  foreach(QString key, tools.keys())
  {
    ui.tools->addItem(key);
  }

  //restore selection
  int index = ui.tools->findText(selected);
  if (index!=-1)
  {
    ui.tools->setCurrentIndex(index);
  }
}

void MainWindow::updateToolForm(QString name)
{
  //clear form
  TDX::clearForm(ui.box);

  //abort if nothing is selected
  if (name=="" || name=="<select tool>") return;

  //validate before use
  QMap<QString, QVariant> tools = Settings::map("tools");
  QString filename = tools[name].toString();
  if (TDX::validate(filename)!="") return;

  //create form
  TDX::createForm(ui.box, filename);
  on_actionHelp_toggled(ui.actionHelp->isChecked());

  //update title
  setWindowTitle(QApplication::applicationName() + " - " + name.left(name.indexOf(' ')));
}

void MainWindow::executeTool()
{
    //get call
    QMap<QString, QVariant> tools = Settings::map("tools");
    QString filename = tools[ui.tools->currentText()].toString();
    QString call = TDX::evaluateForm(ui.box, filename);

    //print formatted call string
    QStringList parts = call.split(" -");
    QString formatted_call = parts[0];
    for(int i=1; i<parts.count(); ++i)
    {
        formatted_call += "\n-" + parts[i];
    }
    ui.call->clear();
    ui.call->setText(formatted_call);

    //update GUI
    ui.terminate->show();
	ui.busy->show();
    ui.execute->setEnabled(false);
    ui.output->clear();
    if  (ui.actionShowCall->isChecked())
    {
        appendOutput(">" + call + "\n");
    }

    //start execute
    QFile::remove(process_file_);
    process_file_pos_ = 0;
    timer_.start();
    process_.start(call);
    process_timer_.start();
}

void MainWindow::executeFinished(int /*code*/, QProcess::ExitStatus /*status*/)
{
    //update GUI
    ui.execute->setEnabled(true);
    ui.terminate->hide();
	ui.busy->hide();

    //internal
    process_timer_.stop();
    printOutput();
    appendOutput("\n>Exit code: " + QString::number(process_.exitCode()));
    if  (ui.actionShowTime->isChecked())
    {
		appendOutput("\n>Execution time: " + Helper::elapsedTime(timer_));
    }
}

void MainWindow::executeError(QProcess::ProcessError)
{
    //update GUI
    ui.execute->setEnabled(true);
    ui.terminate->hide();
	ui.busy->hide();

    //internal
    process_timer_.stop();
    printOutput();
    appendOutput("\n>Process error!");
    appendOutput("\n>Exit code: " + QString::number(process_.exitCode()));
    if  (ui.actionShowTime->isChecked())
    {
		appendOutput("\n>Execution time: " + Helper::elapsedTime(timer_));
    }
}

void MainWindow::terminate()
{
    process_timer_.stop();

    process_.blockSignals(true);
    process_.kill();
    process_.waitForFinished();
    process_.blockSignals(false);

    //update GUI
    ui.execute->setEnabled(true);
    ui.terminate->hide();
	ui.busy->hide();
    printOutput();
    appendOutput("\n>Terminated by user!");
}

void MainWindow::printOutput()
{
  QFile file(process_file_);
  bool opened = file.open(QIODevice::ReadOnly);
  if (!opened) return;

  if (file.size()>process_file_pos_)
  {
    file.seek(process_file_pos_);
    appendOutput(file.readAll());
    process_file_pos_ = file.pos();
  }

  file.close();
}

void MainWindow::appendOutput(QString text)
{
  if (text=="") return;

  ui.output->moveCursor(QTextCursor::End);
  ui.output->insertPlainText(text.replace("\r", ""));

  ui.output_dock->show();
}

void MainWindow::on_actionHelp_toggled(bool on)
{
  for (int i=0; i<ui.box->layout()->count(); ++i)
  {
    QWidget* widget = ui.box->layout()->itemAt(i)->widget();
    if (widget!=0 && widget->objectName().startsWith("desc"))
    {
      widget->setVisible(on);
    }
  }
}
