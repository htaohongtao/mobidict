#include <QDebug>
#include <QtGlobal>

#include "mobidict.h"

#define DIFF_THRESHOLD 2000

MobiDict::MobiDict(const QString &path) : QObject()
{
  m_mobiData  = nullptr;
  m_rawMarkup = nullptr;
  m_path      = path;
  m_title     = QString::null;
  m_codec     = nullptr;
}

MobiDict::~MobiDict()
{
  mobi_free(m_mobiData);
  mobi_free_rawml(m_rawMarkup);

  qDeleteAll(m_wordMap);
}

QString MobiDict::entryForLink(const QString &link)
{
  uint32_t offset   = link.toUInt();
  uint32_t bestDiff = DIFF_THRESHOLD;
  QString match     = QString::null;

  for (const auto &entry : m_wordMap.keys()) {
    uint32_t diff = qAbs(m_wordMap[entry]->startPos - offset);
    if (diff < DIFF_THRESHOLD && diff < bestDiff) {
      match    = entry;
      bestDiff = diff;
      qWarning() << entry << "is a match?" << diff;
    }
  }

  return MobiDict::entryForWord(match);
}

QString MobiDict::entryForWord(const QString &word)
{
  if (m_wordMap.find(word) == m_wordMap.constEnd())
    return QString("<h1>Word not found.</h1>");

  QString result;
  MobiEntry *mobiEntry    = m_wordMap[word];
  uint32_t entry_startpos = mobiEntry->startPos;
  uint32_t entry_textlen  = mobiEntry->textLength;

  char *entry = new char[entry_textlen + 1];
  memcpy(entry, m_rawMarkup->flow->data + entry_startpos, entry_textlen);
  entry[entry_textlen] = '\0';

  if (m_isCP1252)
    result = m_codec->toUnicode(entry);
  else
    result = QString::fromUtf8(entry);

  delete[] entry;
  entry = nullptr;

  // Change filepos -> href, hirecindex -> src
  // so that Qt can give us a url in QTextBrowser::loadResource()
  result = result.replace("filepos=", "href=");
  result = result.replace("hirecindex=", "src=");

  // qDebug() << "HTML entry:";
  // qDebug() << result;

  return result;
}

MOBI_RET MobiDict::open()
{
  m_mobiData = mobi_init();
  if (m_mobiData == nullptr)
    return MOBI_MALLOC_FAILED;

#ifdef Q_OS_WIN
  wchar_t *w_path = new wchar_t[m_path.length() + 1];
  int len         = m_path.toWCharArray(w_path);
  w_path[len]     = NULL;

  FILE *file;
  _wfopen_s(&file, w_path, L"rb");

  delete[] w_path;
  w_path = nullptr;
#else
  FILE *file = fopen(m_path.toLocal8Bit(), "rb");
#endif

  if (file == nullptr)
    return MOBI_ERROR;

  MOBI_RET mobi_ret = mobi_load_file(m_mobiData, file);
  fclose(file);

  if (mobi_ret != MOBI_SUCCESS)
    return mobi_ret;

  m_isCP1252 = mobi_is_cp1252(m_mobiData);
  if (m_isCP1252)
    m_codec = QTextCodec::codecForName("cp1252");

  char *title = mobi_meta_get_title(m_mobiData);
  m_title     = QString::fromUtf8(title);
  free(title);

  m_rawMarkup = mobi_init_rawml(m_mobiData);
  if (m_rawMarkup == nullptr)
    return MOBI_MALLOC_FAILED;

  mobi_ret =
      mobi_parse_rawml_opt(m_rawMarkup, m_mobiData, false, /* parse toc */
                           true,                           /* parse dic */
                           false /* reconstruct */);

  if (mobi_ret != MOBI_SUCCESS)
    return mobi_ret;

  uint32_t entry_startpos;
  uint32_t entry_textlen = 0;

  size_t count = m_rawMarkup->orth->total_entries_count;

  for (size_t i = 0; i < count; ++i) {
    const MOBIIndexEntry *orth_entry = &m_rawMarkup->orth->entries[i];
    entry_startpos = mobi_get_orth_entry_start_offset(orth_entry);
    entry_textlen  = mobi_get_orth_entry_text_length(orth_entry);

    if (entry_startpos == 0 || entry_textlen == 0) {
      ++i;
      continue;
    }

    // Even though entry_textlen is a uin32_t, we use memcpy hence limited by
    // size_t which cannot be larger than 0xFFFF per ISO/IEC 9899-2011 7.20.3
    if (entry_textlen > 0xFFFF)
      return MOBI_DATA_CORRUPT;

    MobiEntry *mobiEntry  = new MobiEntry;
    mobiEntry->startPos   = entry_startpos;
    mobiEntry->textLength = entry_textlen;

    QString label;

    if (m_isCP1252)
      label = m_codec->toUnicode(orth_entry->label);
    else
      label = QString::fromUtf8(orth_entry->label);

    if (m_wordMap.constFind(label) != m_wordMap.constEnd())
      continue;

    m_wordMap[label] = mobiEntry;

    // qDebug("Adding %s", orth_entry->label);
  }

  if (m_wordMap.isEmpty())
    return MOBI_DATA_CORRUPT;

  qDebug("Dictionary loaded.");

  return MOBI_SUCCESS;
}

const QString &MobiDict::title()
{
  return m_title;
}

const QList<QString> MobiDict::words()
{
  return m_wordMap.keys();
}

MOBIPart *MobiDict::getResourceByUid(const size_t &uid)
{
  return mobi_get_resource_by_uid(m_rawMarkup, uid);
}
