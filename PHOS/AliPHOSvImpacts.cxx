/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

//_________________________________________________________________________
// Implementation version vImpacts of PHOS Manager class.
// This class inherits from v1 and adds impacts storing.
// Impacts stands for exact values of track coming to the detectors
// EMC, CPV or PPSD.
// Impacts are written to the same tree as hits are
// but in separate branches.
//---
//*-- Author: Yuri Kharlov (IHEP, Protvino/SUBATECH, Nantes)


// --- ROOT system ---

#include "TTree.h"  
#include "TClonesArray.h" 

// --- Standard library ---

// --- AliRoot header files ---

#include "AliRun.h"
#include "AliPHOSvImpacts.h"
#include "AliPHOSGeometry.h"
#include "AliPHOSImpact.h"

ClassImp(AliPHOSvImpacts)

//____________________________________________________________________________
AliPHOSvImpacts::AliPHOSvImpacts():AliPHOSv1()
{
  // ctor
  fEMCImpacts  = 0 ;
  fPPSDImpacts = 0 ;
  fCPVImpacts  = 0 ; 
}

//____________________________________________________________________________
AliPHOSvImpacts::AliPHOSvImpacts(const char *name, const char *title):
AliPHOSv1(name,title) 
{
  // ctor : title is used to identify the layout
  //
  // We store hits :
  //   - fHits (the "normal" one), which retains the hits associated with
  //     the current primary particle being tracked
  //     (this array is reset after each primary has been tracked).
  //     This part inherits from AliPHOSv1
  //
  // We store impacts :
  //  - fEMCImpacts, fCPVImpacts which are
  //    TList of EMC and CPV modules respectively, each
  //    modules contains TClonesArray of AliPHOSImpacts
  
  fEMCImpacts  = new TList();
  fCPVImpacts  = new TList();

  Int_t nPHOSModules = GetGeometry()->GetNModules();
  Int_t nCPVModules  = GetGeometry()->GetNModules();

  Int_t iPHOSModule;
  TClonesArray * impacts;
  for (iPHOSModule=0; iPHOSModule<nPHOSModules; iPHOSModule++) {
    fEMCImpacts->Add(new TClonesArray("AliPHOSImpact",200)) ;
    fNEMCImpacts[iPHOSModule] = 0;
    impacts = dynamic_cast<TClonesArray *>(fEMCImpacts->At(iPHOSModule));
  }
  for (iPHOSModule=0; iPHOSModule<nCPVModules; iPHOSModule++) {
    fCPVImpacts->Add(new TClonesArray("AliPHOSImpact",200)) ;
    fNCPVImpacts[iPHOSModule] = 0;
    impacts = dynamic_cast<TClonesArray *>(fCPVImpacts->At(iPHOSModule));
  }

}

//____________________________________________________________________________
AliPHOSvImpacts::~AliPHOSvImpacts()
{
  // dtor

  // Delete hits
  if ( fHits ) {
    fHits->Delete() ; 
    delete fHits ;
    fHits = 0 ; 
  }

  // Delete impacts in EMC, CPV
  if ( fEMCImpacts ) {
    fEMCImpacts->Delete() ; 
    delete fEMCImpacts ;
    fEMCImpacts = 0 ; 
  }
  if ( fCPVImpacts ) {
    fCPVImpacts->Delete() ; 
    delete fCPVImpacts ;
    fCPVImpacts = 0 ; 
  }
}

//____________________________________________________________________________
void AliPHOSvImpacts::AddImpact( char* det, Int_t shunt, Int_t primary, Int_t track, Int_t module,
			   Int_t pid, TLorentzVector p, Float_t *xyz)
{
  // Add an impact to the impact list.

  TClonesArray * impacts = 0;
  Int_t         nImpacts = 0;

  if (strcmp(det,"EMC ")==0) {
    impacts = dynamic_cast<TClonesArray *>(fEMCImpacts->At(module));
    nImpacts= fNEMCImpacts[module];
    fNEMCImpacts[module]++ ;
  }
  else if (strcmp(det,"CPV ")==0) {
    impacts = dynamic_cast<TClonesArray *>(fCPVImpacts->At(module));
    nImpacts= fNCPVImpacts[module];
    fNCPVImpacts[module]++ ;
  }

  new((*impacts)[nImpacts]) AliPHOSImpact(shunt,primary,track,pid,p,xyz) ;

  if (fDebug==1) {
    printf("Module %d %s: ",module,det);
    (dynamic_cast<AliPHOSImpact*>((impacts->At(nImpacts))))->Print();
  }
}

