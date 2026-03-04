#pragma once
#include <QMainWindow>
#include "BeamSolver.h"
#include "BeamWidget.h"
#include "DiagramWidget.h"
#include "ControlPanel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onParametersChanged();
    void onPointLoadMoved(double pos);

private:
    BeamSolver*    m_solver;
    BeamWidget*    m_beamWidget;
    DiagramWidget* m_sfdWidget;
    DiagramWidget* m_bmdWidget;
    ControlPanel*  m_controls;
};
