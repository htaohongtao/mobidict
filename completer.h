#ifndef COMPLETER_H
#define COMPLETER_H

#include <QObject>

class Completer : public QObject {
 public:
  Completer(const QList<QString>&);
  ~Completer();

  void setCompletionPrefix(const QString&);
  const QList<QString>& matches();

 private:
  QList<QString> m_wordList;
  QList<QString> m_matches;
};

#endif
