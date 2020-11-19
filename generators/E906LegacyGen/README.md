Authors: Abinash Pun, Kun Liu <br />
E906 legacy generators for comparison and fast DY simulation

## Create the Realistic primary vertex distribution
```
  -SQBeamlineObject.C
  -SQBeamlineObject.h
  -SQPrimaryVertexGen.C
  -SQPrimaryVertexGen.h
  ```

## Creates primary particles following desired physics
```
  -SQPrimaryParticleGen.C
  -SQPrimaryParticleGen.h
```
## Stores dimuon truth information in a node
  ```
  -SQMCEvent
  -SQDimuonVector
```
The individual primary particle's info is passed to Geant4 simlation via Fun4All interface  and their truth info can be read as how it is being read now (i.e. via PHG4TruthInfoContainer)  <br /> <br /> 




