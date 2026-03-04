#include "ControlPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <cmath>

static QString sectionStyle() {
    return R"(
        QGroupBox {
            color: #6080a0;
            font: bold 9px 'Monospace';
            border: 1px solid #1e2a38;
            border-radius: 4px;
            margin-top: 10px;
            padding-top: 6px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px;
            top: -1px;
            color: #5090c0;
        }
    )";
}

ControlPanel::ControlPanel(BeamSolver* solver, QWidget* parent)
    : QWidget(parent), m_solver(solver)
{
    setFixedWidth(240);
    applyDarkStyle();

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // ── BEAM LENGTH ──────────────────────────────────────
    auto* lenGroup = new QGroupBox("Beam Length", this);
    lenGroup->setStyleSheet(sectionStyle());
    auto* lenLayout = new QVBoxLayout(lenGroup);
    m_beamLenLabel = makeLabel("L = 6.0 m");
    m_beamLenSlider = makeSlider(20, 120, 60);  // /10 = metres
    connect(m_beamLenSlider, &QSlider::valueChanged, this, &ControlPanel::onBeamLengthChanged);
    lenLayout->addWidget(m_beamLenLabel);
    lenLayout->addWidget(m_beamLenSlider);
    mainLayout->addWidget(lenGroup);

    // ── SUPPORTS ─────────────────────────────────────────
    auto* supGroup = new QGroupBox("Support Conditions", this);
    supGroup->setStyleSheet(sectionStyle());
    auto* supLayout = new QVBoxLayout(supGroup);

    auto addCombo = [&](const QString& label, QComboBox*& combo) {
        auto* lbl = makeLabel(label);
        combo = new QComboBox(this);
        combo->addItems({"Pinned", "Roller", "Fixed"});
        combo->setStyleSheet(R"(
            QComboBox {
                background: #0f1117;
                color: #a0c0d0;
                border: 1px solid #1e2a38;
                border-radius: 3px;
                padding: 3px 6px;
                font: 9px 'Monospace';
            }
            QComboBox QAbstractItemView {
                background: #0f1117;
                color: #a0c0d0;
                selection-background-color: #1e3050;
            }
        )");
        connect(combo, &QComboBox::currentIndexChanged, this, &ControlPanel::onSupportChanged);
        supLayout->addWidget(lbl);
        supLayout->addWidget(combo);
    };

    addCombo("Left Support:", m_leftSupportCombo);
    m_leftSupportCombo->setCurrentIndex(0); // Pinned

    addCombo("Right Support:", m_rightSupportCombo);
    m_rightSupportCombo->setCurrentIndex(1); // Roller
    mainLayout->addWidget(supGroup);

    // ── POINT LOAD ────────────────────────────────────────
    auto* pGroup = new QGroupBox("Point Load", this);
    pGroup->setStyleSheet(sectionStyle());
    auto* pLayout = new QVBoxLayout(pGroup);

    m_loadMagLabel = makeLabel("Magnitude: 10.0 kN");
    m_loadMagSlider = makeSlider(1, 100, 100);  // /10 = kN
    connect(m_loadMagSlider, &QSlider::valueChanged, this, &ControlPanel::onLoadMagnitudeChanged);

    m_loadPosLabel = makeLabel("Position: 3.00 m  (drag on beam)");
    m_loadPosLabel->setWordWrap(true);

    pLayout->addWidget(m_loadMagLabel);
    pLayout->addWidget(m_loadMagSlider);
    pLayout->addWidget(m_loadPosLabel);
    mainLayout->addWidget(pGroup);

    // ── UDL ───────────────────────────────────────────────
    auto* uGroup = new QGroupBox("Uniform Distributed Load", this);
    uGroup->setStyleSheet(sectionStyle());
    auto* uLayout = new QVBoxLayout(uGroup);

    m_udlCheck = new QCheckBox("Enable UDL", this);
    m_udlCheck->setStyleSheet("color: #a0b0c0; font: 9px 'Monospace';");
    connect(m_udlCheck, &QCheckBox::toggled, this, &ControlPanel::onUDLToggled);

    m_udlIntensLabel = makeLabel("Intensity: 5.0 kN/m");
    m_udlIntensSlider = makeSlider(1, 100, 50);  // /10 = kN/m
    m_udlIntensSlider->setEnabled(false);
    connect(m_udlIntensSlider, &QSlider::valueChanged, this, &ControlPanel::onUDLIntensityChanged);

    uLayout->addWidget(m_udlCheck);
    uLayout->addWidget(m_udlIntensLabel);
    uLayout->addWidget(m_udlIntensSlider);
    mainLayout->addWidget(uGroup);

    // ── RESULTS ───────────────────────────────────────────
    auto* rGroup = new QGroupBox("Results", this);
    rGroup->setStyleSheet(sectionStyle());
    auto* rLayout = new QVBoxLayout(rGroup);

    m_raLabel   = makeLabel("Ra = —");
    m_rbLabel   = makeLabel("Rb = —");
    m_maLabel   = makeLabel("Ma = —");
    m_maxMLabel = makeLabel("M_max = —");
    m_maxVLabel = makeLabel("V_max = —");

    for (auto* lbl : {m_raLabel, m_rbLabel, m_maLabel, m_maxMLabel, m_maxVLabel}) {
        lbl->setStyleSheet("color: #80ffc0; font: bold 9px 'Monospace'; padding: 1px 0;");
        rLayout->addWidget(lbl);
    }

    mainLayout->addWidget(rGroup);
    mainLayout->addStretch();

    // ── Footer ───────────────────────────────────────────
    auto* footer = makeLabel("⚡ BeamPlayground v1.0");
    footer->setStyleSheet("color: #304050; font: 8px 'Monospace'; padding: 4px 0;");
    footer->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(footer);
}

