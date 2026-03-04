# Beam-Playground

A real-time interactive beam mechanics visualizer built with C++ and Qt.

Beam Playground is a focused structural mechanics tool that lets you drag loads, switch support conditions, and instantly observe how reactions, shear force, and bending moment diagrams change.

It is designed for visualization, experimentation, and understanding core beam behavior — without the overhead of a full FEM system.

---

## Features

### Interactive Controls
- Drag point load along the beam
- Adjust load magnitude using sliders
- Toggle Uniformly Distributed Load (UDL)
- Switch support types:
  - Simply Supported (Pinned + Roller)
  - Propped Cantilever (Fixed + Roller)

### Live Structural Response
- Automatic reaction force recalculation
- Real-time Shear Force Diagram (SFD)
- Real-time Bending Moment Diagram (BMD)
- Maximum shear and moment detection
- Diagram zero-crossing detection

All updates occur instantly as inputs change.

---

## Solver Overview

The core logic is implemented in `BeamSolver`.

Supported structural systems:

### 1. Simply Supported Beam
Solved using static equilibrium equations.

### 2. Propped Cantilever
Solved using compatibility of deflection:
- Unit load method
- Superposition principle
- Linear elastic assumptions

Shear and moment values are sampled across the beam length for smooth and accurate diagram rendering.

---

