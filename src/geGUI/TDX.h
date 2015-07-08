#ifndef TDX_H
#define TDX_H

#include <QString>
#include <QGroupBox>

class TDX
{
  public:
    static QString extractName(QString filename);
    static void clearForm(QGroupBox* box);
    static void createForm(QGroupBox* box, QString filename);
    static QString evaluateForm(QGroupBox* box, QString filename);
    static void populateForm(QGroupBox* box, QMap<QString,QStringList> parameters);

    //Returns an empty string if the tool is valid, an error message otherwise.
    static QString validate(QString filename, bool show_message=true);

  private:
    TDX();
    static QString escape(QString text);
};

#endif // TDX_H