void ControlPanel::applyDarkStyle() {
    setStyleSheet(R"(
        QWidget {
            background: #0a0d14;
            color: #8090a0;
        }
        QSlider::groove:horizontal {
            background: #1a2030;
            height: 4px;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #40a0d0;
            width: 12px;
            height: 12px;
            border-radius: 6px;
            margin: -4px 0;
        }
        QSlider::sub-page:horizontal {
            background: #40a0d0;
            border-radius: 2px;
        }
    )");
}

QLabel* ControlPanel::makeLabel(const QString& text) {
    auto* l = new QLabel(text, this);
    l->setStyleSheet("color: #6080a0; font: 9px 'Monospace';");
    return l;
}

QSlider* ControlPanel::makeSlider(int mn, int mx, int val, Qt::Orientation orient) {
    auto* s = new QSlider(orient, this);
    s->setRange(mn, mx);
    s->setValue(val);
    return s;
}

void ControlPanel::onSupportChanged() {
    auto toType = [](int idx) -> SupportType {
        switch (idx) {
        case 0: return SupportType::Pinned;
        case 1: return SupportType::Roller;
        case 2: return SupportType::Fixed;
        default: return SupportType::Pinned;
        }
    };
    m_solver->setSupports(
        toType(m_leftSupportCombo->currentIndex()),
        toType(m_rightSupportCombo->currentIndex())
        );
    emit parametersChanged();
}

void ControlPanel::onLoadMagnitudeChanged(int val) {
    double mag = val / 10.0;
    m_loadMagLabel->setText(QString("Magnitude: %1 kN").arg(mag, 0, 'f', 1));
    m_solver->setPointLoad(m_solver->pointLoadPos(), mag);
    emit parametersChanged();
}

void ControlPanel::onUDLToggled(bool on) {
    m_solver->setUDLActive(on);
    m_udlIntensSlider->setEnabled(on);
    emit parametersChanged();
}

void ControlPanel::onUDLIntensityChanged(int val) {
    double intens = val / 10.0;
    m_udlIntensLabel->setText(QString("Intensity: %1 kN/m").arg(intens, 0, 'f', 1));
    m_solver->setUDLIntensity(intens);
    emit parametersChanged();
}

void ControlPanel::onBeamLengthChanged(int val) {
    double len = val / 10.0;
    m_beamLenLabel->setText(QString("L = %1 m").arg(len, 0, 'f', 1));
    m_solver->setBeamLength(len);
    emit parametersChanged();
}

void ControlPanel::updateReactionDisplay(const BeamResults& res) {
    m_raLabel->setText(QString("Ra    = %1 kN").arg(res.Ra, 7, 'f', 2));
    m_rbLabel->setText(QString("Rb    = %1 kN").arg(res.Rb, 7, 'f', 2));

    bool showMa = (m_solver->leftSupport() == SupportType::Fixed);
    m_maLabel->setVisible(showMa);
    if (showMa)
        m_maLabel->setText(QString("Ma    = %1 kNm").arg(res.Ma, 7, 'f', 2));

    m_maxMLabel->setText(QString("M_max = %1 kNm").arg(res.maxMoment, 7, 'f', 2));
    m_maxVLabel->setText(QString("V_max = %1 kN").arg(res.maxShear, 7, 'f', 2));
}

void ControlPanel::setLoadPosition(double pos) {
    m_loadPosLabel->setText(
        QString("Position: %1 m  (drag on beam)").arg(pos, 0, 'f', 2));
}
