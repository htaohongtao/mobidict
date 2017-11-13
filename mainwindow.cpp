#include <QtConcurrent/qtconcurrentrun.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QShortcut>
#include <QTest>

#include "mainwindow.h"

//#define SELF_TEST

MainWindow::MainWindow() : QMainWindow(), m_ui(new Ui::MainWindow())
{
  m_ui->setupUi(this);
  m_ui->splitter->setStretchFactor(0, 2);
  m_ui->splitter->setStretchFactor(1, 8);

  m_ui->resultBrowser->document()->setDefaultStyleSheet(
      "* {font-size: 18px; font-family: \"Monospace\" }");

  m_completer   = new Completer;
  m_currentDict = nullptr;

  m_ui->searchLine->installEventFilter(this);

  connect(m_ui->searchLine, &QLineEdit::returnPressed, this,
          &MainWindow::searchWord);
  connect(m_ui->searchLine, &QLineEdit::textChanged, this,
          &MainWindow::loadMatches);
  connect(m_ui->matchesWidget, &QListWidget::itemActivated, this,
          &MainWindow::searchItem);
  connect(m_ui->matchesWidget, &QListWidget::itemClicked, this,
          &MainWindow::searchItem);
  connect(m_ui->matchesWidget, &QListWidget::currentItemChanged, this,
          &MainWindow::searchItem);
  connect(m_ui->resultBrowser, &QTextBrowser::anchorClicked, this,
          &MainWindow::openLink);
  connect(
      m_ui->dictComboBox,
      static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated),
      this, &MainWindow::loadDictionary);
  connect(&m_watcher, &QFutureWatcher<bool>::finished, this,
          &MainWindow::dictionaryLoaded);

  new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), m_ui->searchLine,
                SLOT(setFocus()));

#ifdef Q_OS_OSX
  m_ui->matchesWidget->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif
}

MainWindow::~MainWindow()
{
  delete m_ui;
  m_ui = nullptr;

  delete m_currentDict;
  m_currentDict = nullptr;

  delete m_completer;
  m_completer = nullptr;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
  if ((obj == m_ui->searchLine) && (event->type() == QEvent::KeyPress)) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Down) {
      m_ui->matchesWidget->setFocus();
      m_ui->matchesWidget->setCurrentItem(m_ui->matchesWidget->item(0));
    }
  }

  return false;
}

void MainWindow::closeEvent(QCloseEvent* ev)
{
  m_settings.setValue("mainwindow/geometry", geometry());
  m_settings.setValue("mainwindow/splitterSizes", m_ui->splitter->saveState());
  m_settings.setValue("viewer/lastDictionary",
                      m_ui->dictComboBox->currentText());

  QMainWindow::closeEvent(ev);
}

void MainWindow::showEvent(QShowEvent* ev)
{
  QRect rect = m_settings.value("mainwindow/geometry", QRect()).toRect();
  QByteArray splitterSizes =
      m_settings.value("mainwindow/splitterSizes", QByteArray()).toByteArray();

  QString lastDictionary =
      m_settings.value("viewer/lastDictionary", QString()).toString();

  if (!rect.isEmpty())
    setGeometry(rect);

  if (!splitterSizes.isEmpty())
    m_ui->splitter->restoreState(splitterSizes);

  if (!lastDictionary.isNull())
    loadDictionary(lastDictionary);
  else
    loadDictionary(m_ui->dictComboBox->currentText());

  QMainWindow::showEvent(ev);
}

void MainWindow::dictionaryLoaded()
{
  MOBI_RET result = m_future.result();

  if (result != MOBI_SUCCESS) {
    QMessageBox::critical(
        this, "Error opening dictionary",
        QString("Error code %1: %2").arg(result).arg(error_messages[result]));

    setWindowTitle("Mobidict");
    delete m_currentDict;
    m_currentDict = nullptr;
  }
  else {
    m_completer->setWordList(m_currentDict->words());

    setWindowTitle(m_currentDict->title());
    m_ui->searchLine->setEnabled(true);

    // Populate the list widget
    loadMatches(QString::null);

    // and select the first item
    m_ui->matchesWidget->setCurrentItem(m_ui->matchesWidget->item(0));

#ifdef SELF_TEST
    selfTest();
#endif
  }

  m_ui->dictComboBox->setEnabled(true);
}

