#include "completer.h"

Completer::Completer(const QList<QString>& words)
{
  m_wordList = words;
  m_matches  = QList<QString>();
}

Completer::~Completer()
{
}

void Completer::setCompletionPrefix(const QString& prefix)
{
  if (prefix.isEmpty()) {
    m_matches = m_wordList;
    return;
  }

  m_matches.clear();
  QString lower = prefix.toLower();

  for (auto word : m_wordList)
    if (word.toLower().startsWith(lower))
      m_matches.append(word);
}

const QList<QString>& Completer::matches()
{
  return m_matches;
}
