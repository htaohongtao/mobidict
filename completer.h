#ifndef COMPLETER_H
#define COMPLETER_H

#include <QObject>

class Completer : public QObject {
 public:
  void setWordList(const QList<QString>&);
  QList<QString> matches(const QString&);

 private:
  QList<QString> m_wordList;
};

#endif
