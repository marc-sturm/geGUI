#ifndef MULTIFileChooser_H
#define MULTIFileChooser_H

#include <QWidget>
#include "ui_MultiFileChooser.h"


class MultiFileChooser
    : public QWidget
{
  Q_OBJECT

  public:
    MultiFileChooser(QWidget* parent = 0);
    QStringList files();
    void setFiles(QStringList files);

  protected slots:
    void browseFiles();
    virtual void contextMenuEvent(QContextMenuEvent* e);

  private:
    Ui::MultiFileChooser ui;
    void addFile(QString file);
};

#endif // MULTIMultiFileChooser_H
