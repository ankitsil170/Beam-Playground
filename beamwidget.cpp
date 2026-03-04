#include "BeamWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QFontMetrics>
#include <cmath>

static const int   BEAM_Y_FRAC  = 50;   // beam drawn at 50% height
static const int   MARGIN_X     = 80;   // px margin each side
static const double LOAD_ARROW_H = 70.0; // px height of load arrow
static const double SNAP_RADIUS  = 20.0; // px snap zone for dragging

BeamWidget::BeamWidget(BeamSolver* solver, QWidget* parent)
    : QWidget(parent), m_solver(solver)
{
    setMinimumHeight(220);
    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
}

QPointF BeamWidget::beamStart() const {
    return QPointF(MARGIN_X, height() * BEAM_Y_FRAC / 100.0);
}

QPointF BeamWidget::beamEnd() const {
    return QPointF(width() - MARGIN_X, height() * BEAM_Y_FRAC / 100.0);
}

double BeamWidget::beamPixelLength() const {
    return beamEnd().x() - beamStart().x();
}

double BeamWidget::posToPixel(double pos) const {
    return beamStart().x() + (pos / m_solver->beamLength()) * beamPixelLength();
}

double BeamWidget::pixelToPos(double px) const {
    double t = (px - beamStart().x()) / beamPixelLength();
    t = std::clamp(t, 0.0, 1.0);
    return t * m_solver->beamLength();
}

void BeamWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    p.fillRect(rect(), QColor(15, 17, 23));

    drawUDL(p);
    drawBeam(p);
    drawSupports(p);
    drawPointLoad(p);
    drawReactions(p);
    drawLabels(p);
}

void BeamWidget::drawBeam(QPainter& p) {
    double by = beamStart().y();
    double bx1 = beamStart().x();
    double bx2 = beamEnd().x();

    // Beam shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 200, 150, 30));
    p.drawRect(QRectF(bx1, by - 7, bx2 - bx1, 14));

    // Beam body
    QLinearGradient grad(0, by - 6, 0, by + 6);
    grad.setColorAt(0.0, QColor(100, 200, 180));
    grad.setColorAt(0.5, QColor(60, 160, 140));
    grad.setColorAt(1.0, QColor(30, 100, 90));
    p.setBrush(grad);
    p.setPen(QPen(QColor(120, 220, 200), 1));
    p.drawRect(QRectF(bx1, by - 6, bx2 - bx1, 12));

    // Length label
    p.setPen(QColor(120, 140, 160));
    QFont f("Monospace", 8);
    p.setFont(f);
    QString lenStr = QString("L = %1 m").arg(m_solver->beamLength(), 0, 'f', 1);
    p.drawText(QRectF(bx1, by + 20, bx2 - bx1, 20), Qt::AlignHCenter, lenStr);
}

void BeamWidget::drawSupports(QPainter& p) {
    auto drawPin = [&](double cx, double by, SupportType type, bool flip) {
        double triH = 30.0;
        double triW = 26.0;
        double sign = flip ? -1.0 : 1.0;

        QPainterPath tri;
        tri.moveTo(cx, by);
        tri.lineTo(cx - triW/2, by + sign * triH);
        tri.lineTo(cx + triW/2, by + sign * triH);
        tri.closeSubpath();

        p.setPen(QPen(QColor(255, 200, 80), 1.5));
        p.setBrush(QColor(255, 200, 80, 40));
        p.drawPath(tri);

        if (type == SupportType::Pinned || type == SupportType::Roller) {
            // Ground hatch
            double gx = cx - triW/2 - 6;
            double gy = by + sign * triH;
            p.setPen(QPen(QColor(255, 200, 80, 160), 1.5));
            p.drawLine(QPointF(gx, gy), QPointF(cx + triW/2 + 6, gy));
            for (int i = 0; i < 6; ++i) {
                double hx = gx + i * (triW + 12) / 5.0;
                p.drawLine(QPointF(hx, gy), QPointF(hx - 8, gy + sign * 8));
            }
        }

        if (type == SupportType::Roller) {
            // Roller circles
            double ry = by + sign * (triH + 6);
            for (int i = -1; i <= 1; ++i) {
                p.setPen(QPen(QColor(255, 200, 80), 1.5));
                p.setBrush(QColor(255, 200, 80, 60));
                p.drawEllipse(QPointF(cx + i * 9, ry), 4, 4);
            }
        }

        if (type == SupportType::Fixed) {
            // Fixed wall
            double wx = flip ? cx + 8 : cx - 8;
            double wy = by - 30;
            double wh = 60;
            p.setPen(QPen(QColor(255, 200, 80), 2));
            p.setBrush(QColor(255, 200, 80, 30));
            p.drawRect(QRectF(wx, wy, flip ? -14 : 14, wh));
            // Hatch inside wall
            p.setPen(QPen(QColor(255, 200, 80, 100), 1));
            for (int i = 0; i < 5; ++i) {
                double hy = wy + i * (wh / 4.0);
                double dx = flip ? -10 : 10;
                p.drawLine(QPointF(wx, hy), QPointF(wx + dx, hy + 10));
            }
        }
    };

    double by = beamStart().y();
    drawPin(beamStart().x(), by, m_solver->leftSupport(),  false);
    drawPin(beamEnd().x(),   by, m_solver->rightSupport(), false);
}

