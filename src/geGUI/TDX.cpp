#include "TDX.h"
#include <QDomDocument>
#include <QFile>
#include <QLabel>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include "Exceptions.h"
#include "FileChooser.h"
#include "MultiFileChooser.h"
#include "GUIHelper.h"
#include "XmlHelper.h"
#include <limits>
#include <QDebug>
#include <QTemporaryFile>
#include <QDir>
#include <QStandardPaths>

QString TDX::extractName(QString filename)
{
  QDomDocument doc;
  QFile file(filename);
  doc.setContent(&file);

  QDomElement tool = doc.elementsByTagName("Tool").at(0).toElement();

  return tool.attribute("name") + " (" + tool.attribute("version") + ")";
}

void TDX::clearForm(QGroupBox* box)
{
    QGridLayout* layout = qobject_cast<QGridLayout*>(box->layout());
    while (layout->count())
    {
        QLayoutItem* child = layout->takeAt(0);
        layout->removeItem(child);
        delete child->widget();
        delete child;
    }
}

void TDX::createForm(QGroupBox* box, QString filename)
{    
  QGridLayout* layout = qobject_cast<QGridLayout*>(box->layout());

  //load document
  QDomDocument doc;
  QFile file(filename);
  doc.setContent(&file);

  //process child elements
  QDomElement tool = doc.elementsByTagName("Tool").at(0).toElement();
  QDomNodeList list = tool.childNodes();
  for (int i=0; i<list.count(); ++i)
  {
    QDomElement element = list.at(i).toElement();

    //special handling: tool description tag
    if (element.tagName()=="Description")
    {
      QString description = element.text();
      description = description.replace("\n", " ");
      description = description.replace("\t", " ");
      description = description.replace("  ", " ");
      QLabel* label_d = new QLabel(description);
      label_d->setObjectName("desc");
      label_d->setWordWrap(true);
      layout->addWidget(label_d, 0, 0, 1, 3);
      continue;
    }

    //add optional checkbox to layout
    QString param_name = element.attribute("name");
    QCheckBox* optional = new QCheckBox();
    QDomNodeList list = element.elementsByTagName("Optional");
    if (list.count()!=1)
    {
        optional->setChecked(true);
        optional->setEnabled(false);
        optional->setToolTip("Mandatory parameter, that cannot be disabled.");
    }
    else
    {
        optional->setToolTip("Optional parameter.");
    }
    optional->setObjectName("check_" + param_name);
    layout->addWidget(optional, 2*i, 0, Qt::AlignVCenter | Qt::AlignLeft);

    //extract default value
    QString default_value;
    if (list.count()==1)
    {
        default_value = list.at(0).toElement().attribute("defaultValue");
    }

    //add label to layout
    QString description = element.elementsByTagName("Description").at(0).toElement().text();
    if (!default_value.isNull())
    {
        description +="<br>Default value: '" + default_value + "'";
    }
    QLabel* label = new QLabel(param_name + ":");
    label->setToolTip(description);
    label->setObjectName("label_" + param_name);
    layout->addWidget(label, 2*i, 1, Qt::AlignVCenter | Qt::AlignLeft);

    //add field to layout
    QWidget* field = 0;
    if (element.tagName()=="String")
    {
      QLineEdit* edit = new QLineEdit();
      field = edit;

      //default
      if (!default_value.isNull())
      {
        edit->setText(default_value);
      }
    }
    else if (element.tagName()=="Flag")
    {
      field = new QCheckBox();
      field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    else if (element.tagName()=="Int")
    {
      QSpinBox* box = new QSpinBox();
      field = box;
      field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

      //default
      if (!default_value.isNull())
      {
        box->setValue(default_value.toInt());
      }

      //min/max
      int min = -std::numeric_limits<int>::max();
      int max = std::numeric_limits<int>::max();
      QDomNodeList list = element.elementsByTagName("Min");
      if (list.count()==1) min = list.at(0).toElement().text().toInt();
      list = element.elementsByTagName("Max");
      if (list.count()==1) max = list.at(0).toElement().text().toInt();
      box->setRange(min, max);
    }
    else if (element.tagName()=="Float")
    {
      QDoubleSpinBox* box = new QDoubleSpinBox();
      field = box;
      field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

      //default
      if (!default_value.isNull())
      {
        box->setValue(default_value.toDouble());
      }

      //min/max
      float min = -std::numeric_limits<float>::max();
      float max = std::numeric_limits<float>::max();
      QDomNodeList list = element.elementsByTagName("Min");
      if (list.count()==1) min = list.at(0).toElement().text().toFloat();
      list = element.elementsByTagName("Max");
      if (list.count()==1) max = list.at(0).toElement().text().toFloat();
      box->setRange(min, max);
    }
    else if (element.tagName()=="Enum")
    {
      QComboBox* box = new QComboBox();
      field = box;
      field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

      //values
      QDomNodeList values = element.elementsByTagName("Value");
      for (int i=0; i<values.count(); ++i)
      {
        box->addItem(values.at(i).toElement().text());
      }

      //default
      if (!default_value.isNull())
      {
          box->setCurrentIndex(box->findText(default_value));
      }
    }
    else if (element.tagName()=="Infile")
    {
      field = new FileChooser(FileChooser::LOAD);
    }
    else if (element.tagName()=="InfileList")
    {
      field = new MultiFileChooser();
    }
    else if (element.tagName()=="Outfile")
    {
      field = new FileChooser(FileChooser::STORE);
    }
    else
    {
		THROW(Exception, "Settings: Unknown type for parameter '" + element.tagName() + "'.");
    }
    field->setObjectName("param_" + param_name);
    layout->addWidget(field, 2*i, 2);

    //connect checkbox
    if (optional->isEnabled())
    {
        field->setEnabled(false);
        optional->connect(optional, SIGNAL(toggled(bool)), field, SLOT(setEnabled(bool)));
    }

    //add description row to layout
    QLabel* label_d = new QLabel(description);
    label_d->setObjectName("desc_" + param_name);
    label_d->setWordWrap(true);
    layout->addWidget(label_d, 2*i+1, 1, 1, 2);
  }

  //add spacer
  QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
  layout->addItem(spacer, layout->rowCount(), 0, 1, 3);

  //show
  box->hide();
  box->show();
}

QString TDX::validate(QString filename, bool show_message)
{
    //(1) check TDX exists
    if (!QFile::exists(filename))
    {
      if (show_message)
      {
		QMap<QString, QString> add;
        add.insert("file name", filename);
		GUIHelper::showMessage("Error", "TDX file not present!", add);
      }
      return "TDX file not present!";
    }

    //(2) check executable exists
    QString executable = filename.mid(0, filename.length()-4);
    if (!QFile::exists(executable))
    {
      if (show_message)
      {
		QMap<QString, QString> add;
        add.insert("file name", executable);
		GUIHelper::showMessage("Error", "Executable not present!", add);
      }
      return "Executable not present!";
    }

    //(3) validate TDX file against schema
	QString messages = XmlHelper::isValidXml(filename, ":/Resources/Resources/TDX_v1.xsd");
	if (messages!="")
    {
      if (show_message)
      {
		QMap<QString, QString> add;
        add.insert("file name", filename);
        add.insert("details", messages);
		GUIHelper::showMessage("Error", "Invalid TDX file!", add);
      }
      return "Invalid TDX file!";
    }

    //(4) check interpreter exists (if applicable)
    QDomDocument doc;
    QFile file(filename);
    doc.setContent(&file);
    QDomElement tool = doc.elementsByTagName("Tool").at(0).toElement();
    QString inter = tool.attribute("interpreter");
    if (inter!="")
    {
        if (QDir::isAbsolutePath(inter))
        {
            if (!QFile::exists(inter))
            {
              if (show_message)
              {
				QMap<QString, QString> add;
                add.insert("interpreter", inter);
				GUIHelper::showMessage("Error", "Interpreter not found!", add);
              }
              return "Interpreter not found!";
            }
        }
        else
        {
            QString inter_loc = QStandardPaths::findExecutable(inter);
            if (!QFile::exists(inter_loc))
            {
              if (show_message)
              {
				QMap<QString, QString> add;
                add.insert("interpreter", inter);
                add.insert("path", inter_loc);
				GUIHelper::showMessage("Error", "Interpreter not found!", add);
              }
              return "Interpreter not found!";
            }
        }
    }

    return "";
}

QString TDX::evaluateForm(QGroupBox* box, QString filename)
{
  //determine interpreter when set
  QDomDocument doc;
  QFile file(filename);
  doc.setContent(&file);
  QDomElement tool = doc.elementsByTagName("Tool").at(0).toElement();

  QString output = escape(tool.attribute("interpreter")) + " ";
  output += escape(filename.mid(0, filename.length()-4));

  //process tool nodes
  QDomNodeList list = tool.childNodes();
  for (int i=0; i<list.count(); ++i)
  {
    QDomElement element = list.at(i).toElement();

    //skip description
    if (element.tagName()=="Description") continue;

    //skip optional elements that are not active
    QString param_name = element.attribute("name");
    QCheckBox* checkbox = box->findChild<QCheckBox*>("check_" + param_name);
    if (!checkbox->isChecked()) continue;

    //process parameters
    if (element.tagName()=="String")
    {
      QLineEdit* field = box->findChild<QLineEdit*>("param_" + param_name);
      output += " -" + param_name + " " + escape(field->text());
    }
    else if (element.tagName()=="Flag")
    {
        QCheckBox* field = box->findChild<QCheckBox*>("param_" + param_name);
        if (field->isChecked())
        {
            output += " -" + param_name;
        }
    }
    else if (element.tagName()=="Int")
    {
        QSpinBox* field = box->findChild<QSpinBox*>("param_" + param_name);
        output += " -" + param_name + " " + QString::number(field->value());
    }
    else if (element.tagName()=="Float")
    {
        QDoubleSpinBox* field = box->findChild<QDoubleSpinBox*>("param_" + param_name);
        output += " -" + param_name + " " + QString::number(field->value());
    }
    else if (element.tagName()=="Enum")
    {
        QComboBox* field = box->findChild<QComboBox*>("param_" + param_name);
        output += " -" + param_name + " " + field->currentText();
    }
    else if (element.tagName()=="Infile" || element.tagName()=="Outfile")
    {
        FileChooser* field = box->findChild<FileChooser*>("param_" + param_name);
        output += " -" + param_name + " " + escape(field->file());
    }
    else if (element.tagName()=="InfileList")
    {
      MultiFileChooser* field = box->findChild<MultiFileChooser*>("param_" + param_name);
	  output += " -" + param_name + " ";
      foreach(QString file, field->files())
      {
        output += escape(file) + " ";
      }
    }
    else
    {
		THROW(Exception, "Settings: Unknown type for parameter '" + element.tagName() + "'.");
    }
  }

  return output.trimmed();
}

void TDX::populateForm(QGroupBox* box, QMap<QString, QStringList> parameters)
{
    QMap<QString, QStringList>::Iterator i = parameters.begin();
    while (i!= parameters.end())
    {
        QString name = i.key();
        QStringList values = i.value();

        //find widget
        if (name.startsWith("-")) name = name.mid(1);
		//qDebug() << "name: " << name;
        QWidget* widget = box->findChild<QWidget*>("param_" + name);
        if (widget==0)
        {
			GUIHelper::showMessage("Error", "Could not find widget for parameter name '" + name + "'!");
            return;
        }

        //enable check box
        box->findChild<QCheckBox*>("check_" + name)->setChecked(true);

        //flag
        if (qobject_cast<QCheckBox*>(widget)!=0)
        {
            qobject_cast<QCheckBox*>(widget)->setChecked(true);
        }
        //string
        else if (qobject_cast<QLineEdit*>(widget)!=0)
        {
            qobject_cast<QLineEdit*>(widget)->setText(values.first());
        }
        //int
        else if (qobject_cast<QSpinBox*>(widget)!=0)
        {
            qobject_cast<QSpinBox*>(widget)->setValue(values.first().toInt());
        }
        //double
        else if (qobject_cast<QDoubleSpinBox*>(widget)!=0)
        {
            qobject_cast<QDoubleSpinBox*>(widget)->setValue(values.first().toDouble());
        }
        //enum
        else if (qobject_cast<QComboBox*>(widget)!=0)
        {
            qobject_cast<QComboBox*>(widget)->setCurrentText(values.first());
        }
        //infile / outfile
        else if (qobject_cast<FileChooser*>(widget)!=0)
        {
            qobject_cast<FileChooser*>(widget)->setFile(values.first());
        }
        //infile list
        else if (qobject_cast<MultiFileChooser*>(widget)!=0)
        {
            qobject_cast<MultiFileChooser*>(widget)->setFiles(values);
        }

        ++i;
    }
}

QString TDX::escape(QString text)
{
  if (text.contains(" ") && text.at(0)!='"' && text.at(text.length()-1)!='"')
  {
    return "\"" + text + "\"";
  }

  return text;
}
