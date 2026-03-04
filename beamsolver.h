#pragma once
#include <vector>
#include <cmath>

enum class SupportType {
    Pinned,
    Roller,
    Fixed
};

enum class LoadType {
    PointLoad,
    UDL
};

struct PointLoad {
    double position;   // x position along beam [0, beamLength]
    double magnitude;  // kN (positive = downward)
};

struct UDL {
    double start;      // x start position
    double end;        // x end position
    double intensity;  // kN/m (positive = downward)
};

struct SupportCondition {
    SupportType leftType  = SupportType::Pinned;
    SupportType rightType = SupportType::Roller;
};

struct BeamResults {
    double Ra = 0.0;           // Reaction at left support (kN)
    double Rb = 0.0;           // Reaction at right support (kN)
    double Ma = 0.0;           // Fixed-end moment at left (kNm) — only if Fixed
    double maxMoment = 0.0;    // Max absolute bending moment (kNm)
    double maxShear  = 0.0;    // Max absolute shear force (kN)
    double maxMomentPos = 0.0; // Position of max moment (m)

    std::vector<double> x;     // Sample positions
    std::vector<double> shear; // Shear force at each x
    std::vector<double> moment;// Bending moment at each x
};

class BeamSolver {
public:
    BeamSolver();

    void setBeamLength(double length);
    void setSupports(SupportType left, SupportType right);
    void setPointLoad(double position, double magnitude);
    void setUDLActive(bool active);
    void setUDLIntensity(double intensity);

    BeamResults solve() const;

    double beamLength()    const { return m_length; }
    double pointLoadPos()  const { return m_pointLoad.position; }
    double pointLoadMag()  const { return m_pointLoad.magnitude; }
    bool   udlActive()     const { return m_udlActive; }
    double udlIntensity()  const { return m_udl.intensity; }
    SupportType leftSupport()  const { return m_supports.leftType; }
    SupportType rightSupport() const { return m_supports.rightType; }

private:
    double          m_length   = 6.0;
    PointLoad       m_pointLoad{ 3.0, 10.0 };
    UDL             m_udl{ 0.0, 6.0, 5.0 };
    bool            m_udlActive = false;
    SupportCondition m_supports;

    // Simply-supported solver
    BeamResults solveSimplySupported() const;
    // Fixed-roller solver (propped cantilever)
    BeamResults solveProppedCantilever() const;
};
