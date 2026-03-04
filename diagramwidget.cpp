#include "DiagramWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <algorithm>
#include <cmath>

DiagramWidget::DiagramWidget(BeamSolver* solver, DiagramType type, QWidget* parent)
    : QWidget(parent), m_solver(solver), m_type(type)
{
    setMinimumHeight(180);
}

void DiagramWidget::refresh() { update(); }

void DiagramWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(15, 17, 23));

    BeamResults res = m_solver->solve();

    if (res.x.empty()) return;

    drawAxes(p, res);
    drawDiagram(p, res);
    drawLabels(p, res);
}

void DiagramWidget::drawAxes(QPainter& p, const BeamResults& res) {
    int ox = originX();
    int oy = originY();
    int pw = plotW();
    int ph = plotH();

    // Grid lines
    p.setPen(QPen(QColor(40, 50, 60), 1, Qt::DashLine));
    p.drawLine(ox, oy - ph/2, ox + pw, oy - ph/2);
    p.drawLine(ox, oy + ph/2, ox + pw, oy + ph/2);
    p.drawLine(ox, oy,        ox + pw, oy);

    // Baseline (x axis)
    p.setPen(QPen(QColor(60, 80, 100), 1.5));
    p.drawLine(ox, oy, ox + pw, oy);

    // Y axis
    p.setPen(QPen(QColor(60, 80, 100), 1.5));
    p.drawLine(ox, marginTop(), ox, marginTop() + ph);

    // Title
    QString title = (m_type == DiagramType::ShearForce)
                        ? "Shear Force Diagram  (kN)"
                        : "Bending Moment Diagram  (kNm)";
    QColor titleColor = (m_type == DiagramType::ShearForce)
                            ? QColor(100, 200, 255)
                            : QColor(255, 180, 80);
    p.setPen(titleColor);
    QFont tf("Monospace", 9, QFont::Bold);
    p.setFont(tf);
    p.drawText(ox + 4, marginTop() - 8, title);
}

void DiagramWidget::drawDiagram(QPainter& p, const BeamResults& res) {
    const auto& vals = (m_type == DiagramType::ShearForce) ? res.shear : res.moment;
    if (vals.empty()) return;

    double maxAbs = (m_type == DiagramType::ShearForce) ? res.maxShear : res.maxMoment;
    if (maxAbs < 0.001) maxAbs = 1.0;

    int ox = originX();
    int oy = originY();
    int pw = plotW();
    int ph = plotH();
    double scale = (ph / 2.0 - 4) / maxAbs;

    int N = static_cast<int>(res.x.size()) - 1;
    double L = m_solver->beamLength();

    auto toScreen = [&](int i) -> QPointF {
        double px = ox + (res.x[i] / L) * pw;
        double py = oy - vals[i] * scale;
        return QPointF(px, py);
    };

    // Filled area
    QColor fillCol = (m_type == DiagramType::ShearForce)
                         ? QColor(100, 200, 255, 50)
                         : QColor(255, 180, 80, 50);
    QColor lineCol = (m_type == DiagramType::ShearForce)
                         ? QColor(100, 200, 255)
                         : QColor(255, 200, 80);

    QPainterPath fillPath;
    fillPath.moveTo(ox, oy);
    for (int i = 0; i <= N; ++i) {
        fillPath.lineTo(toScreen(i));
    }
    fillPath.lineTo(ox + pw, oy);
    fillPath.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(fillCol);
    p.drawPath(fillPath);

    // Outline
    QPainterPath linePath;
    linePath.moveTo(toScreen(0));
    for (int i = 1; i <= N; ++i) {
        linePath.lineTo(toScreen(i));
    }
    p.setPen(QPen(lineCol, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    p.drawPath(linePath);

    // Zero crossings dot
    for (int i = 1; i <= N; ++i) {
        if (vals[i-1] * vals[i] < 0) {
            // linear interpolation to find exact zero
            double t = -vals[i-1] / (vals[i] - vals[i-1]);
            double zx = ox + ((res.x[i-1] + t * (res.x[i] - res.x[i-1])) / L) * pw;
            p.setPen(Qt::NoPen);
            p.setBrush(lineCol);
            p.drawEllipse(QPointF(zx, oy), 3, 3);
        }
    }
}

void DiagramWidget::drawLabels(QPainter& p, const BeamResults& res) {
    double maxVal  = (m_type == DiagramType::ShearForce) ? res.maxShear : res.maxMoment;
    double maxPos  = (m_type == DiagramType::ShearForce) ? 0.0          : res.maxMomentPos;
    double maxAbs  = maxVal;
    if (maxAbs < 0.001) return;

    int ox = originX();
    int oy = originY();
    int pw = plotW();
    int ph = plotH();
    double scale  = (ph / 2.0 - 4) / maxAbs;
    double L      = m_solver->beamLength();
    QString unit  = (m_type == DiagramType::ShearForce) ? "kN" : "kNm";

    QColor labelColor = (m_type == DiagramType::ShearForce)
                            ? QColor(100, 200, 255)
                            : QColor(255, 200, 80);

    p.setPen(labelColor);
    QFont f("Monospace", 8);
    p.setFont(f);

    // Y scale labels
    p.setPen(QColor(80, 100, 120));
    p.drawText(QPoint(2, oy - static_cast<int>(maxAbs * scale) + 4),
               QString("%1").arg(maxAbs, 0, 'f', 1));
    p.drawText(QPoint(2, oy + static_cast<int>(maxAbs * scale) + 4),
               QString("-%1").arg(maxAbs, 0, 'f', 1));

    // Max label at peak (BMD only)
    if (m_type == DiagramType::BendingMoment) {
        double px = ox + (maxPos / L) * pw;
        double py = oy - maxVal * scale;
        p.setPen(QPen(labelColor, 1, Qt::DashLine));
        p.drawLine(QPointF(px, py), QPointF(px, oy));
        p.setPen(labelColor);
        p.drawText(QPointF(px + 4, py - 2),
                   QString("%1 %2").arg(maxVal, 0, 'f', 2).arg(unit));
    }

    // X axis tick marks and labels (start, mid, end)
    p.setPen(QColor(80, 100, 120));
    QFont sf("Monospace", 7);
    p.setFont(sf);
    for (int t = 0; t <= 4; ++t) {
        double xfrac = t / 4.0;
        int tx = ox + static_cast<int>(xfrac * pw);
        p.drawLine(tx, oy - 3, tx, oy + 3);
        p.drawText(QPoint(tx - 8, oy + 16),
                   QString("%1m").arg(xfrac * L, 0, 'f', 1));
    }
}
