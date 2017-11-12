#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFutureWatcher>
#include <QSettings>

#include "mobidict.h"
#include "ui_mainwindow.h"

static const char* error_messages[] = {
    "Success",
    "Generic error",
    "Wrong function parameter",
    "Corrupted data",
    "File not found",
    "Document is encrypted",
    "Unsupported document format",
    "Memory allocation error",
    "Initialization error",
    "Buffer error",
    "XML error",
    "Invalid DRM pid",
    "DRM key not found",
    "DRM support not included",
};

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
  QCompleter* m_completer;

  QFutureWatcher<MOBI_RET> m_watcher;
  QFuture<MOBI_RET> m_future;

  QSettings m_settings;

  void createResources(const QString&);
  void selfTest();
};

#endif
