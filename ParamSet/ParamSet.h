#ifndef PARAMSET_H
#define PARAMSET_H

#include <QObject>
#include <QSettings>
#include <QVariant>

class ParamSet : public QObject
{
    Q_OBJECT
public:
    explicit ParamSet(QObject *parent = 0);
    void loadSetting();
    void setFileName(QString strFile);
    void initDefaultSetting();

signals:

public slots:

private:
   QString m_strFileName;

};

#endif // PARAMSET_H
