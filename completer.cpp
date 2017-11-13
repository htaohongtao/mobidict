#include "completer.h"

Completer::Completer()
{
  m_matches = QList<QString>();
}

Completer::Completer(const QList<QString>& words) : Completer()
{
  m_wordList = words;
}

Completer::~Completer()
{
}

void Completer::setWordList(const QList<QString>& wordList)
{
  m_wordList.clear();
  m_wordList = wordList;
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
