#include "completer.h"
#include <QRegularExpression>

void Completer::setWordList(const QList<QString>& wordList)
{
  m_wordList = wordList;
}

QList<QString> Completer::matches(const QString& prefix)
{
  return m_wordList.filter(
      QRegularExpression(prefix, QRegularExpression::CaseInsensitiveOption));
}
