#ifndef HSQML_CANVAS_H
#define HSQML_CANVAS_H

#include <QtCore/QObject>
#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QMetaType>
#include <QtCore/QScopedPointer>
#include <QtCore/QSharedData>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QQuickItem>

#include "hsqml.h"

class QSGTexture;
class QQuickWindow;
class HsQMLCanvasBackEnd;

class HsQMLGLCallbacks : public QSharedData
{
public:
    HsQMLGLCallbacks(
        HsQMLGLDeInitCb, HsQMLGLDeInitCb, HsQMLGLSyncCb, HsQMLGLPaintCb);
    ~HsQMLGLCallbacks();

    HsQMLGLDeInitCb mInitCb;
    HsQMLGLDeInitCb mDeinitCb;
    HsQMLGLSyncCb mSyncCb;
    HsQMLGLPaintCb mPaintCb;
};

class HsQMLGLDelegateImpl : public QSharedData
{
public:
    HsQMLGLDelegateImpl(HsQMLGLMakeCallbacksCb);
    ~HsQMLGLDelegateImpl();

    HsQMLGLMakeCallbacksCb mMakeCallbacksCb;
};

class HsQMLGLDelegate
{
public:
    HsQMLGLDelegate();
    ~HsQMLGLDelegate();
    void setup(HsQMLGLMakeCallbacksCb);
    typedef QExplicitlySharedDataPointer<HsQMLGLCallbacks> CallbacksRef;
    CallbacksRef makeCallbacks();

private:
    QExplicitlySharedDataPointer<HsQMLGLDelegateImpl> mImpl;
};

Q_DECLARE_METATYPE(HsQMLGLDelegate)

class HsQMLCanvas : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(DisplayMode)
    Q_PROPERTY(DisplayMode displayMode READ displayMode WRITE setDisplayMode
        NOTIFY displayModeChanged)
    Q_PROPERTY(qreal canvasWidth READ canvasWidth WRITE setCanvasWidth
        RESET unsetCanvasWidth NOTIFY canvasWidthChanged)
    Q_PROPERTY(qreal canvasHeight READ canvasHeight WRITE setCanvasHeight
        RESET unsetCanvasHeight NOTIFY canvasHeightChanged)
    Q_PROPERTY(QVariant delegate READ delegate WRITE setDelegate
        NOTIFY delegateChanged)
    Q_PROPERTY(QJSValue model READ model WRITE setModel
        NOTIFY modelChanged)

public:
    enum DisplayMode {Above, Below, Inline};

    HsQMLCanvas(QQuickItem* = NULL);
    ~HsQMLCanvas();

private:
    Q_DISABLE_COPY(HsQMLCanvas);

    void geometryChanged(const QRectF&, const QRectF&) Q_DECL_OVERRIDE;
    QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*) Q_DECL_OVERRIDE;
    void detachBackEnd();
    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode);
    qreal canvasWidth() const;
    void setCanvasWidth(qreal, bool = true);
    void unsetCanvasWidth();
    qreal canvasHeight() const;
    void setCanvasHeight(qreal, bool = true);
    void unsetCanvasHeight();
    QVariant delegate() const;
    void setDelegate(const QVariant&);
    QJSValue model() const;
    void setModel(const QJSValue&);
    Q_SIGNAL void displayModeChanged();
    Q_SIGNAL void canvasWidthChanged();
    Q_SIGNAL void canvasHeightChanged();
    Q_SIGNAL void delegateChanged();
    Q_SIGNAL void modelChanged();
    Q_SLOT void doWindowChanged(QQuickWindow*);

    QQuickWindow* mWindow;
    HsQMLCanvasBackEnd* mBackEnd;
    DisplayMode mDisplayMode;
    qreal mCanvasWidth;
    bool mCanvasWidthSet;
    qreal mCanvasHeight;
    bool mCanvasHeightSet;
    QVariant mDelegate;
    HsQMLGLDelegate::CallbacksRef mGLCallbacks;
    QJSValue mModel;
    bool mLoadModel;
    bool mValidModel;
};

class HsQMLCanvasBackEnd : public QObject
{
    Q_OBJECT

public:
    HsQMLCanvasBackEnd(
        QQuickWindow*, const HsQMLGLDelegate::CallbacksRef&);
    ~HsQMLCanvasBackEnd();
    void setModeSize(HsQMLCanvas::DisplayMode, qreal, qreal);
    QSGTexture* updateFBO();

private:
    Q_DISABLE_COPY(HsQMLCanvasBackEnd)

    Q_SLOT void doRendering();
    Q_SLOT void doCleanup();
    Q_SLOT void doCleanupKill();

    QQuickWindow* mWindow;
    HsQMLGLDelegate::CallbacksRef mGLCallbacks;
    QOpenGLContext* mGL;
    HsQMLCanvas::DisplayMode mDisplayMode;
    qreal mCanvasWidth;
    qreal mCanvasHeight;
    QScopedPointer<QOpenGLFramebufferObject> mFBO;
    QScopedPointer<QSGTexture> mTexture;
};

#endif //HSQML_CANVAS_H
