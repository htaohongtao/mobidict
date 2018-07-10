#include "completer.h"
#include <QRegularExpression>

void Completer::setWordList(const QList<QString>& wordList)
{
  m_wordList = wordList;
}

QList<QString> Completer::matches(const QString& prefix)
{
  QRegularExpression regex(prefix, QRegularExpression::CaseInsensitiveOption);
  if (!regex.isValid())
    return QList<QString>();

  return m_wordList.filter(regex);
}
