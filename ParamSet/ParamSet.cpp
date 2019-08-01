#include <stdio.h>
#include <string>
#include <QFile>
#include <QDebug>

#include "ParamSet.h"
#include "../nox/noxClient.h"

ParamSet::ParamSet(QObject *parent):
    QObject(parent)
{
    ;
}

void ParamSet::loadSetting()
{
    QSettings *settings = new QSettings(m_strFileName, QSettings::IniFormat);

    settings->beginGroup("FACTORY_CFG");
    g_is_exit_factory_wifi_pass  = settings->value("exit_factroy", 0).toInt();

    settings->endGroup();

    delete settings;
}

void ParamSet::setFileName(QString strFile)
{
    m_strFileName = strFile;
}

void ParamSet::initDefaultSetting()
{
    QSettings *settings =  new QSettings(m_strFileName, QSettings::IniFormat);

    settings->beginGroup("FACTORY_CFG");
    settings->setValue("exit_factroy", 0);
    settings->endGroup();

    delete settings;
}
