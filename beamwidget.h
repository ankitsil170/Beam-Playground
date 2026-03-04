#pragma once
#include <QWidget>
#include <QPointF>
#include "BeamSolver.h"

class BeamWidget : public QWidget {
    Q_OBJECT

public:
    explicit BeamWidget(BeamSolver* solver, QWidget* parent = nullptr);

signals:
    void pointLoadMoved(double position);  // emitted while dragging

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    BeamSolver* m_solver;
    bool        m_dragging = false;

    // Layout helpers
    QPointF beamStart() const;
    QPointF beamEnd()   const;
    double  beamPixelLength() const;
    double  posToPixel(double pos) const;
    double  pixelToPos(double px)  const;

    void drawBeam(QPainter& p);
    void drawSupports(QPainter& p);
    void drawPointLoad(QPainter& p);
    void drawUDL(QPainter& p);
    void drawLabels(QPainter& p);
    void drawReactions(QPainter& p);
};
