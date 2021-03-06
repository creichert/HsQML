#ifndef HSQML_MANAGER_H
#define HSQML_MANAGER_H

#include <QtCore/QAtomicPointer>
#include <QtCore/QAtomicInt>
#include <QtCore/QByteArray>
#include <QtCore/QMutex>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtWidgets/QApplication>

#include "hsqml.h"
#include "Engine.h"

#define HSQML_LOG(ll, msg) if (gManager->checkLogLevel(ll)) gManager->log(msg)

class HsQMLManagerApp;
class HsQMLObjectEvent;

class HsQMLManager
{
public:
    enum CounterId {
        ClassCount,
        ObjectCount,
        QObjectCount,
        VariantCount,
        ClassSerial,
        ObjectSerial,
        TotalCounters
    }; 

    HsQMLManager(
        void (*)(HsFunPtr),
        void (*)(HsStablePtr));
    void setLogLevel(int);
    bool checkLogLevel(int);
    void log(const QString&);
    int updateCounter(CounterId, int);
    void freeFun(HsFunPtr);
    void freeStable(HsStablePtr);
    bool setArgs(const QStringList&);
    QVector<char*>& argsPtrs();
    void registerObject(const QObject*);
    void unregisterObject(const QObject*);
    void hookedConstruct(QVariant::Private*, const void*);
    void hookedClear(QVariant::Private*);
    bool isEventThread();
    typedef HsQMLEventLoopStatus EventLoopStatus;
    EventLoopStatus runEventLoop(
        HsQMLTrivialCb, HsQMLTrivialCb, HsQMLTrivialCb);
    EventLoopStatus requireEventLoop();
    void releaseEventLoop();
    void notifyJobs();
    void createEngine(const HsQMLEngineConfig&);
    void setActiveEngine(HsQMLEngine*);
    HsQMLEngine* activeEngine();
    void postObjectEvent(HsQMLObjectEvent*);
    EventLoopStatus shutdown();

private:
    friend class HsQMLManagerApp;
    Q_DISABLE_COPY(HsQMLManager)

    int mLogLevel;
    QMutex mLogLock;
    QAtomicInt mCounters[TotalCounters];
    bool mAtExit;
    void (*mFreeFun)(HsFunPtr);
    void (*mFreeStable)(HsStablePtr);
    QVector<QByteArray> mArgs;
    QVector<char*> mArgsPtrs;
    QSet<const QObject*> mObjectSet;
    const QVariant::Handler* mOriginalHandler;
    HsQMLManagerApp* mApp;
    QMutex mLock;
    bool mRunning;
    int mRunCount;
    bool mShutdown;
    void* mStackBase;
    HsQMLTrivialCb mStartCb;
    HsQMLTrivialCb mJobsCb;
    HsQMLTrivialCb mYieldCb;
    HsQMLEngine* mActiveEngine;
};

class HsQMLManagerApp : public QObject
{
    Q_OBJECT

public:
    HsQMLManagerApp();
    virtual ~HsQMLManagerApp();
    virtual void customEvent(QEvent*);
    virtual void timerEvent(QTimerEvent*);
    Q_SLOT void createEngine(HsQMLEngineConfig);
    int exec();

    enum CustomEventIndicies {
        StartedLoopEventIndex,
        StopLoopEventIndex,
        PendingJobsEventIndex,
        RemoveGCLockEventIndex
    };

    static const QEvent::Type StartedLoopEvent =
        static_cast<QEvent::Type>(QEvent::User+StartedLoopEventIndex);
    static const QEvent::Type StopLoopEvent =
        static_cast<QEvent::Type>(QEvent::User+StopLoopEventIndex);
    static const QEvent::Type PendingJobsEvent =
        static_cast<QEvent::Type>(QEvent::User+PendingJobsEventIndex);
    static const QEvent::Type RemoveGCLockEvent =
        static_cast<QEvent::Type>(QEvent::User+RemoveGCLockEventIndex);

private:
    Q_DISABLE_COPY(HsQMLManagerApp)

    QVariant::Handler mHookedHandler;
    int mArgC;
    QApplication mApp;
};

class ManagerPointer : public QAtomicPointer<HsQMLManager>
{
public:
    HsQMLManager* operator->() const
    {
        return load();
    }

    operator HsQMLManager*() const
    {
        return load();
    }
};

extern ManagerPointer gManager;

#endif /*HSQML_MANAGER_H*/
