#include "completer.h"
#include <QRegularExpression>

Completer::Completer() {}

Completer::Completer(const QList<QString>& words) : Completer()
{
  m_wordList = words;
}

Completer::~Completer() {}

void Completer::setWordList(const QList<QString>& wordList)
{
  m_wordList.clear();
  m_wordList = wordList;
}

QList<QString> Completer::matches(const QString& prefix)
{
  return m_wordList.filter(
      QRegularExpression(prefix, QRegularExpression::CaseInsensitiveOption));
}
