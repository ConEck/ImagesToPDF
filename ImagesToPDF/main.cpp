#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <iostream>

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);

  QTranslator translator;
  const QStringList ui_languages = QLocale::system().uiLanguages();
  for (const QString& locale : ui_languages)
  {
    const QString base_name = QLocale(locale).name();
    if (translator.load(":/i18n/" + base_name))
    {
      a.installTranslator(&translator);
      break;
    }
  }
  MainWindow w;
  w.show();
  return a.exec();
}
