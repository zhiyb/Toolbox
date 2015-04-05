#ifndef DEBUG_H
#define DEBUG_H

#include <QDebug>
#include <QString>

#define DEBUG_LEVEL	LV_PKG

#define pr_debug(str, level)	if (level <= DEBUG_LEVEL) qDebug() << QString("[DEBUG/%1] (%2:%3, %4) %5").arg(level).arg(__FILE__).arg(__LINE__).arg(__func__).arg(str).toLocal8Bit().constData()
#define pr_info(str)		qDebug() << QString("[INFO] %1").arg(str).toLocal8Bit().constData()
#define pr_warning(str)		qWarning(QString("[WARNING] %1").arg(str).toLocal8Bit())
#define pr_error(str)		qFatal(QString("[ERROR] %1").arg(str).toLocal8Bit())

#define LV_BYTE	5
#define LV_PKG	4
#define LV_INFO	3
#define LV_TMP	2
#define LV_MSG	1
#define LV_AWS	0

#endif // DEBUG_H
