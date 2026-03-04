#pragma once
#include <QWidget>
#include "BeamSolver.h"

enum class DiagramType { ShearForce, BendingMoment };

class DiagramWidget : public QWidget {
    Q_OBJECT

public:
    DiagramWidget(BeamSolver* solver, DiagramType type, QWidget* parent = nullptr);

    void refresh();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    BeamSolver*  m_solver;
    DiagramType  m_type;

    void drawAxes(QPainter& p, const BeamResults& res);
    void drawDiagram(QPainter& p, const BeamResults& res);
    void drawLabels(QPainter& p, const BeamResults& res);

    // Layout
    int marginLeft()   const { return 60; }
    int marginRight()  const { return 20; }
    int marginTop()    const { return 30; }
    int marginBottom() const { return 30; }

    int plotW() const { return width()  - marginLeft() - marginRight(); }
    int plotH() const { return height() - marginTop()  - marginBottom(); }
    int originX() const { return marginLeft(); }
    int originY() const { return marginTop() + plotH() / 2; }
};
