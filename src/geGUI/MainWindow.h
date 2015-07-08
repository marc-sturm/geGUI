#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QProcess>
#include <QMovie>
#include <QTime>
#include <QTimer>
#include "ui_MainWindow.h"

class MainWindow
    : public QMainWindow
{
  Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0);

  public slots:
    void updateToolsList();
    void updateToolForm(QString name);

    void executeTool();
    void executeFinished(int code, QProcess::ExitStatus status);
    void executeError(QProcess::ProcessError);
    void terminate();
    void printOutput();
    void appendOutput(QString text);

    void processCLI();

    //auto-connecting actions
	void on_actionHelp_toggled(bool on);
    void on_actionAbout_triggered();
    void on_actionManage_triggered();
    void on_actionOutput_triggered();
    void on_actionHelp_triggered();
    void on_actionShowCall_triggered();
    void on_actionShowTime_triggered();
    void on_actionCall_triggered();

  private:
    Ui::MainWindow ui;
    QProcess process_;
    QTimer process_timer_;
    QString process_file_;
	qint64 process_file_pos_;
    QTime timer_;
};

#endif // MAINWINDOW_H
