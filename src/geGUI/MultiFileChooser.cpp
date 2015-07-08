#include "MultiFileChooser.h"
#include "Settings.h"
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMenu>
#include <QDesktopServices>
#include <QLineEdit>

MultiFileChooser::MultiFileChooser(QWidget *parent)
  : QWidget(parent)
{
  ui.setupUi(this);
  connect(ui.button, SIGNAL(clicked()), this, SLOT(browseFiles()));
  addFile("");
}

QStringList MultiFileChooser::files()
{
  QList<QLineEdit*> edits = findChildren<QLineEdit*>();

  QStringList output;
  foreach(QLineEdit* edit, edits)
  {
    QString file = edit->text();
    if (file!="")
    {
      output.append(file);
    }
  }

  return output;
}

void MultiFileChooser::setFiles(QStringList files)
{
    foreach(QString file, files)
    {
        addFile(file);
    }
}

void MultiFileChooser::browseFiles()
{
  QStringList open_files = QFileDialog::getOpenFileNames(this, "Add input file(s).", Settings::path("open_data_folder"), "*.*");
  if (open_files.count()!=0)
  {
    Settings::setPath("open_data_folder", open_files.at(0));
  }

  //add files (if not already contained)
  foreach(QString file, open_files)
  {
    if (!files().contains(file))
    {
      addFile(file);
    }
  }
}

void MultiFileChooser::addFile(QString file)
{
  //remove empty dummy edit if present
  if (files().count()==0)
  {
    delete(ui.files_layout->takeAt(0));
  }

  //add edit
  QLineEdit* edit = new QLineEdit();
  edit->setText(file);
  edit->setToolTip(file);
  edit->setReadOnly(true);
  edit->setContextMenuPolicy(Qt::NoContextMenu);
  ui.files_layout->addWidget(edit);
  edit->show();
}

void MultiFileChooser::contextMenuEvent(QContextMenuEvent* e)
{
  //make sure a (valid) file is clicked
  QWidget* clicked_widget = childAt(e->pos());
  QLineEdit* clicked_file = qobject_cast<QLineEdit*>(clicked_widget);
  if (clicked_file==0 || clicked_file->text()=="") return;

  //create menu
  QMenu* menu = new QMenu();
  menu->addAction("Remove");
  menu->addAction("Open");

  //execute
  QAction* selected = menu->exec(e->globalPos());

  //evaluate
  if (selected!=0)
  {
    if(selected->text()=="Remove")
    {
        int index = ui.files_layout->indexOf(clicked_widget);
        QLayoutItem* item = ui.files_layout->takeAt(index);
        delete item->widget();
        delete item;

        if (ui.files_layout->count()==0)
        {
          addFile("");
        }
    }
    if(selected->text()=="Open")
    {
      QDesktopServices::openUrl("file:///" + clicked_file->text());
    }
  }
}
