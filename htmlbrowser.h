#ifndef HTMLBROWSER_H
#define HTMLBROWSER_H

#include <QTextBrowser>
#include <QUrl>
#include <QVariant>

class HtmlBrowser : public QTextBrowser {
  Q_OBJECT

 public:
  HtmlBrowser(QWidget* parent);
  ~HtmlBrowser();

  void setText(const QString&);
  void setResourceMap(QMap<QString, QImage*>);

 protected:
  QVariant loadResource(int, const QUrl&) override;

 private:
  QMap<QString, QImage*> m_resourceMap;
};

#endif
