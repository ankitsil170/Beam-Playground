#include "MainWindow.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QFrame>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("⚡ Beam Playground — Interactive Structural Analysis");
    resize(1100, 750);

    // Dark title bar approximation via stylesheet
    setStyleSheet(R"(
        QMainWindow { background: #0a0d14; }
        QSplitter::handle { background: #1a2030; }
    )");

    m_solver = new BeamSolver();

    // Central widget
    auto* central = new QWidget(this);
    central->setStyleSheet("background: #0a0d14;");
    setCentralWidget(central);

    auto* rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ── Left panel: controls ─────────────────────────────
    m_controls = new ControlPanel(m_solver, this);
    rootLayout->addWidget(m_controls);

    // Divider
    auto* divider = new QFrame(this);
    divider->setFrameShape(QFrame::VLine);
    divider->setFixedWidth(1);
    divider->setStyleSheet("background: #1a2030;");
    rootLayout->addWidget(divider);

    // ── Right panel: visualizations ──────────────────────
    auto* rightWidget = new QWidget(this);
    rightWidget->setStyleSheet("background: #0a0d14;");
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(2);
    rightLayout->setContentsMargins(8, 8, 8, 8);

    // Header
    auto* header = new QLabel("BEAM PLAYGROUND", this);
    header->setStyleSheet(R"(
        color: #204060;
        font: bold 10px 'Monospace';
        letter-spacing: 4px;
        padding: 4px 0 8px 0;
    )");
    rightLayout->addWidget(header);

    // Beam widget
    m_beamWidget = new BeamWidget(m_solver, this);
    m_beamWidget->setStyleSheet("border-radius: 6px;");
    rightLayout->addWidget(m_beamWidget);

    // Separator
    auto* sep1 = new QFrame(this);
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFixedHeight(1);
    sep1->setStyleSheet("background: #1a2030;");
    rightLayout->addWidget(sep1);

    // SFD
    m_sfdWidget = new DiagramWidget(m_solver, DiagramType::ShearForce, this);
    rightLayout->addWidget(m_sfdWidget);

    auto* sep2 = new QFrame(this);
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFixedHeight(1);
    sep2->setStyleSheet("background: #1a2030;");
    rightLayout->addWidget(sep2);

    // BMD
    m_bmdWidget = new DiagramWidget(m_solver, DiagramType::BendingMoment, this);
    rightLayout->addWidget(m_bmdWidget);

    rootLayout->addWidget(rightWidget, 1);

    // ── Connections ──────────────────────────────────────
    connect(m_controls, &ControlPanel::parametersChanged,
            this,       &MainWindow::onParametersChanged);
    connect(m_beamWidget, &BeamWidget::pointLoadMoved,
            this,         &MainWindow::onPointLoadMoved);

    // Initial render
    onParametersChanged();
}

void MainWindow::onParametersChanged() {
    m_beamWidget->update();
    m_sfdWidget->refresh();
    m_bmdWidget->refresh();

    BeamResults res = m_solver->solve();
    m_controls->updateReactionDisplay(res);
}

void MainWindow::onPointLoadMoved(double pos) {
    m_solver->setPointLoad(pos, m_solver->pointLoadMag());
    m_controls->setLoadPosition(pos);
    onParametersChanged();
}