bool MainWindow::discoverDictionaries()
{
  QDir dir(QString("%1/Dictionaries").arg(QDir::homePath()));
  QStringList dictionaries = dir.entryList(
      QStringList("*.mobi"), QDir::Files | QDir::Readable, QDir::Name);

  if (dictionaries.isEmpty()) {
    QMessageBox::critical(
        nullptr, "No dictionary found",
        QString("Please put your dictionaries (in mobi format) under "
                "<b>%1/Dictionaries</b> and retry.")
            .arg(QDir::homePath()));
    return false;
  }

  for (auto dict : dictionaries)
    m_ui->dictComboBox->addItem(QFileInfo(dir, dict).baseName());

  return true;
}

void MainWindow::loadDictionary(const QString& text)
{
  if (m_currentDict)
    delete m_currentDict;

  m_currentDict = new MobiDict(
      QString("%1/Dictionaries/%2.mobi").arg(QDir::homePath()).arg(text));
  m_future = QtConcurrent::run(m_currentDict, &MobiDict::open);
  m_watcher.setFuture(m_future);

  m_ui->matchesWidget->clear();
  m_ui->searchLine->setEnabled(false);
  m_ui->dictComboBox->setEnabled(false);
  m_ui->dictComboBox->setCurrentText(text);
  m_ui->resultBrowser->clear();

  setWindowTitle(QString("Loading %1 ...").arg(text));
}

void MainWindow::searchWord()
{
  QString word = m_ui->searchLine->text();
  QString html = m_currentDict->entryForWord(word);

  createResources(html);
  m_ui->resultBrowser->setHtml(html);
}

void MainWindow::searchItem(QListWidgetItem* item)
{
  if (!item)
    return;

  QString html = m_currentDict->entryForWord(item->text());
  createResources(html);

  m_ui->resultBrowser->setHtml(html);
}

void MainWindow::loadMatches(const QString& word)
{
  if (!m_completer)
    return;

  m_completer->setCompletionPrefix(word);
  m_ui->matchesWidget->clear();
  m_ui->matchesWidget->addItems(m_completer->matches());
}

void MainWindow::createResources(const QString& html)
{
  QRegularExpression re("src=\"(\\d+)\"");
  QMap<QString, QImage*> resourceMap;
  size_t uid = 0;

  QRegularExpressionMatchIterator i = re.globalMatch(html);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    QString word                  = match.captured(1);

    if (resourceMap.constFind(word) != resourceMap.constEnd())
      continue;

    uid            = word.toUInt(nullptr, 10);
    MOBIPart* flow = m_currentDict->getResourceByUid(uid);

    if (flow != nullptr) {
      switch (flow->type) {
        case MOBIFiletype::T_SVG:
        case MOBIFiletype::T_JPG:
        case MOBIFiletype::T_GIF:
        case MOBIFiletype::T_PNG:
        case MOBIFiletype::T_BMP: {
          QImage* img  = new QImage;
          bool success = img->loadFromData(
              QByteArray((const char*)flow->data, flow->size));
          if (!success) {
            qWarning() << "Failed to load image for" << match;
            delete img;
            img = nullptr;
          }
          else
            resourceMap[word] = img;
        } break;
        default:
          break;
      }
    }
    else
      qWarning() << "Failed to get a resource for" << word;
  }

  m_ui->resultBrowser->setResourceMap(resourceMap);
}

void MainWindow::openLink(const QUrl& link)
{
  QString result = m_currentDict->entryForLink(link.toString());
  m_ui->resultBrowser->setHtml(result);
}

void MainWindow::selfTest()
{
  QApplication::processEvents();
  for (int i = 0; i < m_ui->matchesWidget->count(); ++i) {
    QListWidgetItem* item = m_ui->matchesWidget->item(i);
    QPoint center         = m_ui->matchesWidget->visualItemRect(item).center();
    QWidget* viewPort     = m_ui->matchesWidget->viewport();
    m_ui->matchesWidget->setCurrentItem(item);

    QTest::mouseClick(viewPort, Qt::LeftButton, Qt::NoModifier, center, 0);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  }
}
