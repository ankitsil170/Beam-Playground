#include "BeamSolver.h"
#include <algorithm>
#include <stdexcept>

BeamSolver::BeamSolver() {}

void BeamSolver::setBeamLength(double length) {
    m_length = std::max(1.0, length);
    m_pointLoad.position = std::clamp(m_pointLoad.position, 0.0, m_length);
    m_udl.end = m_length;
}

void BeamSolver::setSupports(SupportType left, SupportType right) {
    m_supports.leftType  = left;
    m_supports.rightType = right;
}

void BeamSolver::setPointLoad(double position, double magnitude) {
    m_pointLoad.position  = std::clamp(position, 0.0, m_length);
    m_pointLoad.magnitude = magnitude;
}

void BeamSolver::setUDLActive(bool active) { m_udlActive = active; }

void BeamSolver::setUDLIntensity(double intensity) {
    m_udl.intensity = intensity;
}

BeamResults BeamSolver::solve() const {
    bool leftFixed  = (m_supports.leftType  == SupportType::Fixed);
    bool rightFixed = (m_supports.rightType == SupportType::Fixed);

    if (leftFixed && !rightFixed) {
        return solveProppedCantilever();
    }
    // Default: simply supported (Pinned + Roller)
    return solveSimplySupported();
}

BeamResults BeamSolver::solveSimplySupported() const {
    const double L = m_length;
    const int    N = 500;

    double Ra = 0.0, Rb = 0.0;

    // Point load contribution
    {
        double a = m_pointLoad.position;
        double P = m_pointLoad.magnitude;
        // Rb = P*a/L, Ra = P*(L-a)/L
        Ra += P * (L - a) / L;
        Rb += P * a       / L;
    }

    // UDL contribution
    if (m_udlActive) {
        double w    = m_udl.intensity;
        double W    = w * L; // total UDL force
        Ra += W / 2.0;
        Rb += W / 2.0;
    }

    BeamResults res;
    res.Ra = Ra;
    res.Rb = Rb;
    res.x.resize(N + 1);
    res.shear.resize(N + 1);
    res.moment.resize(N + 1);

    for (int i = 0; i <= N; ++i) {
        double x = L * i / N;
        res.x[i] = x;

        double V = Ra;
        double M = Ra * x;

        // Point load contribution to shear/moment
        if (x > m_pointLoad.position) {
            V -= m_pointLoad.magnitude;
            M -= m_pointLoad.magnitude * (x - m_pointLoad.position);
        }

        // UDL contribution
        if (m_udlActive) {
            double w = m_udl.intensity;
            V -= w * x;
            M -= w * x * x / 2.0;
        }

        res.shear[i]  = V;
        res.moment[i] = M;
    }

    // Find max values
    for (int i = 0; i <= N; ++i) {
        double absM = std::abs(res.moment[i]);
        double absV = std::abs(res.shear[i]);
        if (absM > res.maxMoment) {
            res.maxMoment    = absM;
            res.maxMomentPos = res.x[i];
        }
        if (absV > res.maxShear) res.maxShear = absV;
    }

    return res;
}

BeamResults BeamSolver::solveProppedCantilever() const {
    // Fixed left, roller/pinned right
    const double L = m_length;
    const int    N = 500;

    double Ra = 0.0, Rb = 0.0, Ma = 0.0;

    // Point load: use propped cantilever formulas
    {
        double a = m_pointLoad.position;
        double b = L - a;
        double P = m_pointLoad.magnitude;
        // Rb = P*b^2*(3L - b) / (2*L^3)  ... standard formula
        // Wait, use: Rb = P*a*(2L^2 - a^2 - a*L + ...) — use standard tables
        // Standard propped cantilever (fixed left, roller right), point load P at a from left:
        // Rb = P*a^2*(3L - a) / (2*L^3)
        // Ra = P - Rb
        // Ma = P*a*b*(L+b)/(2*L^2)  but sign convention varies
        // Using: Rb = P*(3aL^2 - a^3)/(2L^3)
        Rb += P * a * a * (3.0*L - a) / (2.0 * L*L*L);
        Ra += P - Rb;
        Ma += Ra * 0 - Rb * L;  // Will calc below properly
    }

    // Recompute properly from equilibrium
    // sum Fy: Ra + Rb = total downward load
    // sum M about A: Rb*L - Ma = sum moments of loads about A
    // For propped cantilever: Ma = moment reaction at fixed end

    // Reset and use proper method:
    Ra = 0; Rb = 0; Ma = 0;

    double totalLoad = 0.0;
    double momentAboutA = 0.0;

    {
        double P = m_pointLoad.magnitude;
        double a = m_pointLoad.position;
        totalLoad     += P;
        momentAboutA  += P * a;
    }
    if (m_udlActive) {
        double w = m_udl.intensity;
        double W = w * L;
        totalLoad    += W;
        momentAboutA += W * (L / 2.0);
    }

    // Propped cantilever: Rb (roller, right) from compatibility
    // Rb = (3 * momentAboutA) / (2 * L) for standard case — approximate
    // Exact for point load at a: Rb = P*a^2*(3L-a)/(2L^3)
    // Exact for UDL: Rb = 3wL/8
    // We'll compute Rb by superposition of compatibility conditions:
    //   delta_B due to loads = Rb * L^3 / (3EI)  =>  Rb = delta_B_loads * 3EI / L^3
    // Using unit EI, deflection formulas:

    double delta_B = 0.0; // deflection at B due to loads on cantilever (before propping)

    {   // Point load P at a from fixed end (cantilever formula)
        double P = m_pointLoad.magnitude;
        double a = m_pointLoad.position;
        double b = L - a;
        // delta at free end B = P*a^2*(3L - a) / (6EI) — cantilever, load at a
        delta_B += P * a * a * (3.0*L - a) / 6.0;
    }
    if (m_udlActive) {
        double w = m_udl.intensity;
        // delta at free end of cantilever with full UDL = wL^4 / (8EI)
        delta_B += w * L*L*L*L / 8.0;
    }

    // Rb * L^3 / 3 = delta_B  =>  Rb = 3*delta_B / L^3
    Rb = 3.0 * delta_B / (L*L*L);
    Ra = totalLoad - Rb;

    // Ma from moment equilibrium about A: Ma + Rb*L = momentAboutA  => Ma = momentAboutA - Rb*L
    // (Ma positive = hogging at left)
    Ma = momentAboutA - Rb * L;

    BeamResults res;
    res.Ra = Ra;
    res.Rb = Rb;
    res.Ma = Ma;
    res.x.resize(N + 1);
    res.shear.resize(N + 1);
    res.moment.resize(N + 1);

    for (int i = 0; i <= N; ++i) {
        double x = L * i / N;
        res.x[i] = x;

        double V = Ra;
        double M = Ra * x - Ma;  // Ma is hogging (negative moment at fixed end)

        if (x > m_pointLoad.position) {
            V -= m_pointLoad.magnitude;
            M -= m_pointLoad.magnitude * (x - m_pointLoad.position);
        }
        if (m_udlActive) {
            double w = m_udl.intensity;
            V -= w * x;
            M -= w * x * x / 2.0;
        }

        res.shear[i]  = V;
        res.moment[i] = M;
    }

    for (int i = 0; i <= N; ++i) {
        double absM = std::abs(res.moment[i]);
        double absV = std::abs(res.shear[i]);
        if (absM > res.maxMoment) {
            res.maxMoment    = absM;
            res.maxMomentPos = res.x[i];
        }
        if (absV > res.maxShear) res.maxShear = absV;
    }

    return res;
}
