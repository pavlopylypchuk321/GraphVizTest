#include <QApplication>
#include <QByteArray>
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <QSvgWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <graphviz/cgraph.h>
#include <graphviz/gvc.h>

static QByteArray renderDotToSvg(const char *dot)
{
    GVC_t *gvc = gvContext();
    if (!gvc)
        return {};

    QByteArray mutableDot = QByteArray(dot) + '\0';
    Agraph_t *g = agmemread(mutableDot.data());
    if (!g) {
        gvFreeContext(gvc);
        return {};
    }

    if (gvLayout(gvc, g, "dot") != 0) {
        agclose(g);
        gvFreeContext(gvc);
        return {};
    }

    char *result = nullptr;
    unsigned int length = 0;
    if (gvRenderData(gvc, g, "svg", &result, &length) != 0 || !result) {
        gvFreeLayout(gvc, g);
        agclose(g);
        gvFreeContext(gvc);
        return {};
    }

    QByteArray svg(result, static_cast<int>(length));
    gvFreeRenderData(result);
    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);
    return svg;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    static const char kSampleDot[] = R"(digraph G {
  rankdir=LR;
  hello -> world [label="GraphViz + Qt"];
  world -> qt;
})";

    const QByteArray svg = renderDotToSvg(kSampleDot);
    if (svg.isEmpty()) {
        QMainWindow w;
        w.setCentralWidget(new QLabel(QStringLiteral("GraphViz failed to render SVG.")));
        w.resize(480, 120);
        w.show();
        return app.exec();
    }

    auto *svgWidget = new QSvgWidget;
    svgWidget->load(svg);

    auto *scroll = new QScrollArea;
    scroll->setWidget(svgWidget);
    scroll->setWidgetResizable(true);

    auto *central = new QWidget;
    auto *layout = new QVBoxLayout(central);
    layout->addWidget(scroll);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle(QStringLiteral("Qt + GraphViz (MacPorts)"));
    mainWindow.setCentralWidget(central);
    mainWindow.resize(640, 480);
    mainWindow.show();

    return app.exec();
}
