#include <QDebug>
#include <QtGlobal>
#include <QSysInfo>

#include "mobidict.h"

#if defined(Q_OS_WIN)
if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS10)
  static const char *emojiFont = "Segoe MDL2 Assets";
else
  static const char *emojiFont = "Segoe UI Symbol";
#elif defined(Q_OS_LINUX)
static const char *emojiFont = "NotoColorEmoji";
#elif defined(Q_OS_MAC)
static const char *emojiFont = "Apple Color Emoji";
#endif

MobiDict::MobiDict(const QString &path, const QString &serial) : QObject()
{
  m_mobiData     = nullptr;
  m_rawMarkup    = nullptr;
  m_path         = path;
  m_title        = QString::null;
  m_codec        = nullptr;
  m_deviceSerial = serial;
}

MobiDict::~MobiDict()
{
  mobi_free(m_mobiData);
  mobi_free_rawml(m_rawMarkup);

  for (const auto &key : m_wordMap.keys())
    qDeleteAll(m_wordMap[key]);
}

QString MobiDict::entryForLink(const QString &link)
{
  uint32_t offset = link.toUInt();
  QString match   = QString::null;

  for (const auto &entry : m_wordMap.keys()) {
    for (const auto &mobiEntry : m_wordMap[entry]) {
      if (mobiEntry->startPos == offset) {
        match = entry;
        break;
      }
    }
  }

  return MobiDict::entryForWord(match);
}

QString MobiDict::entryForWord(const QString &word)
{
  if (m_wordMap.constFind(word) == m_wordMap.constEnd())
    return QString(
               "<br><br><center><font face='%1' size='+6'>🤔</font><br><br></span> The "
               "word <b>%2</b> not found in dictionary.</center>")
        .arg(emojiFont)
        .arg(word);

  QString result;

  for (const auto &mobiEntry : m_wordMap[word]) {
    MobiEntry *m            = mobiEntry;
    uint32_t entry_startpos = m->startPos;
    uint32_t entry_textlen  = m->textLength;

    char *entry = new char[entry_textlen + 1];
    memcpy(entry, m_rawMarkup->flow->data + entry_startpos, entry_textlen);
    entry[entry_textlen] = '\0';

    if (m_isCP1252)
      result.append(m_codec->toUnicode(entry));
    else
      result.append(QString::fromUtf8(entry));

    delete[] entry;
    entry = nullptr;

    // Change filepos -> href, hirecindex -> src
    // so that Qt can give us a url in QTextBrowser::loadResource()
    result = result.replace("filepos=", "href=");
    result = result.replace("hirecindex=", "src=");

    // qDebug() << "HTML entry:";
    // qDebug() << result;
  }

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

  if (mobi_is_encrypted(m_mobiData)) {
    if (!m_deviceSerial.isEmpty()) {
      qWarning() << "Using device serial" << m_deviceSerial;
      mobi_drm_setkey_serial(m_mobiData, m_deviceSerial.toLatin1());
    }
    else {
      return MOBI_FILE_ENCRYPTED;
    }
  }

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

  mobi_ret = mobi_parse_rawml_opt(m_rawMarkup, m_mobiData, false, /* parse toc */
                                  true,                           /* parse dic */
                                  false /* reconstruct */);

  if (mobi_ret != MOBI_SUCCESS)
    return mobi_ret;

  uint32_t entry_startpos;
  uint32_t entry_textlen = 0;

  size_t count = m_rawMarkup->orth->total_entries_count;

  for (size_t i = 0; i < count; ++i) {
    const MOBIIndexEntry *orth_entry = &m_rawMarkup->orth->entries[i];
    entry_startpos                   = mobi_get_orth_entry_start_offset(orth_entry);
    entry_textlen                    = mobi_get_orth_entry_text_length(orth_entry);

    if (entry_startpos == 0 || entry_textlen == 0) {
      ++i;
      continue;
    }

    QString label;

    if (m_isCP1252)
      label = m_codec->toUnicode(orth_entry->label);
    else
      label = QString::fromUtf8(orth_entry->label);

    if (m_wordMap.constFind(label) != m_wordMap.constEnd())
      qDebug() << label << "exists more than once.";

    MobiEntry *mobiEntry  = new MobiEntry;
    mobiEntry->startPos   = entry_startpos;
    mobiEntry->textLength = entry_textlen;

    m_wordMap[label].append(mobiEntry);

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