void BeamWidget::drawPointLoad(QPainter& p) {
    double px  = posToPixel(m_solver->pointLoadPos());
    double by  = beamStart().y();
    double mag = m_solver->pointLoadMag();
    double dir = (mag >= 0) ? 1.0 : -1.0; // positive = downward

    double arrowTip = by - 6 * dir;        // arrow touches beam top
    double arrowTail= arrowTip - LOAD_ARROW_H * dir;

    // Glow under cursor when nearby
    bool hover = m_dragging;

    QColor col  = hover ? QColor(255, 120, 60)  : QColor(255, 80, 80);
    QColor colA = hover ? QColor(255, 120, 60, 80) : QColor(255, 80, 80, 60);

    // Shaft
    p.setPen(QPen(col, 2.5, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(px, arrowTail), QPointF(px, arrowTip));

    // Arrowhead
    QPainterPath arrow;
    double ah = 12 * dir;
    arrow.moveTo(px, arrowTip);
    arrow.lineTo(px - 7, arrowTip - ah);
    arrow.lineTo(px + 7, arrowTip - ah);
    arrow.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(col);
    p.drawPath(arrow);

    // Drag handle circle at tail
    p.setPen(QPen(col, 2));
    p.setBrush(colA);
    p.drawEllipse(QPointF(px, arrowTail), 7, 7);

    // Label
    p.setPen(col);
    QFont f("Monospace", 9, QFont::Bold);
    p.setFont(f);
    QString label = QString("P = %1 kN").arg(std::abs(mag), 0, 'f', 1);
    double lx = px + 12;
    if (lx + 80 > width()) lx = px - 90;
    p.drawText(QPointF(lx, arrowTail - 4), QString("x = %1 m").arg(m_solver->pointLoadPos(), 0, 'f', 2));
    p.drawText(QPointF(lx, arrowTail + 12), label);
}

void BeamWidget::drawUDL(QPainter& p) {
    if (!m_solver->udlActive()) return;

    double by  = beamStart().y();
    double bx1 = beamStart().x();
    double bx2 = beamEnd().x();
    double udlH = 40.0;
    double topY = by - 6 - udlH;

    // Fill
    QLinearGradient g(0, topY, 0, by - 6);
    g.setColorAt(0, QColor(100, 150, 255, 80));
    g.setColorAt(1, QColor(100, 150, 255, 20));
    p.fillRect(QRectF(bx1, topY, bx2 - bx1, udlH), g);

    // Top line
    p.setPen(QPen(QColor(120, 160, 255), 1.5));
    p.drawLine(QPointF(bx1, topY), QPointF(bx2, topY));

    // Arrows
    int nArrows = 10;
    for (int i = 0; i <= nArrows; ++i) {
        double ax = bx1 + i * (bx2 - bx1) / nArrows;
        p.setPen(QPen(QColor(120, 160, 255), 1.5));
        p.drawLine(QPointF(ax, topY), QPointF(ax, by - 8));
        QPainterPath arh;
        arh.moveTo(ax,     by - 8);
        arh.lineTo(ax - 4, by - 18);
        arh.lineTo(ax + 4, by - 18);
        arh.closeSubpath();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(120, 160, 255));
        p.drawPath(arh);
    }

    // Label
    p.setPen(QColor(120, 160, 255));
    QFont f("Monospace", 8);
    p.setFont(f);
    QString label = QString("w = %1 kN/m").arg(m_solver->udlIntensity(), 0, 'f', 1);
    p.drawText(QPointF(bx1 + (bx2 - bx1) / 2 - 35, topY - 5), label);
}

