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

#include <cstdio>
#include <cstdlib>

#if __has_include(<graphviz/pathgeom.h>)
#include <graphviz/pathgeom.h>
#define HAVE_GRAPHVIZ_PATHGEOM_H 1
#elif __has_include(<pathgeom.h>)
#include <pathgeom.h>
#define HAVE_GRAPHVIZ_PATHGEOM_H 1
#else
#define HAVE_GRAPHVIZ_PATHGEOM_H 0
#endif

static void demoFreePath()
{
#if HAVE_GRAPHVIZ_PATHGEOM_H
    // freePath() releases both the polyline struct and its point buffer.
    // Typical flow: allocate/fill Ppolyline_t, use it, then freePath(polyline).
    auto *polyline = static_cast<Ppolyline_t *>(std::malloc(sizeof(Ppolyline_t)));
    if (!polyline) {
        std::fprintf(stderr, "demoFreePath: malloc failed for Ppolyline_t\n");
        return;
    }

    polyline->pn = 3;
    polyline->ps = static_cast<Ppoint_t *>(std::malloc(sizeof(Ppoint_t) * static_cast<size_t>(polyline->pn)));
    if (!polyline->ps) {
        std::fprintf(stderr, "demoFreePath: malloc failed for Ppoint_t[]\n");
        std::free(polyline);
        return;
    }

    polyline->ps[0] = Ppoint_t{0, 0};
    polyline->ps[1] = Ppoint_t{50, 20};
    polyline->ps[2] = Ppoint_t{100, 0};

    std::fprintf(stderr,
                 "demoFreePath: created polyline with %d points; freeing via freePath()\n",
                 polyline->pn);
    freePath(polyline);
#else
    std::fprintf(stderr,
                 "demoFreePath: <graphviz/pathgeom.h> not found.\n"
                 "Install Graphviz development headers so this demo can compile.\n");
#endif
}

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
    demoFreePath();
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
