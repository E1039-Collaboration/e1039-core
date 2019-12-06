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
  -SQMCDimuon.C
  -SQMCDimuon.h
```
## Stores truth dimuon information in a node
  ```
  -SQDimuonTruthInfoContainer.C
  -SQDimuonTruthInfoContainer.h
```
## Passes the primary truth info to Geant4 simlation

