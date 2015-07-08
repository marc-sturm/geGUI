#include "ToolsDialog.h"
#include "GUIHelper.h"
#include "Settings.h"
#include "TDX.h"
#include <QMenu>
#include <QFileDialog>
#include <QInputDialog>
#include <QProcess>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QDebug>

ToolsDialog::ToolsDialog(QWidget *parent)
  : QDialog(parent)
  , changed_(false)
{
    ui.setupUi(this);
    loadTools(false);

    //signals and slots
    connect(ui.remove, &QPushButton::clicked, this, &ToolsDialog::remove);
    connect(ui.importTool, &QPushButton::clicked, this, &ToolsDialog::importTool);
    connect(ui.importTDX, &QPushButton::clicked, this, &ToolsDialog::importTDX);
    connect(ui.tools, &QListWidget::customContextMenuRequested, this, &ToolsDialog::listContextMenu);
}

void ToolsDialog::loadTools(bool changed)
{
    //update tools list
    ui.tools->clear();
    QMap<QString, QVariant> tools = Settings::map("tools");
    foreach(QString key, tools.keys())
    {
        QListWidgetItem* item = new QListWidgetItem(key);
        QString error_message = TDX::validate(tools[key].toString(), false);
        if (error_message!="")
        {
          item->setToolTip(error_message);
          item->setIcon(QIcon(":/Resources/Icons/Attention.png"));
        }
        ui.tools->addItem(item);
    }

    //changed?
    if (changed)
    {
      changed_ = true;
    }
}

bool ToolsDialog::toolsChanged()
{
    return changed_;
}

void ToolsDialog::remove()
{
    //get tools
    QList<QListWidgetItem*> selected = ui.tools->selectedItems();
    if (selected.count()==0) return;

    //update tools list in Settings
    QMap<QString, QVariant> tools = Settings::map("tools");
    foreach(QListWidgetItem* item, selected)
    {
        QString key = item->text();
        tools.remove(key);
    }
    Settings::setMap("tools", tools);

    //update tools in GUI
    loadTools(true);
}

void ToolsDialog::importTool()
{
    //get tool
    QString tool = QFileDialog::getOpenFileName(this, "Select tool to import.", Settings::path("open_folder"), "executables (*.exe);;scripts (*.php *.pl *.py);;All files (*.*)");
    if  (tool=="") return;

    //get parameter
    QString parameter = QInputDialog::getText(this, "Specify parameter for TDX generation.", "parameter:", QLineEdit::Normal, "--tdx");
    if (parameter=="") return;

    //create TDX file
    QString call = tool + " " + parameter;
    int exit_code = QProcess::execute(call);
    if (exit_code!=0)
    {
		GUIHelper::showMessage("Error", "Could not create TDX file! Exit code '" + QString::number(exit_code) + "' returned by system call '" + call + "'!");
        return;
    }

    //validate before import
    QString tdx = tool + ".tdx";
    if (TDX::validate(tdx)!="") return;

    //update tools list in Settings
    QMap<QString, QVariant> tools = Settings::map("tools");
    tools.insert(TDX::extractName(tdx), tdx);
    Settings::setMap("tools", tools);

    //update open_folder location
    Settings::setPath("open_folder", tdx);

    //update tools in GUI
    loadTools(true);
}

void ToolsDialog::importTDX()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Select TDX files to import.", Settings::path("open_folder"), "TDX files (*.tdx);;All files (*.*)");
    if  (files.count()==0) return;

    //update tools list in Settings
    QMap<QString, QVariant> tools = Settings::map("tools");
    foreach(QString file, files)
    {
      //validate before import
      if (TDX::validate(file)!="") continue;

      tools.insert(TDX::extractName(file), file);
    }
    Settings::setMap("tools", tools);

    //update open_folder location
    Settings::setPath("open_folder", files[0]);

    //update tools in GUI
    loadTools(true);
}

void ToolsDialog::listContextMenu(const QPoint& p)
{
  //check that one item is selcted
  QListWidgetItem* item = ui.tools->itemAt(p);
  if (item==0) return;

  //get file info
  QString tool_name = item->text();
  QMap<QString, QVariant> tools = Settings::map("tools");
  QString tool_path = tools[tool_name].toString();
  bool is_absolute = QFileInfo(tool_path).isAbsolute();

  //show menu
  QMenu* menu = new QMenu(this);
  menu->addAction("Open containing folder");
  menu->addAction(is_absolute ? "Use relative path" : "Use absolute path");
  QAction* action = menu->exec(ui.tools->mapToGlobal(p));
  if (action==0) return;

  //perform actions
  if (action->text()=="Open containing folder")
  {
    QFileInfo file(tool_path);
    QDesktopServices::openUrl(file.canonicalPath());
  }
  if (action->text()=="Use relative path")
  {
    QString relative = QDir().relativeFilePath(tool_path);
    tools[tool_name] = relative;
    Settings::setMap("tools", tools);
  }
  if (action->text()=="Use absolute path")
  {
    QString absolute = QDir().absoluteFilePath(tool_path);
    tools[tool_name] = absolute;
    Settings::setMap("tools", tools);
  }
}
