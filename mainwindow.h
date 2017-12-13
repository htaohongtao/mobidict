#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFutureWatcher>
#include <QSettings>
#include <QWidget>

#include "mobidict.h"
#include "ui_mainwindow.h"

class Completer;
class Settings;

class MainWindow : public QWidget {
  Q_OBJECT

 public:
  MainWindow();
  ~MainWindow();

  bool discoverDictionaries();
  void loadMatches(const QString&);
  void searchItem(QListWidgetItem*);
  void searchWord();

 public slots:
  void dictionaryLoaded();
  void loadDictionary(const QString&);
  void openLink(const QUrl& link);
  void showSettingsDialog();
  void copyWordToClipboard(QListWidgetItem*);
  void clearAndFocus();

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;
  void closeEvent(QCloseEvent*) override;
  void showEvent(QShowEvent*) override;

 private:
  Ui::MainWindow* m_ui;

  MobiDict* m_currentDict;
  QString m_currentDictName;
  Completer* m_completer;
  QString m_deviceSerial;
  QString m_emojiFont;
  QString m_html;
  QString m_fontName;
  int m_fontSize;

  QFutureWatcher<MOBI_RET> m_watcher;
  QFuture<MOBI_RET> m_future;

  Settings* m_settingsDialog;
  QSettings* m_settings;

  void createResources(const QString&);

#ifdef AUTOTEST
  void selfTest();
  bool m_stopTesting;
#endif
};

#endif
