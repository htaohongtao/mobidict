#ifndef COMPLETER_H
#define COMPLETER_H

#include <QObject>

class Completer : public QObject {
 public:
  Completer();
  Completer(const QList<QString>&);
  ~Completer();

  void setWordList(const QList<QString>&);
  void setCompletionPrefix(const QString&);
  const QList<QString>& matches();

 private:
  QList<QString> m_wordList;
  QList<QString> m_matches;
};

#endif
