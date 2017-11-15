#include <QSettings>

#include "settings.h"

Settings::Settings(QWidget* parent, QSettings* settings) : QDialog(parent), m_ui(new Ui::Settings)
{
  m_ui->setupUi(this);

  m_settings = settings;

  for (const auto& point : QFontDatabase::standardSizes())
    m_ui->pointComboBox->addItem(QString::number(point));

  QString fontName     = m_settings->value("viewer/fontName", "Consolas").toString();
  int fontSize         = m_settings->value("viewer/fontSize", 18).toInt();
  QString deviceSerial = m_settings->value("viewer/deviceSerial", QString()).toString();

  m_ui->fontComboBox->setCurrentFont(QFont(fontName, fontSize));
  m_ui->serialNumber->setText(deviceSerial);
  m_ui->pointComboBox->setCurrentText(QString::number(fontSize));

  connect(this, &QDialog::accepted, this, &Settings::saveSettings);
}

Settings::~Settings()
{
  delete m_ui;
  m_ui = nullptr;
}

void Settings::saveSettings()
{
  QString fontName     = m_ui->fontComboBox->currentFont().family();
  int pointSize        = m_ui->pointComboBox->currentText().toUInt(nullptr);
  QString deviceSerial = m_ui->serialNumber->text();

  m_settings->setValue("viewer/fontName", fontName);
  m_settings->setValue("viewer/fontSize", pointSize);
  m_settings->setValue("viewer/deviceSerial", deviceSerial);
  m_settings->sync();
}
