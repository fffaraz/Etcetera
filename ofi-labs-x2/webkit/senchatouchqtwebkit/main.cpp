#include <QtGui>
#include <QtWebKit>

#if defined(Q_OS_SYMBIAN) && defined(ORIENTATIONLOCK)
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif // Q_OS_SYMBIAN && ORIENTATIONLOCK

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if defined(Q_OS_SYMBIAN) && defined(ORIENTATIONLOCK)
    const CAknAppUiBase::TAppUiOrientation uiOrientation = CAknAppUi::EAppUiOrientationLandscape;
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*> (CEikonEnv::Static()->AppUi());
    TRAPD(error,
        if (appUi)
            appUi->SetOrientationL(uiOrientation);
    );
    Q_UNUSED(error)
#endif // ORIENTATIONLOCK

    const QSize screenSize(640, 360);

    QGraphicsScene scene;

    QGraphicsView view(&scene);
    view.setFrameShape(QFrame::NoFrame);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsWebView webview;
    webview.resize(screenSize);
    webview.load(QString::fromLatin1("html/examples/kitchensink/index.html"));

    scene.addItem(&webview);

#if defined(Q_OS_SYMBIAN)
    view.showFullScreen();
#elif defined(Q_WS_MAEMO_5)
    view.showMaximized();
#else
    view.resize(screenSize);
    view.show();
#endif

    return app.exec();
}
