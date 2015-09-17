/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmlprofilerclient.h"
#include "qmlprofilerdata.h"

#include <private/qqmlprofilerclient_p_p.h>

#include <QtCore/QStack>
#include <QtCore/QStringList>
#include <QtCore/QDataStream>

#include <limits>

class QmlProfilerClientPrivate : public QQmlProfilerClientPrivate
{
    Q_DECLARE_PUBLIC(QmlProfilerClient)
public:
    QmlProfilerClientPrivate(QQmlDebugConnection *connection, QmlProfilerData *data);

    QmlProfilerData *data;

    qint64 inProgressRanges;
    QStack<qint64> rangeStartTimes[QQmlProfilerDefinitions::MaximumRangeType];
    QStack<QStringList> rangeDatas[QQmlProfilerDefinitions::MaximumRangeType];
    QStack<QQmlEventLocation> rangeLocations[QQmlProfilerDefinitions::MaximumRangeType];
    int rangeCount[QQmlProfilerDefinitions::MaximumRangeType];

    bool enabled;
};

QmlProfilerClientPrivate::QmlProfilerClientPrivate(QQmlDebugConnection *connection,
                                                   QmlProfilerData *data) :
    QQmlProfilerClientPrivate(connection), data(data), inProgressRanges(0), enabled(false)
{
    ::memset(rangeCount, 0, QQmlProfilerDefinitions::MaximumRangeType * sizeof(int));
}

QmlProfilerClient::QmlProfilerClient(QQmlDebugConnection *connection, QmlProfilerData *data) :
    QQmlProfilerClient(*(new QmlProfilerClientPrivate(connection, data)))
{
}

void QmlProfilerClient::clearPendingData()
{
    Q_D(QmlProfilerClient);
    for (int i = 0; i < QQmlProfilerDefinitions::MaximumRangeType; ++i) {
        d->rangeCount[i] = 0;
        d->rangeDatas[i].clear();
        d->rangeLocations[i].clear();
    }
}

void QmlProfilerClient::stateChanged(State state)
{
    Q_D(QmlProfilerClient);
    if ((d->enabled && state != Enabled) || (!d->enabled && state == Enabled)) {
        d->enabled = (state == Enabled);
        emit enabledChanged(d->enabled);
    }
}

void QmlProfilerClient::traceStarted(qint64 time, int engineId)
{
    Q_UNUSED(engineId);
    Q_D(QmlProfilerClient);
    d->data->setTraceStartTime(time);
    emit recordingStarted();
}

void QmlProfilerClient::traceFinished(qint64 time, int engineId)
{
    Q_UNUSED(engineId);
    Q_D(QmlProfilerClient);
    d->data->setTraceEndTime(time);
}

void QmlProfilerClient::rangeStart(QQmlProfilerDefinitions::RangeType type, qint64 startTime)
{
    Q_D(QmlProfilerClient);
    d->rangeStartTimes[type].push(startTime);
    d->inProgressRanges |= (static_cast<qint64>(1) << type);
    ++d->rangeCount[type];
}

void QmlProfilerClient::rangeData(QQmlProfilerDefinitions::RangeType type, qint64 time,
                                  const QString &data)
{
    Q_UNUSED(time);
    Q_D(QmlProfilerClient);
    int count = d->rangeCount[type];
    if (count > 0) {
        while (d->rangeDatas[type].count() < count)
            d->rangeDatas[type].push(QStringList());
        d->rangeDatas[type][count - 1] << data;
    }
}

void QmlProfilerClient::rangeLocation(QQmlProfilerDefinitions::RangeType type, qint64 time,
                                      const QQmlEventLocation &location)
{
    Q_UNUSED(time);
    Q_D(QmlProfilerClient);
    if (d->rangeCount[type] > 0)
        d->rangeLocations[type].push(location);
}

void QmlProfilerClient::rangeEnd(QQmlProfilerDefinitions::RangeType type, qint64 endTime)
{
    Q_D(QmlProfilerClient);

    if (d->rangeCount[type] == 0) {
        emit error(tr("Spurious range end detected."));
        return;
    }

    --d->rangeCount[type];
    if (d->inProgressRanges & (static_cast<qint64>(1) << type))
        d->inProgressRanges &= ~(static_cast<qint64>(1) << type);
    QStringList data = d->rangeDatas[type].count() ? d->rangeDatas[type].pop() : QStringList();
    QQmlEventLocation location = d->rangeLocations[type].count() ? d->rangeLocations[type].pop() :
                                                                   QQmlEventLocation();
    qint64 startTime = d->rangeStartTimes[type].pop();

    if (d->rangeCount[type] == 0 && d->rangeDatas[type].count() + d->rangeStartTimes[type].count()
            + d->rangeLocations[type].count() != 0) {
        emit error(tr("Incorrectly nested range data"));
        return;
    }

    d->data->addQmlEvent(type, QQmlProfilerDefinitions::QmlBinding, startTime, endTime - startTime,
                         data, location);
}

void QmlProfilerClient::animationFrame(qint64 time, int frameRate, int animationCount, int threadId)
{
    Q_D(QmlProfilerClient);
    d->data->addFrameEvent(time, frameRate, animationCount, threadId);
}

void QmlProfilerClient::sceneGraphEvent(QQmlProfilerDefinitions::SceneGraphFrameType type,
                                        qint64 time, qint64 numericData1, qint64 numericData2,
                                        qint64 numericData3, qint64 numericData4,
                                        qint64 numericData5)
{
    Q_D(QmlProfilerClient);
    d->data->addSceneGraphFrameEvent(type, time, numericData1, numericData2, numericData3,
                                     numericData4, numericData5);
}

void QmlProfilerClient::pixmapCacheEvent(QQmlProfilerDefinitions::PixmapEventType type, qint64 time,
                                         const QString &url, int numericData1, int numericData2)
{
    Q_D(QmlProfilerClient);
    d->data->addPixmapCacheEvent(type, time, url, numericData1, numericData2);
}

void QmlProfilerClient::memoryAllocation(QQmlProfilerDefinitions::MemoryType type, qint64 time,
                                         qint64 amount)
{
    Q_D(QmlProfilerClient);
    d->data->addMemoryEvent(type, time, amount);
}

void QmlProfilerClient::inputEvent(QQmlProfilerDefinitions::InputEventType type, qint64 time,
                                   int a, int b)
{
    Q_D(QmlProfilerClient);
    d->data->addInputEvent(type, time, a, b);
}

void QmlProfilerClient::complete()
{
    Q_D(QmlProfilerClient);
    d->data->complete();
}
