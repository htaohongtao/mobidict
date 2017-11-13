#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFutureWatcher>
#include <QSettings>

#include "completer.h"
#include "mobidict.h"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow {
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

 protected:
  bool eventFilter(QObject* obj, QEvent* ev) override;
  void closeEvent(QCloseEvent*) override;
  void showEvent(QShowEvent*) override;

 private:
  Ui::MainWindow* m_ui;

  MobiDict* m_currentDict;
  Completer* m_completer;
  QString m_deviceSerial;

  QFutureWatcher<MOBI_RET> m_watcher;
  QFuture<MOBI_RET> m_future;

  QSettings m_settings;

  void createResources(const QString&);
  void selfTest();
};

#endif