void BeamWidget::drawReactions(QPainter& p) {
    BeamResults res = m_solver->solve();
    double by  = beamStart().y();
    double bxL = beamStart().x();
    double bxR = beamEnd().x();

    auto drawReaction = [&](double cx, double val, bool isFixed, double fixedM) {
        if (std::abs(val) < 0.001) return;
        double arrowLen = 55.0;
        double tipY = by + 6;          // bottom of beam
        double tailY = tipY + arrowLen; // upward reaction arrow

        QColor rc(80, 220, 120);
        p.setPen(QPen(rc, 2));
        p.drawLine(QPointF(cx, tailY), QPointF(cx, tipY));
        QPainterPath arh;
        arh.moveTo(cx, tipY);
        arh.lineTo(cx - 6, tipY + 12);
        arh.lineTo(cx + 6, tipY + 12);
        arh.closeSubpath();
        p.setPen(Qt::NoPen);
        p.setBrush(rc);
        p.drawPath(arh);

        // Value label
        p.setPen(rc);
        QFont f("Monospace", 8, QFont::Bold);
        p.setFont(f);
        double lx = cx + 8;
        if (lx + 70 > width() - 10) lx = cx - 75;
        p.drawText(QPointF(lx, tailY + 4),
                   QString("%1 kN").arg(val, 0, 'f', 2));

        if (isFixed && std::abs(fixedM) > 0.001) {
            // Draw moment arc
            p.setPen(QPen(QColor(255, 180, 60), 2));
            QRectF arc(cx - 18, by - 18, 36, 36);
            p.drawArc(arc, 30 * 16, 300 * 16);
            p.setPen(QColor(255, 180, 60));
            QFont fm("Monospace", 8);
            p.setFont(fm);
            p.drawText(QPointF(cx + 20, by - 10),
                       QString("M = %1 kNm").arg(fixedM, 0, 'f', 2));
        }
    };

    bool leftFixed = (m_solver->leftSupport() == SupportType::Fixed);
    drawReaction(bxL, res.Ra, leftFixed, res.Ma);
    drawReaction(bxR, res.Rb, false, 0);
}

void BeamWidget::drawLabels(QPainter& p) {
    BeamResults res = m_solver->solve();
    QFont f("Monospace", 9, QFont::Bold);
    p.setFont(f);

    // Max moment annotation
    double mx = posToPixel(res.maxMomentPos);
    double by = beamStart().y();
    p.setPen(QPen(QColor(255, 200, 80, 120), 1, Qt::DashLine));
    p.drawLine(QPointF(mx, by - 6), QPointF(mx, by - 55));
    p.setPen(QColor(255, 200, 80));
    p.drawText(QPointF(mx + 5, by - 40),
               QString("M_max = %1 kNm").arg(res.maxMoment, 0, 'f', 2));
}

void BeamWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) return;
    double px = posToPixel(m_solver->pointLoadPos());
    double ex = event->position().x();
    if (std::abs(ex - px) < SNAP_RADIUS) {
        m_dragging = true;
        setCursor(Qt::ClosedHandCursor);
    }
}

void BeamWidget::mouseMoveEvent(QMouseEvent* event) {
    double ex = event->position().x();
    double px = posToPixel(m_solver->pointLoadPos());

    if (m_dragging) {
        double newPos = pixelToPos(ex);
        emit pointLoadMoved(newPos);
        update();
    } else {
        if (std::abs(ex - px) < SNAP_RADIUS)
            setCursor(Qt::OpenHandCursor);
        else
            setCursor(Qt::ArrowCursor);
    }
}

void BeamWidget::mouseReleaseEvent(QMouseEvent*) {
    m_dragging = false;
    setCursor(Qt::ArrowCursor);
}

void BeamWidget::resizeEvent(QResizeEvent*) {
    update();
}
