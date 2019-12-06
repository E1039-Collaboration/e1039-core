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
Passes the primary particles' truth info to Geant4 simlation via Fun4All interface  <br />
You need read SQDimuonTruthInfoContainer node to your analysis moudle to catch the truth information of dimuon including the cross-section (weight) for each event.



