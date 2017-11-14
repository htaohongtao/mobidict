#ifndef MOBIDICT_H
#define MOBIDICT_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QTextCodec>

#include <mobi.h>

typedef struct {
  uint32_t startPos;
  uint32_t textLength;
} MobiEntry;

class MobiDict : public QObject {
 public:
  MobiDict(const QString&, const QString&);
  ~MobiDict();

  MOBI_RET open();
  const QString& title();

  MOBIPart* getResourceByUid(const size_t& uid);

  const QList<QString> words();
  QString entryForLink(const QString&);
  QString entryForWord(const QString&);

 private:
  MOBIData* m_mobiData;
  MOBIRawml* m_rawMarkup;

  QString m_deviceSerial;
  QString m_path;
  QString m_title;

  bool m_isCP1252;

  QMap<QString, QList<MobiEntry*>> m_wordMap;
  QTextCodec* m_codec;
};

#endif
