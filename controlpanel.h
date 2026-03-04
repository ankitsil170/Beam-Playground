#pragma once
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include "BeamSolver.h"

class ControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit ControlPanel(BeamSolver* solver, QWidget* parent = nullptr);

    void updateReactionDisplay(const BeamResults& res);
    void setLoadPosition(double pos);   // called from beam drag

signals:
    void parametersChanged();

private slots:
    void onSupportChanged();
    void onLoadMagnitudeChanged(int val);
    void onUDLToggled(bool on);
    void onUDLIntensityChanged(int val);
    void onBeamLengthChanged(int val);

private:
    BeamSolver* m_solver;

    QComboBox* m_leftSupportCombo;
    QComboBox* m_rightSupportCombo;
    QSlider*   m_loadMagSlider;
    QLabel*    m_loadMagLabel;
    QSlider*   m_loadPosSlider;
    QLabel*    m_loadPosLabel;
    QCheckBox* m_udlCheck;
    QSlider*   m_udlIntensSlider;
    QLabel*    m_udlIntensLabel;
    QSlider*   m_beamLenSlider;
    QLabel*    m_beamLenLabel;

    // Results display
    QLabel*    m_raLabel;
    QLabel*    m_rbLabel;
    QLabel*    m_maLabel;
    QLabel*    m_maxMLabel;
    QLabel*    m_maxVLabel;

    QLabel* makeLabel(const QString& text);
    QSlider* makeSlider(int min, int max, int val, Qt::Orientation orient = Qt::Horizontal);
    void applyDarkStyle();
};