//____________________________________________________________________________
void AliPHOSvImpacts::MakeBranch(Option_t *opt, const char *file)
{  
  // Create new branch in the current Hits Root Tree containing
  // a list of PHOS impacts (exact values of track coming to detector)

  AliDetector::MakeBranch(opt,file);
  
  Int_t bufferSize = 32000 ;
  Int_t splitlevel = 0 ;
  gAlice->TreeH()->Branch("PHOSEmcImpacts" , "TList", &fEMCImpacts , bufferSize, splitlevel);
  gAlice->TreeH()->Branch("PHOSCpvImpacts" , "TList", &fCPVImpacts , bufferSize, splitlevel);
  
}

//____________________________________________________________________________
void AliPHOSvImpacts::ResetHits() 
{              
  // Reset impact branches for EMC, CPV and PPSD

  AliDetector::ResetHits();

  Int_t i;
  for (i=0; i<GetGeometry()->GetNModules(); i++) {
    (dynamic_cast<TClonesArray*>(fEMCImpacts->At(i))) -> Clear();
    fNEMCImpacts[i] = 0 ;
  }

  for (i=0; i<GetGeometry()->GetNModules(); i++) {
    (dynamic_cast<TClonesArray*>(fCPVImpacts->At(i))) -> Clear();
    fNCPVImpacts[i] = 0 ;
  }
  
}

//_____________________________________________________________________________
void AliPHOSvImpacts::StepManager(void)
{
  // Find impacts (tracks which enter the EMC, CPV)
  // and add them to to the impact lists

  AliPHOSv1::StepManager();

  Float_t xyzm[3], xyzd[3], pm[3], pd[3];
  TLorentzVector pmom     ;           // Lorentz momentum of the particle initiated hit
  TLorentzVector pos      ;           // Lorentz vector of the track current position
  Int_t          copy     ;

  Int_t tracknumber =  gAlice->CurrentTrack() ; 
  Int_t primary     =  gAlice->GetPrimary( gAlice->CurrentTrack() ); 
  TString name      =  GetGeometry()->GetName() ; 

  // Add impact to EMC

  if( gMC->CurrentVolID(copy) == gMC->VolId("PXTL") &&
      gMC->IsTrackEntering() ) {
    gMC->TrackMomentum(pmom);
    gMC->TrackPosition(pos) ;

    Int_t i;
    for (i=0; i<3; i++) xyzm[i] = pos[i];

    for (i=0; i<3; i++) {
      xyzm[i] = pos[i] ;
      pm[i]   = pmom[i];
    }
    gMC -> Gmtod (xyzm, xyzd, 1);    // transform coordinate from master to daughter system
    gMC -> Gmtod (pm,   pd,   2);    // transform 3-momentum from master to daughter system

    // Select tracks coming to the crystal from up or down sides
    if (pd[1]<0 && xyzd[1] >  GetGeometry()->GetCrystalSize(1)/2-0.1 ||
	pd[1]>0 && xyzd[1] < -GetGeometry()->GetCrystalSize(1)/2+0.1) {
    // Select tracks coming to the crystal from up or down sides
      Int_t pid = gMC->TrackPid();
      Int_t module;
      gMC->CurrentVolOffID(10,module);
      module--;
      AddImpact("EMC ",fIshunt, primary,tracknumber, module, pid, pmom, xyzm);
    }
  }

  // Add impact to CPV

  if( gMC->CurrentVolID(copy) == gMC->VolId("PCPQ") &&
      gMC->IsTrackEntering() ) {
    gMC->TrackMomentum(pmom);
    gMC->TrackPosition(pos) ;

    Int_t i;
    for (i=0; i<3; i++) xyzm[i] = pos[i];

    for (i=0; i<3; i++) {
      xyzm[i] = pos[i] ;
      pm[i]   = pmom[i];
    }
    Int_t pid = gMC->TrackPid();
    Int_t module;
    gMC->CurrentVolOffID(3,module);
    module--;
    AddImpact("CPV ",fIshunt, primary,tracknumber, module, pid, pmom, xyzm);
  }
  
}
