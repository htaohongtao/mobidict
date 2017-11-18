#include "completer.h"

Completer::Completer()
{
  m_matches = QList<QString>();
}

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

void Completer::setCompletionPrefix(const QString& prefix)
{
  if (prefix.isEmpty()) {
    m_matches = m_wordList;
    return;
  }

  m_matches.clear();
  QString upper = prefix.toUpper();

  for (auto word : m_wordList)
    if (word.toUpper().indexOf(upper) >= 0)
      m_matches.append(word);
}

const QList<QString>& Completer::matches()
{
  return m_matches;
}
