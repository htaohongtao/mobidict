#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

#include "ui_settings.h"

class QSettings;

class Settings : public QDialog {
  Q_OBJECT

 public:
  Settings(QWidget* parent, QSettings*);
  ~Settings();

 public slots:
  void saveSettings();

 private:
  Ui::Settings* m_ui;
  QSettings* m_settings;
};

#endif
