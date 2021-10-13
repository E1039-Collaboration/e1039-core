# Notes on E1039 Simulation Codes

The E1039 simulation codes had been taken from (s)PHENIX, together with the Fun4All framework.
Many obsolete/unused files are left in this directory.
This note is to pick up and explain key files/classes, as a guideline for new developer.

## Detector Configuration

* The E1039 detector planes are built as `PHG4BlockSubsystem`, as defined in `macros/G4_SensitiveDetectors.C`.

## Data Nodes

Each event generator fills generated events into the "PHG4*" objects.
Or some generators (like PYTHIA8) fills into the "HepMC" objects and then `HepMCNodeReader` transfers into "PHG4*".

* `PHG4InEvent`
    * `PHG4Particle`
    * `PHG4VtxPoint`

## Interface to Geant4

The "PHG4*" objects are converted into "G4*" objects by `PHG4PrimaryGeneratorAction`.
