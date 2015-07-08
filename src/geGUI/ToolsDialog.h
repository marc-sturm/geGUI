#ifndef TOOLSDIALOG_H
#define TOOLSDIALOG_H

#include <QDialog>
#include "ui_ToolsDialog.h"

class ToolsDialog
  : public QDialog
{
    Q_OBJECT
    
    public:
        explicit ToolsDialog(QWidget *parent = 0);
        bool toolsChanged();

    protected slots:
        void loadTools(bool changed);
        void remove();
        void importTool();
        void importTDX();

        void listContextMenu(const QPoint &p);

    private:
        Ui::ToolsDialog ui;
        bool changed_;
};

#endif // TOOLSDIALOG_H
