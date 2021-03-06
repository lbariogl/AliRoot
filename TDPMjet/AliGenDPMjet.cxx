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


// Generator using DPMJET as an external generator
// The main DPMJET options are accessable for the user through this interface.
// Uses the TDPMjet implementation of TGenerator.

#include <TDPMjet.h>
#include "DPMcommon.h"
#include <TRandom.h>
#include <TArrayI.h>
#include <TParticle.h>
#include <TGraph.h>
#include <TDatabasePDG.h>
#include <TParticlePDG.h>
#include <TParticleClassPDG.h>
#include <TPDGCode.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>
#include "AliRunLoader.h"
#include "AliGenDPMjet.h"
#include "AliGenDPMjetEventHeader.h"
#include "AliRun.h"
#include "AliDpmJetRndm.h"
#include "AliIonPDGCodes.h"
#include "AliHeader.h"
#include "AliStack.h"
#include "AliMC.h"
#include "AliLog.h"

ClassImp(AliGenDPMjet)

//______________________________________________________________________________
AliGenDPMjet::AliGenDPMjet()
    :AliGenMC(), 
     fBeamEn(0.),
     fMinImpactParam(0.),
     fMaxImpactParam(100.),
     fICentr(0),
     fSelectAll(0),
     fFlavor(0),
     fTrials(0),
     fNprimaries(0),
     fSpectators(1),
     fSpecn(0),
     fSpecp(0),
     fDPMjet(0),
     fNoGammas(0),
     fLHC(0),
     fPi0Decay(1),
     fDecayAll(0),
     fGenImpPar(0.),
     fProcess(kDpmMb),
     fTriggerParticle(0),
     fTriggerEta(0.9),     
     fTriggerMinPt(-1),  
     fTriggerMaxPt(1000),  
     fTriggerMultiplicity(0),
     fTriggerMultiplicityEta(0),
     fTriggerMultiplicityPtMin(0),
     fkTuneForDiff(0),
     fProcDiff(0),
     fFragmentation(kFALSE),
     fHeader(AliGenDPMjetEventHeader("DPMJET"))

{
// Constructor
    fEnergyCMS = 5500.;
    AliDpmJetRndm::SetDpmJetRandom(GetRandom());
}


//______________________________________________________________________________
AliGenDPMjet::AliGenDPMjet(Int_t npart)
    :AliGenMC(npart),
     fBeamEn(0.),
     fMinImpactParam(0.),
     fMaxImpactParam(100.),
     fICentr(0),
     fSelectAll(0),
     fFlavor(0),
     fTrials(0),
     fNprimaries(0),
     fSpectators(1),
     fSpecn(0),
     fSpecp(0),
     fDPMjet(0),
     fNoGammas(0),
     fLHC(0),
     fPi0Decay(1),
     fDecayAll(0),
     fGenImpPar(0.),
     fProcess(kDpmMb),
     fTriggerParticle(0),
     fTriggerEta(0.9),     
     fTriggerMinPt(-1),  
     fTriggerMaxPt(1000),  
     fTriggerMultiplicity(0),
     fTriggerMultiplicityEta(0),
     fTriggerMultiplicityPtMin(0),
     fkTuneForDiff(0),
     fProcDiff(0),
     fFragmentation(kFALSE),
     fHeader(AliGenDPMjetEventHeader("DPMJET"))


{
// Default PbPb collisions at 5. 5 TeV
//
    fEnergyCMS = 5500.;
    fName = "DPMJET";
    fTitle= "Particle Generator using DPMJET";
    SetTarget();
    SetProjectile();
    fVertex.Set(3);
    AliDpmJetRndm::SetDpmJetRandom(GetRandom());
}

AliGenDPMjet::AliGenDPMjet(const AliGenDPMjet &/*Dpmjet*/)
    :AliGenMC(),
     fBeamEn(0.),
     fMinImpactParam(0.),
     fMaxImpactParam(100.),
     fICentr(0),
     fSelectAll(0),
     fFlavor(0),
     fTrials(0),
     fNprimaries(0),
     fSpectators(1),
     fSpecn(0),
     fSpecp(0),
     fDPMjet(0),
     fNoGammas(0),
     fLHC(0),
     fPi0Decay(1),
     fDecayAll(0),
     fGenImpPar(0.),
     fProcess(kDpmMb),
     fTriggerParticle(0),
     fTriggerEta(0.9),     
     fTriggerMinPt(-1),  
     fTriggerMaxPt(1000),  
     fTriggerMultiplicity(0),
     fTriggerMultiplicityEta(0),
     fTriggerMultiplicityPtMin(0),
     fkTuneForDiff(0),
     fProcDiff(0),
     fFragmentation(kFALSE),
     fHeader(0x0)


{
    // Dummy copy constructor
    fEnergyCMS = 5500.;
}

//______________________________________________________________________________
AliGenDPMjet::~AliGenDPMjet()
{
// Destructor
}
//______________________________________________________________________________
void AliGenDPMjet::Init()
{
// Initialization
    
    if(fEnergyCMS>0. && fBeamEn<0.1) fBeamEn = fEnergyCMS/2;
    SetMC(new TDPMjet(fProcess, fAProjectile, fZProjectile, fATarget, fZTarget, 
		      fBeamEn,fEnergyCMS));

    fDPMjet=(TDPMjet*) fMCEvGen;
    if (fAProjectile == 1 && TMath::Abs(fZProjectile == 1)) fDPMjet->SetfIdp(1);
    
    // **** Flag to force central production
    // fICentr=1. central production forced 
    // fICentr<0 && fICentr>-100 -> bmin = fMinImpactParam, bmax = fMaxImpactParam	  
    // fICentr<-99 -> fraction of x-sec. = XSFRAC		  
    // fICentr=-1. -> evaporation/fzc suppressed		  
    // fICentr<-1. -> evaporation/fzc allowed		      
    fDPMjet->SetfFCentr(fICentr);  
    
    fDPMjet->SetbRange(fMinImpactParam, fMaxImpactParam); 
    fDPMjet->SetPi0Decay(fPi0Decay);
    fDPMjet->SetDecayAll(fDecayAll);
    fDPMjet->SetFragmentProd(fFragmentation);
    
//
//  Initialize DPMjet  
//    
    fDPMjet->Initialize();
}


//______________________________________________________________________________
void AliGenDPMjet::Generate()
{
// Generate one event

  Double_t polar[3]    =   {0,0,0};
  Double_t origin[3]   =   {0,0,0};
  Double_t p[4]        =   {0};
  Float_t tof;

//  converts from mm/c to s
  const Float_t kconv = 0.001/2.999792458e8;
  Int_t nt  = 0;
  Int_t jev = 0;
  Int_t kf, ks, imo;
  kf = 0;
  fTrials = 0;
  fNprimaries = 0;
  //  Set collision vertex position 
  if (fVertexSmear == kPerEvent) Vertex();
  
  while(1)
  {
//    Generate one event
// --------------------------------------------------------------------------
      fSpecn = 0;  
      fSpecp = 0;
// --------------------------------------------------------------------------
      fDPMjet->GenerateEvent();
      
      fTrials++;

      fDPMjet->ImportParticles(&fParticles,"All");      
      if (fLHC) Boost();

      // Temporaneo
      fGenImpPar = fDPMjet->GetBImpac();

      if(TMath::Abs(fXingAngleY) > 1.e-10) BeamCrossAngle();

      Int_t np = fParticles.GetEntriesFast();
      //
      // Multiplicity Trigger
      if (fTriggerMultiplicity > 0) {
	Int_t multiplicity = 0;
	for (Int_t i = 0; i < np; i++) {
	  TParticle *  iparticle = (TParticle *) fParticles.At(i);
	
	  Int_t statusCode = iparticle->GetStatusCode();
	
	  // Initial state particle
	  if (statusCode != 1)
	    continue;
	  // eta cut
	  if (fTriggerMultiplicityEta > 0 && TMath::Abs(iparticle->Eta()) > fTriggerMultiplicityEta)
	    continue;
	  // pt cut
	  if (iparticle->Pt() < fTriggerMultiplicityPtMin) 
	    continue;
	  
	  TParticlePDG* pdgPart = iparticle->GetPDG();
	  if (pdgPart && pdgPart->Charge() == 0)
	    continue;
	  ++multiplicity;
	}
	//
	//
	if (multiplicity < fTriggerMultiplicity) continue;
	Printf("Triggered on event with multiplicity of %d >= %d", multiplicity, fTriggerMultiplicity);
      }    

  //Trigger on the presence of a given particle in some phase space
    if (fTriggerParticle) {
	Bool_t triggered = kFALSE;
	    for (Long_t i = 0; i < np; i++) {
	        TParticle *  iparticle = (TParticle *) fParticles.At(i);
	        kf = CheckPDGCode(iparticle->GetPdgCode());
	        if (kf != fTriggerParticle) continue;
	        if (iparticle->Pt() == 0.) continue;
	        if (TMath::Abs(iparticle->Eta()) > fTriggerEta) continue;
	        if ( iparticle->Pt() > fTriggerMaxPt || iparticle->Pt() < fTriggerMinPt ) continue;
	        triggered = kTRUE;
	        break;
	    }
      if (!triggered) continue; 
    }

      if(fkTuneForDiff && ( (TMath::Abs(fEnergyCMS - 900) < 1) || (TMath::Abs(fEnergyCMS - 2760) < 1) || (TMath::Abs(fEnergyCMS - 7000) < 1)) ) {
	if(!CheckDiffraction() ) continue;
      }


      Int_t nc = 0;
      if (np == 0) continue;

      Int_t i;
      Int_t* newPos     = new Int_t[np];
      Int_t* pSelected  = new Int_t[np];

      for (i = 0; i<np; i++) {
	  newPos[i]    = i;
	  pSelected[i] = 0;
      }
      
//      First select parent particles

      for (i = 0; i<np; i++) {
	  TParticle *iparticle = (TParticle *) fParticles.At(i);

// Is this a parent particle ?

	  if (Stable(iparticle)) continue;

	  Bool_t  selected             =  kTRUE;
	  Bool_t  hasSelectedDaughters =  kFALSE;
	  
	  kf = iparticle->GetPdgCode();
	  if (kf == 92 || kf == 99999) continue;
	  ks = iparticle->GetStatusCode();
// No initial state partons
          if (ks==21) continue;
	  if (!fSelectAll) selected = KinematicSelection(iparticle, 0) && 
			       SelectFlavor(kf);

	  
	  hasSelectedDaughters = DaughtersSelection(iparticle);


// Put particle on the stack if it is either selected or 
// it is the mother of at least one seleted particle

	  if (selected || hasSelectedDaughters) {
	      nc++;
	      pSelected[i] = 1;
	  } // selected
      } // particle loop parents

// Now select the final state particles


      for (i=0; i<np; i++) {
	  TParticle *iparticle = (TParticle *) fParticles.At(i);

// Is this a final state particle ?

	  if (!Stable(iparticle)) continue;
      
	  Bool_t  selected =  kTRUE;
	  kf = iparticle->GetPdgCode();
	  ks = iparticle->GetStatusCode();

// --------------------------------------------------------------------------
// Count spectator neutrons and protons (ks == 13, 14)
	  if(ks == 13 || ks == 14){
	      if(kf == kNeutron) fSpecn += 1;
	      if(kf == kProton)  fSpecp += 1;
	  }
// --------------------------------------------------------------------------

	  if (!fSelectAll) {
	      selected = KinematicSelection(iparticle,0)&&SelectFlavor(kf);
	      if (!fSpectators && selected) selected = (ks == 13 || ks == 14);
	  }

// Put particle on the stack if selected

	  if (selected) {
	      nc++;
	      pSelected[i] = 1;
	  } // selected
      } // particle loop final state

// Write particles to stack

      for (i = 0; i<np; i++) {
	  TParticle *  iparticle = (TParticle *) fParticles.At(i);
	  Bool_t  hasMother   = (iparticle->GetFirstMother()>=0);
	  if (pSelected[i]) {
	      
	      kf   = iparticle->GetPdgCode();	      
	      ks   = iparticle->GetStatusCode();	      
	      
	      p[0] = iparticle->Px();
	      p[1] = iparticle->Py();
	      p[2] = iparticle->Pz();
	      p[3] = iparticle->Energy();
	      origin[0] = fVertex[0]+iparticle->Vx()/10; // [cm]
	      origin[1] = fVertex[1]+iparticle->Vy()/10; // [cm]
	      origin[2] = fVertex[2]+iparticle->Vz()/10; // [cm]
		    
	      tof = fTime + kconv*iparticle->T();
	      
	      imo = -1;
	      TParticle* mother = 0;
	      if (hasMother) {
		  imo = iparticle->GetFirstMother();
		  mother = (TParticle *) fParticles.At(imo);
		  imo = (mother->GetPdgCode() != 92 && mother->GetPdgCode() != 99999) ? newPos[imo] : -1;
	      } // if has mother   


	      
	      Bool_t tFlag = (fTrackIt && (ks==1 || ks==-1));
	      //printf(" AliGemDPMJet->PushTrack: kf %d  ks %d  flag %d\n",kf,ks,tFlag);
	      if(kf>10000 && (ks==-1 || ks==1000 || ks==1001)) kf += 1000000000;
	      PushTrack(tFlag, imo, kf, 
			p[0], p[1], p[2], p[3], 
			origin[0], origin[1], origin[2], tof,
			polar[0], polar[1], polar[2],
			kPNoProcess, nt, 1., ks);
	      KeepTrack(nt);
	      fNprimaries++;
	      newPos[i] = nt;
	  } // if selected
      } // particle loop
      delete[] newPos;
      delete[] pSelected;
      if (nc>0) {
	  jev += nc;
	  if (jev >= fNpart || fNpart == -1) {
	      break;
	  }
      }
  } // event loop
  MakeHeader();
  SetHighWaterMark(nt);
}

//______________________________________________________________________________
Bool_t AliGenDPMjet::DaughtersSelection(TParticle* iparticle)
{
//
// Looks recursively if one of the daughters has been selected
//
//    printf("\n Consider daughters %d:",iparticle->GetPdgCode());
    Int_t imin = -1;
    Int_t imax = -1;
    Int_t i;
    Bool_t hasDaughters = (iparticle->GetFirstDaughter() >=0);
    Bool_t selected = kFALSE;
    if (hasDaughters) {
	imin = iparticle->GetFirstDaughter();
	imax = iparticle->GetLastDaughter();       
	for (i = imin; i <= imax; i++){
	    TParticle *  jparticle = (TParticle *) fParticles.At(i);	
	    Int_t ip = jparticle->GetPdgCode();
	    if (KinematicSelection(jparticle,0)&&SelectFlavor(ip)) {
		selected=kTRUE; break;
	    }
	    if (DaughtersSelection(jparticle)) {selected=kTRUE; break; }
	}
    } else {
	return kFALSE;
    }
    return selected;
}



//______________________________________________________________________________
Bool_t AliGenDPMjet::SelectFlavor(Int_t pid)
{
// Select flavor of particle
// 0: all
// 4: charm and beauty
// 5: beauty
    Bool_t res = 0;
    
    if (fFlavor == 0) {
	res = kTRUE;
    } else {
	Int_t ifl = TMath::Abs(pid/100);
	if (ifl > 10) ifl/=10;
	res = (fFlavor == ifl);
    }
//
//  This part if gamma writing is inhibited
    if (fNoGammas) 
	res = res && (pid != kGamma && pid != kPi0);
//
    return res;
}

//______________________________________________________________________________
Bool_t AliGenDPMjet::Stable(TParticle*  particle)
{
// Return true for a stable particle
//
    int st = particle->GetStatusCode();
    if(st == 1 || st == -1) return kTRUE;
    else return kFALSE;

}

//______________________________________________________________________________
void AliGenDPMjet::MakeHeader()
{
// Builds the event header, to be called after each event
    fHeader.SetNProduced(fNprimaries);
    fHeader.SetImpactParameter(fDPMjet->GetBImpac());
    fHeader.SetTotalEnergy(fDPMjet->GetTotEnergy());
    fHeader.SetParticipants(fDPMjet->GetProjParticipants(), 
    		            fDPMjet->GetTargParticipants());

    fHeader.SetCollisions(DTGLCP.ncp, DTGLCP.nct, 
    	fDPMjet->GetProjWounded(),fDPMjet->GetTargWounded());
		
    if(fProcDiff>0)  fHeader.SetProcessType(fProcDiff);
    else fHeader.SetProcessType(fDPMjet->GetProcessCode());

    // Bookkeeping for kinematic bias
    fHeader.SetTrials(fTrials);
    // Event Vertex
    fHeader.SetPrimaryVertex(fVertex);
    fHeader.SetInteractionTime(fTime);
    fHeader.SetNDiffractive(POEVT1.nsd1, POEVT1.nsd2, POEVT1.ndd);
//    gAlice->SetGenEventHeader(fHeader);    
    AddHeader(&fHeader);
    fCollisionGeometry = &fHeader;
}

//______________________________________________________________________________
/*void AliGenDPMjet::AddHeader(AliGenEventHeader* fHeader)
{
    // Add fHeader to container or runloader
    if (fContainer) {
        fContainer->AddHeader(fHeader);
    } else {
        AliRunLoader::Instance()->GetHeader()->SetGenEventHeader(fHeader);
    }
}*/


//______________________________________________________________________________
AliGenDPMjet& AliGenDPMjet::operator=(const  AliGenDPMjet& /*rhs*/)
{
// Assignment operator
    return *this;
}


//______________________________________________________________________________
void AliGenDPMjet::FinishRun()
{
    // Print run statistics
    fDPMjet->Dt_Dtuout();
}


//______________________________________________________________________________
Bool_t AliGenDPMjet::CheckDiffraction()
{

  //  printf("AAA\n");

   Int_t np = fParticles.GetEntriesFast();

   Int_t iPart1=-1;
   Int_t iPart2=-1;

   Double_t y1 = 1e10;
   Double_t y2 = -1e10;

  const Int_t kNstable=20;
  const Int_t pdgStable[20] = {
    22,             // Photon
    11,             // Electron
    12,             // Electron Neutrino 
    13,             // Muon 
    14,             // Muon Neutrino
    15,             // Tau 
    16,             // Tau Neutrino
    211,            // Pion
    321,            // Kaon
    311,            // K0
    130,            // K0s
    310,            // K0l
    2212,           // Proton 
    2112,           // Neutron
    3122,           // Lambda_0
    3112,           // Sigma Minus
    3222,           // Sigma Plus
    3312,           // Xsi Minus 
    3322,           // Xsi0
    3334            // Omega
  };
    
     for (Int_t i = 0; i < np; i++) {
	TParticle *  part = (TParticle *) fParticles.At(i);
	
	Int_t statusCode = part->GetStatusCode();
	
	// Initial state particle
	if (statusCode != 1)
	  continue;

	Int_t pdg = TMath::Abs(part->GetPdgCode());
	Bool_t isStable = kFALSE;
	for (Int_t i1 = 0; i1 < kNstable; i1++) {
	  if (pdg == pdgStable[i1]) {
	    isStable = kTRUE;
	    break;
	  }
	}
	if(!isStable) 
	  continue;

	Double_t y = part->Y();

	if (y < y1)
	  {
	    y1 = y;
	    iPart1 = i;
	  }
	if (y > y2)
	{
	  y2 = y;
	  iPart2 = i;
	}
     }

     if(iPart1<0 || iPart2<0) return kFALSE;

     y1=TMath::Abs(y1);
     y2=TMath::Abs(y2);

     TParticle *  part1 = (TParticle *) fParticles.At(iPart1);
     TParticle *  part2 = (TParticle *) fParticles.At(iPart2);

     Int_t pdg1 = part1->GetPdgCode();
     Int_t pdg2 = part2->GetPdgCode();


     Int_t iPart = -1;
     if (pdg1 == 2212 && pdg2 == 2212)
       {
	 if(y1 > y2) 
	   iPart = iPart1;
	 else if(y1 < y2) 
	   iPart = iPart2;
	 else {
	   iPart = iPart1;
	   if((AliDpmJetRndm::GetDpmJetRandom())->Uniform(0.,1.)>0.5) iPart = iPart2;
	 }
       }
     else if (pdg1 == 2212)
       iPart = iPart1;
     else if (pdg2 == 2212)
       iPart = iPart2;





     Double_t M=-1.;
     if(iPart>0) {
       TParticle *  part = (TParticle *) fParticles.At(iPart);
       Double_t E= part->Energy();
       Double_t P= part->P();
       Double_t M2 = (fEnergyCMS-E-P)*(fEnergyCMS-E+P);
       if(M2<0)  return kFALSE;
       M= TMath::Sqrt(M2);
     }

     Double_t Mmin, Mmax, wSD, wDD, wND;

     if(!GetWeightsDiffraction(M, Mmin, Mmax, wSD, wDD, wND)) return kFALSE;
     if(M>-1 && M<Mmin) return kFALSE;
     if(M>Mmax) M=-1;


 Int_t procType=fDPMjet->GetProcessCode();//fPythia->GetMSTI(1);
 if(procType== 4) return kFALSE;
 Int_t proc0=2;
 if(procType== 7) proc0=1;
 if(procType== 5 || procType== 6) proc0=0;


 Int_t proc=2;
 if(M>0) proc=0;
 else if(proc0==1) proc=1;

 if(proc==1 && (AliDpmJetRndm::GetDpmJetRandom())->Uniform(0.,1.) > wDD) return kFALSE;
 if(proc==2 && (AliDpmJetRndm::GetDpmJetRandom())->Uniform(0.,1.) > wND) return kFALSE;



    if(proc!=0) {
      if(proc0!=0) fProcDiff = procType;
      else       fProcDiff = 1;
      return kTRUE;
    }

    if(wSD<0)  AliError("wSD<0 ! \n");

    if((AliDpmJetRndm::GetDpmJetRandom())->Uniform(0.,1.)> wSD) return kFALSE;

    //    printf("iPart = %d\n", iPart);

    if(iPart==iPart1) fProcDiff=5;
    else if(iPart==iPart2) fProcDiff=6;
    else {
      printf("EROOR:  iPart!=iPart1 && iPart!=iPart2\n");

    }

    return kTRUE;
}

// -------------------------------------------------------
void AliGenDPMjet::SetIonPDGCodes()
{
   // Defining PDG codes for the ions
   AliIonPDGCodes *pdgcodes = new AliIonPDGCodes();
   pdgcodes->AddParticlesToPdgDataBase();
}

// -------------------------------------------------------
Bool_t AliGenDPMjet::GetWeightsDiffraction(Double_t M, Double_t &Mmin, Double_t &Mmax, 
                                                       Double_t &wSD, Double_t &wDD, Double_t &wND)
{

  if(TMath::Abs(fEnergyCMS-900)<1 ){
const Int_t nbin=400;
Double_t bin[]={
1.080000, 1.577300, 2.074600, 2.571900, 3.069200, 3.566500, 
4.063800, 4.561100, 5.058400, 5.555700, 6.053000, 6.550300, 
7.047600, 7.544900, 8.042200, 8.539500, 9.036800, 9.534100, 
10.031400, 10.528700, 11.026000, 11.523300, 12.020600, 12.517900, 
13.015200, 13.512500, 14.009800, 14.507100, 15.004400, 15.501700, 
15.999000, 16.496300, 16.993600, 17.490900, 17.988200, 18.485500, 
18.982800, 19.480100, 19.977400, 20.474700, 20.972000, 21.469300, 
21.966600, 22.463900, 22.961200, 23.458500, 23.955800, 24.453100, 
24.950400, 25.447700, 25.945000, 26.442300, 26.939600, 27.436900, 
27.934200, 28.431500, 28.928800, 29.426100, 29.923400, 30.420700, 
30.918000, 31.415300, 31.912600, 32.409900, 32.907200, 33.404500, 
33.901800, 34.399100, 34.896400, 35.393700, 35.891000, 36.388300, 
36.885600, 37.382900, 37.880200, 38.377500, 38.874800, 39.372100, 
39.869400, 40.366700, 40.864000, 41.361300, 41.858600, 42.355900, 
42.853200, 43.350500, 43.847800, 44.345100, 44.842400, 45.339700, 
45.837000, 46.334300, 46.831600, 47.328900, 47.826200, 48.323500, 
48.820800, 49.318100, 49.815400, 50.312700, 50.810000, 51.307300, 
51.804600, 52.301900, 52.799200, 53.296500, 53.793800, 54.291100, 
54.788400, 55.285700, 55.783000, 56.280300, 56.777600, 57.274900, 
57.772200, 58.269500, 58.766800, 59.264100, 59.761400, 60.258700, 
60.756000, 61.253300, 61.750600, 62.247900, 62.745200, 63.242500, 
63.739800, 64.237100, 64.734400, 65.231700, 65.729000, 66.226300, 
66.723600, 67.220900, 67.718200, 68.215500, 68.712800, 69.210100, 
69.707400, 70.204700, 70.702000, 71.199300, 71.696600, 72.193900, 
72.691200, 73.188500, 73.685800, 74.183100, 74.680400, 75.177700, 
75.675000, 76.172300, 76.669600, 77.166900, 77.664200, 78.161500, 
78.658800, 79.156100, 79.653400, 80.150700, 80.648000, 81.145300, 
81.642600, 82.139900, 82.637200, 83.134500, 83.631800, 84.129100, 
84.626400, 85.123700, 85.621000, 86.118300, 86.615600, 87.112900, 
87.610200, 88.107500, 88.604800, 89.102100, 89.599400, 90.096700, 
90.594000, 91.091300, 91.588600, 92.085900, 92.583200, 93.080500, 
93.577800, 94.075100, 94.572400, 95.069700, 95.567000, 96.064300, 
96.561600, 97.058900, 97.556200, 98.053500, 98.550800, 99.048100, 
99.545400, 100.042700, 100.540000, 101.037300, 101.534600, 102.031900, 
102.529200, 103.026500, 103.523800, 104.021100, 104.518400, 105.015700, 
105.513000, 106.010300, 106.507600, 107.004900, 107.502200, 107.999500, 
108.496800, 108.994100, 109.491400, 109.988700, 110.486000, 110.983300, 
111.480600, 111.977900, 112.475200, 112.972500, 113.469800, 113.967100, 
114.464400, 114.961700, 115.459000, 115.956300, 116.453600, 116.950900, 
117.448200, 117.945500, 118.442800, 118.940100, 119.437400, 119.934700, 
120.432000, 120.929300, 121.426600, 121.923900, 122.421200, 122.918500, 
123.415800, 123.913100, 124.410400, 124.907700, 125.405000, 125.902300, 
126.399600, 126.896900, 127.394200, 127.891500, 128.388800, 128.886100, 
129.383400, 129.880700, 130.378000, 130.875300, 131.372600, 131.869900, 
132.367200, 132.864500, 133.361800, 133.859100, 134.356400, 134.853700, 
135.351000, 135.848300, 136.345600, 136.842900, 137.340200, 137.837500, 
138.334800, 138.832100, 139.329400, 139.826700, 140.324000, 140.821300, 
141.318600, 141.815900, 142.313200, 142.810500, 143.307800, 143.805100, 
144.302400, 144.799700, 145.297000, 145.794300, 146.291600, 146.788900, 
147.286200, 147.783500, 148.280800, 148.778100, 149.275400, 149.772700, 
150.270000, 150.767300, 151.264600, 151.761900, 152.259200, 152.756500, 
153.253800, 153.751100, 154.248400, 154.745700, 155.243000, 155.740300, 
156.237600, 156.734900, 157.232200, 157.729500, 158.226800, 158.724100, 
159.221400, 159.718700, 160.216000, 160.713300, 161.210600, 161.707900, 
162.205200, 162.702500, 163.199800, 163.697100, 164.194400, 164.691700, 
165.189000, 165.686300, 166.183600, 166.680900, 167.178200, 167.675500, 
168.172800, 168.670100, 169.167400, 169.664700, 170.162000, 170.659300, 
171.156600, 171.653900, 172.151200, 172.648500, 173.145800, 173.643100, 
174.140400, 174.637700, 175.135000, 175.632300, 176.129600, 176.626900, 
177.124200, 177.621500, 178.118800, 178.616100, 179.113400, 179.610700, 
180.108000, 180.605300, 181.102600, 181.599900, 182.097200, 182.594500, 
183.091800, 183.589100, 184.086400, 184.583700, 185.081000, 185.578300, 
186.075600, 186.572900, 187.070200, 187.567500, 188.064800, 188.562100, 
189.059400, 189.556700, 190.054000, 190.551300, 191.048600, 191.545900, 
192.043200, 192.540500, 193.037800, 193.535100, 194.032400, 194.529700, 
195.027000, 195.524300, 196.021600, 196.518900, 197.016200, 197.513500, 
198.010800, 198.508100, 199.005400, 199.502700, 200.000000};
Double_t w[]={
1.000000, 0.389079, 0.326612, 0.356363, 0.310726, 0.264037, 
0.253810, 0.224655, 0.207990, 0.198149, 0.186803, 0.180209, 
0.178928, 0.161772, 0.162314, 0.158409, 0.148379, 0.143557, 
0.140357, 0.136272, 0.136957, 0.136606, 0.129376, 0.127671, 
0.128406, 0.132239, 0.119311, 0.130227, 0.130233, 0.123312, 
0.115257, 0.120105, 0.119733, 0.117116, 0.110096, 0.117509, 
0.109149, 0.114372, 0.100322, 0.106227, 0.108696, 0.110352, 
0.106748, 0.101475, 0.101837, 0.098585, 0.094433, 0.100148, 
0.096505, 0.100155, 0.103526, 0.098161, 0.093647, 0.100121, 
0.097426, 0.093414, 0.090241, 0.097228, 0.098098, 0.098340, 
0.096779, 0.099265, 0.095101, 0.090342, 0.097469, 0.090663, 
0.093422, 0.093103, 0.089648, 0.096923, 0.088079, 0.089360, 
0.097772, 0.092932, 0.093554, 0.085784, 0.096395, 0.096304, 
0.090183, 0.089255, 0.090265, 0.086262, 0.087044, 0.088965, 
0.085825, 0.095073, 0.085933, 0.091005, 0.091707, 0.092428, 
0.091689, 0.091224, 0.095256, 0.083407, 0.087983, 0.090320, 
0.081580, 0.088077, 0.083478, 0.091309, 0.083734, 0.089906, 
0.100155, 0.092728, 0.086542, 0.078091, 0.085261, 0.094302, 
0.078063, 0.090070, 0.086566, 0.095020, 0.082124, 0.084791, 
0.090624, 0.090236, 0.086344, 0.085706, 0.085913, 0.083107, 
0.092557, 0.081144, 0.091254, 0.096139, 0.079458, 0.088124, 
0.088777, 0.076652, 0.085168, 0.082326, 0.080435, 0.085022, 
0.085693, 0.093957, 0.088134, 0.082347, 0.082139, 0.090980, 
0.078397, 0.093595, 0.091562, 0.081789, 0.085026, 0.080868, 
0.083455, 0.084590, 0.084124, 0.081486, 0.087242, 0.080928, 
0.096392, 0.078324, 0.092093, 0.074267, 0.082925, 0.090717, 
0.081467, 0.080435, 0.078825, 0.075843, 0.088940, 0.081451, 
0.096047, 0.088102, 0.091710, 0.095208, 0.086160, 0.091305, 
0.085406, 0.094432, 0.078227, 0.099870, 0.094140, 0.080130, 
0.090707, 0.083268, 0.082222, 0.088767, 0.084477, 0.088069, 
0.089382, 0.086164, 0.089123, 0.092799, 0.076710, 0.090727, 
0.077097, 0.099905, 0.089733, 0.088101, 0.093705, 0.089215, 
0.085110, 0.086032, 0.083719, 0.079693, 0.088116, 0.090519, 
0.091150, 0.090855, 0.090547, 0.077773, 0.081914, 0.080864, 
0.082935, 0.092952, 0.079390, 0.080255, 0.091123, 0.091331, 
0.095160, 0.089343, 0.083353, 0.087445, 0.094036, 0.084719, 
0.099665, 0.085104, 0.100912, 0.085958, 0.083972, 0.098284, 
0.078318, 0.082042, 0.088007, 0.085469, 0.089984, 0.081181, 
0.098850, 0.086409, 0.089070, 0.081579, 0.089622, 0.089396, 
0.093922, 0.089472, 0.090806, 0.075034, 0.090346, 0.083871, 
0.086931, 0.089207, 0.094425, 0.087830, 0.079537, 0.097316, 
0.096513, 0.092264, 0.082211, 0.083841, 0.081861, 0.092591, 
0.077785, 0.079646, 0.093721, 0.090735, 0.086910, 0.075837, 
0.090729, 0.092800, 0.098704, 0.092441, 0.086404, 0.089344, 
0.083650, 0.082569, 0.085753, 0.091430, 0.085460, 0.095210, 
0.083226, 0.094882, 0.084856, 0.093350, 0.084579, 0.096744, 
0.099211, 0.085142, 0.085499, 0.083395, 0.088352, 0.110418, 
0.093788, 0.096140, 0.082758, 0.097145, 0.089170, 0.090720, 
0.084708, 0.095927, 0.090330, 0.074239, 0.089532, 0.090514, 
0.089823, 0.089709, 0.101840, 0.082676, 0.087157, 0.079221, 
0.096460, 0.108192, 0.088904, 0.084510, 0.096624, 0.099242, 
0.094470, 0.092473, 0.091745, 0.090439, 0.095316, 0.087963, 
0.096400, 0.089479, 0.094880, 0.085964, 0.092775, 0.083200, 
0.095133, 0.090079, 0.088828, 0.087600, 0.094123, 0.089135, 
0.082617, 0.085109, 0.114091, 0.096331, 0.108465, 0.080318, 
0.106576, 0.089671, 0.092023, 0.090722, 0.086603, 0.091788, 
0.098375, 0.082712, 0.102681, 0.098869, 0.089051, 0.109972, 
0.080440, 0.097860, 0.093074, 0.097028, 0.107826, 0.117152, 
0.090968, 0.096790, 0.096725, 0.094641, 0.089535, 0.092561, 
0.095828, 0.084320, 0.089942, 0.105476, 0.087495, 0.089805, 
0.092238, 0.094141, 0.104390, 0.082958, 0.097449, 0.099594, 
0.092640, 0.097332, 0.093223, 0.100183, 0.092511, 0.087035, 
0.088741, 0.097856, 0.116682, 0.091732, 0.097753, 0.094283, 
0.094235, 0.082016, 0.098370, 0.085676, 0.104529, 0.087319, 
0.090289, 0.105314, 0.103634, 0.101261, 0.092764, 0.098217, 
0.098939, 0.096071, 0.096071, 0.094027, 0.092713, 0.089542, 
0.112293, 0.112293, 0.089531, 0.087752};
wDD = 0.188264;
wND = 0.102252;
wSD = -1;

    Mmin = bin[0];
    Mmax = bin[nbin];
    if(M<Mmin || M>Mmax) return kTRUE;

    Int_t ibin=nbin-1;
    for(Int_t i=1; i<=nbin; i++) 
      if(M<=bin[i]) {
	ibin=i-1;
	//	printf("Mi> %f && Mi< %f\n", bin[i-1], bin[i]);
	break;
      }
    wSD=w[ibin];
    return kTRUE;

  }
  else if(TMath::Abs(fEnergyCMS-2760)<1 ){
   
const Int_t nbin=400;
Double_t bin[]={
1.080000, 1.577300, 2.074600, 2.571900, 3.069200, 3.566500, 
4.063800, 4.561100, 5.058400, 5.555700, 6.053000, 6.550300, 
7.047600, 7.544900, 8.042200, 8.539500, 9.036800, 9.534100, 
10.031400, 10.528700, 11.026000, 11.523300, 12.020600, 12.517900, 
13.015200, 13.512500, 14.009800, 14.507100, 15.004400, 15.501700, 
15.999000, 16.496300, 16.993600, 17.490900, 17.988200, 18.485500, 
18.982800, 19.480100, 19.977400, 20.474700, 20.972000, 21.469300, 
21.966600, 22.463900, 22.961200, 23.458500, 23.955800, 24.453100, 
24.950400, 25.447700, 25.945000, 26.442300, 26.939600, 27.436900, 
27.934200, 28.431500, 28.928800, 29.426100, 29.923400, 30.420700, 
30.918000, 31.415300, 31.912600, 32.409900, 32.907200, 33.404500, 
33.901800, 34.399100, 34.896400, 35.393700, 35.891000, 36.388300, 
36.885600, 37.382900, 37.880200, 38.377500, 38.874800, 39.372100, 
39.869400, 40.366700, 40.864000, 41.361300, 41.858600, 42.355900, 
42.853200, 43.350500, 43.847800, 44.345100, 44.842400, 45.339700, 
45.837000, 46.334300, 46.831600, 47.328900, 47.826200, 48.323500, 
48.820800, 49.318100, 49.815400, 50.312700, 50.810000, 51.307300, 
51.804600, 52.301900, 52.799200, 53.296500, 53.793800, 54.291100, 
54.788400, 55.285700, 55.783000, 56.280300, 56.777600, 57.274900, 
57.772200, 58.269500, 58.766800, 59.264100, 59.761400, 60.258700, 
60.756000, 61.253300, 61.750600, 62.247900, 62.745200, 63.242500, 
63.739800, 64.237100, 64.734400, 65.231700, 65.729000, 66.226300, 
66.723600, 67.220900, 67.718200, 68.215500, 68.712800, 69.210100, 
69.707400, 70.204700, 70.702000, 71.199300, 71.696600, 72.193900, 
72.691200, 73.188500, 73.685800, 74.183100, 74.680400, 75.177700, 
75.675000, 76.172300, 76.669600, 77.166900, 77.664200, 78.161500, 
78.658800, 79.156100, 79.653400, 80.150700, 80.648000, 81.145300, 
81.642600, 82.139900, 82.637200, 83.134500, 83.631800, 84.129100, 
84.626400, 85.123700, 85.621000, 86.118300, 86.615600, 87.112900, 
87.610200, 88.107500, 88.604800, 89.102100, 89.599400, 90.096700, 
90.594000, 91.091300, 91.588600, 92.085900, 92.583200, 93.080500, 
93.577800, 94.075100, 94.572400, 95.069700, 95.567000, 96.064300, 
96.561600, 97.058900, 97.556200, 98.053500, 98.550800, 99.048100, 
99.545400, 100.042700, 100.540000, 101.037300, 101.534600, 102.031900, 
102.529200, 103.026500, 103.523800, 104.021100, 104.518400, 105.015700, 
105.513000, 106.010300, 106.507600, 107.004900, 107.502200, 107.999500, 
108.496800, 108.994100, 109.491400, 109.988700, 110.486000, 110.983300, 
111.480600, 111.977900, 112.475200, 112.972500, 113.469800, 113.967100, 
114.464400, 114.961700, 115.459000, 115.956300, 116.453600, 116.950900, 
117.448200, 117.945500, 118.442800, 118.940100, 119.437400, 119.934700, 
120.432000, 120.929300, 121.426600, 121.923900, 122.421200, 122.918500, 
123.415800, 123.913100, 124.410400, 124.907700, 125.405000, 125.902300, 
126.399600, 126.896900, 127.394200, 127.891500, 128.388800, 128.886100, 
129.383400, 129.880700, 130.378000, 130.875300, 131.372600, 131.869900, 
132.367200, 132.864500, 133.361800, 133.859100, 134.356400, 134.853700, 
135.351000, 135.848300, 136.345600, 136.842900, 137.340200, 137.837500, 
138.334800, 138.832100, 139.329400, 139.826700, 140.324000, 140.821300, 
141.318600, 141.815900, 142.313200, 142.810500, 143.307800, 143.805100, 
144.302400, 144.799700, 145.297000, 145.794300, 146.291600, 146.788900, 
147.286200, 147.783500, 148.280800, 148.778100, 149.275400, 149.772700, 
150.270000, 150.767300, 151.264600, 151.761900, 152.259200, 152.756500, 
153.253800, 153.751100, 154.248400, 154.745700, 155.243000, 155.740300, 
156.237600, 156.734900, 157.232200, 157.729500, 158.226800, 158.724100, 
159.221400, 159.718700, 160.216000, 160.713300, 161.210600, 161.707900, 
162.205200, 162.702500, 163.199800, 163.697100, 164.194400, 164.691700, 
165.189000, 165.686300, 166.183600, 166.680900, 167.178200, 167.675500, 
168.172800, 168.670100, 169.167400, 169.664700, 170.162000, 170.659300, 
171.156600, 171.653900, 172.151200, 172.648500, 173.145800, 173.643100, 
174.140400, 174.637700, 175.135000, 175.632300, 176.129600, 176.626900, 
177.124200, 177.621500, 178.118800, 178.616100, 179.113400, 179.610700, 
180.108000, 180.605300, 181.102600, 181.599900, 182.097200, 182.594500, 
183.091800, 183.589100, 184.086400, 184.583700, 185.081000, 185.578300, 
186.075600, 186.572900, 187.070200, 187.567500, 188.064800, 188.562100, 
189.059400, 189.556700, 190.054000, 190.551300, 191.048600, 191.545900, 
192.043200, 192.540500, 193.037800, 193.535100, 194.032400, 194.529700, 
195.027000, 195.524300, 196.021600, 196.518900, 197.016200, 197.513500, 
198.010800, 198.508100, 199.005400, 199.502700, 200.000000};
Double_t w[]={
1.000000, 0.402402, 0.347976, 0.386866, 0.304413, 0.275746, 
0.256941, 0.250439, 0.228486, 0.219225, 0.203368, 0.196124, 
0.180551, 0.169230, 0.159021, 0.157136, 0.154743, 0.160522, 
0.153993, 0.143448, 0.148062, 0.136337, 0.130853, 0.127798, 
0.137576, 0.137530, 0.124175, 0.120225, 0.120774, 0.123584, 
0.121883, 0.114522, 0.118338, 0.119587, 0.106017, 0.125802, 
0.106256, 0.101585, 0.097737, 0.100817, 0.102279, 0.110935, 
0.101429, 0.107629, 0.105802, 0.095688, 0.096617, 0.100985, 
0.101521, 0.100450, 0.111836, 0.088828, 0.102264, 0.101012, 
0.097673, 0.097634, 0.091720, 0.096167, 0.084537, 0.104443, 
0.091966, 0.097204, 0.089456, 0.088273, 0.098220, 0.095151, 
0.088201, 0.085570, 0.088431, 0.077625, 0.088314, 0.084582, 
0.091545, 0.083774, 0.086183, 0.089195, 0.086255, 0.077167, 
0.096118, 0.089881, 0.086206, 0.086968, 0.079827, 0.082002, 
0.083081, 0.076587, 0.081820, 0.092382, 0.082964, 0.080901, 
0.093512, 0.082907, 0.086691, 0.093639, 0.089010, 0.082857, 
0.090795, 0.082403, 0.078602, 0.087284, 0.079638, 0.082905, 
0.077189, 0.084329, 0.075900, 0.082559, 0.084210, 0.070053, 
0.068453, 0.083369, 0.077659, 0.073286, 0.075396, 0.073946, 
0.083293, 0.076235, 0.082184, 0.080104, 0.073755, 0.077369, 
0.068281, 0.084593, 0.074923, 0.069467, 0.072094, 0.077973, 
0.067034, 0.080314, 0.087441, 0.071728, 0.072302, 0.076783, 
0.079931, 0.075557, 0.076318, 0.079029, 0.075126, 0.076859, 
0.080253, 0.074344, 0.083387, 0.087553, 0.084437, 0.076322, 
0.077748, 0.087559, 0.075649, 0.076615, 0.077098, 0.074559, 
0.075540, 0.085883, 0.079269, 0.079866, 0.068922, 0.069770, 
0.068024, 0.077783, 0.086620, 0.086769, 0.068433, 0.078059, 
0.084463, 0.072790, 0.073889, 0.079090, 0.081759, 0.069576, 
0.069160, 0.071695, 0.078569, 0.075727, 0.088055, 0.082395, 
0.079915, 0.075150, 0.073580, 0.069968, 0.071141, 0.065823, 
0.077270, 0.076229, 0.071735, 0.079271, 0.078209, 0.069503, 
0.064732, 0.076312, 0.088579, 0.087271, 0.080566, 0.073527, 
0.071376, 0.068785, 0.071812, 0.072680, 0.067252, 0.059543, 
0.072818, 0.071856, 0.083427, 0.080664, 0.077315, 0.066060, 
0.078772, 0.065442, 0.067255, 0.077676, 0.068306, 0.071578, 
0.059728, 0.073526, 0.078932, 0.070316, 0.067620, 0.067323, 
0.078316, 0.080366, 0.091194, 0.066739, 0.076238, 0.070382, 
0.077118, 0.073822, 0.072830, 0.062947, 0.077375, 0.076284, 
0.070951, 0.086127, 0.071074, 0.076621, 0.070502, 0.070897, 
0.058764, 0.065322, 0.073620, 0.062051, 0.074548, 0.083770, 
0.072502, 0.066823, 0.074192, 0.081570, 0.067340, 0.081896, 
0.080680, 0.072835, 0.069675, 0.078881, 0.072826, 0.069616, 
0.082744, 0.069138, 0.073333, 0.080881, 0.069091, 0.064266, 
0.062849, 0.072846, 0.067462, 0.067229, 0.071319, 0.071099, 
0.069381, 0.073799, 0.076106, 0.075006, 0.073074, 0.092589, 
0.074277, 0.061324, 0.069788, 0.070310, 0.063160, 0.073077, 
0.075447, 0.081013, 0.074102, 0.076565, 0.060192, 0.058326, 
0.071508, 0.072974, 0.065098, 0.060432, 0.062077, 0.075644, 
0.075417, 0.066947, 0.066744, 0.065111, 0.075468, 0.078185, 
0.073216, 0.066502, 0.079582, 0.065405, 0.069828, 0.072165, 
0.071112, 0.064656, 0.080396, 0.066505, 0.063419, 0.066128, 
0.080616, 0.071463, 0.064867, 0.074782, 0.063103, 0.068179, 
0.064911, 0.073029, 0.075746, 0.062264, 0.099126, 0.067890, 
0.074940, 0.070090, 0.073583, 0.056904, 0.063745, 0.063780, 
0.076876, 0.066519, 0.067164, 0.068668, 0.075065, 0.070983, 
0.075751, 0.066317, 0.077520, 0.073139, 0.073961, 0.085492, 
0.074629, 0.059555, 0.081026, 0.059274, 0.055448, 0.084269, 
0.066222, 0.078874, 0.062021, 0.075156, 0.090478, 0.066265, 
0.067845, 0.076653, 0.086671, 0.082362, 0.067246, 0.075975, 
0.072594, 0.068570, 0.095034, 0.065644, 0.070945, 0.066190, 
0.075906, 0.055154, 0.075575, 0.058078, 0.071004, 0.070850, 
0.064374, 0.058251, 0.069430, 0.072293, 0.065484, 0.084197, 
0.090119, 0.091619, 0.067620, 0.079462, 0.063025, 0.068128, 
0.056927, 0.076351, 0.073869, 0.061597, 0.083741, 0.063762, 
0.064489, 0.074269, 0.068832, 0.058648, 0.069536, 0.074824, 
0.081009, 0.073389, 0.076792, 0.084855, 0.075280, 0.061473, 
0.063840, 0.062891, 0.071328, 0.065250};
wDD = 0.077845;
wND = 0.066355;
wSD = -1;

    Mmin = bin[0];
    Mmax = bin[nbin];
    if(M<Mmin || M>Mmax) return kTRUE;

    Int_t ibin=nbin-1;
    for(Int_t i=1; i<=nbin; i++) 
      if(M<=bin[i]) {
	ibin=i-1;
	//	printf("Mi> %f && Mi< %f\n", bin[i-1], bin[i]);
	break;
      }
    wSD=w[ibin];
    return kTRUE;
   
  }
  else if(TMath::Abs(fEnergyCMS-7000)<1 ){
    
const Int_t nbin=400;
Double_t bin[]={
1.080000, 1.577300, 2.074600, 2.571900, 3.069200, 3.566500, 
4.063800, 4.561100, 5.058400, 5.555700, 6.053000, 6.550300, 
7.047600, 7.544900, 8.042200, 8.539500, 9.036800, 9.534100, 
10.031400, 10.528700, 11.026000, 11.523300, 12.020600, 12.517900, 
13.015200, 13.512500, 14.009800, 14.507100, 15.004400, 15.501700, 
15.999000, 16.496300, 16.993600, 17.490900, 17.988200, 18.485500, 
18.982800, 19.480100, 19.977400, 20.474700, 20.972000, 21.469300, 
21.966600, 22.463900, 22.961200, 23.458500, 23.955800, 24.453100, 
24.950400, 25.447700, 25.945000, 26.442300, 26.939600, 27.436900, 
27.934200, 28.431500, 28.928800, 29.426100, 29.923400, 30.420700, 
30.918000, 31.415300, 31.912600, 32.409900, 32.907200, 33.404500, 
33.901800, 34.399100, 34.896400, 35.393700, 35.891000, 36.388300, 
36.885600, 37.382900, 37.880200, 38.377500, 38.874800, 39.372100, 
39.869400, 40.366700, 40.864000, 41.361300, 41.858600, 42.355900, 
42.853200, 43.350500, 43.847800, 44.345100, 44.842400, 45.339700, 
45.837000, 46.334300, 46.831600, 47.328900, 47.826200, 48.323500, 
48.820800, 49.318100, 49.815400, 50.312700, 50.810000, 51.307300, 
51.804600, 52.301900, 52.799200, 53.296500, 53.793800, 54.291100, 
54.788400, 55.285700, 55.783000, 56.280300, 56.777600, 57.274900, 
57.772200, 58.269500, 58.766800, 59.264100, 59.761400, 60.258700, 
60.756000, 61.253300, 61.750600, 62.247900, 62.745200, 63.242500, 
63.739800, 64.237100, 64.734400, 65.231700, 65.729000, 66.226300, 
66.723600, 67.220900, 67.718200, 68.215500, 68.712800, 69.210100, 
69.707400, 70.204700, 70.702000, 71.199300, 71.696600, 72.193900, 
72.691200, 73.188500, 73.685800, 74.183100, 74.680400, 75.177700, 
75.675000, 76.172300, 76.669600, 77.166900, 77.664200, 78.161500, 
78.658800, 79.156100, 79.653400, 80.150700, 80.648000, 81.145300, 
81.642600, 82.139900, 82.637200, 83.134500, 83.631800, 84.129100, 
84.626400, 85.123700, 85.621000, 86.118300, 86.615600, 87.112900, 
87.610200, 88.107500, 88.604800, 89.102100, 89.599400, 90.096700, 
90.594000, 91.091300, 91.588600, 92.085900, 92.583200, 93.080500, 
93.577800, 94.075100, 94.572400, 95.069700, 95.567000, 96.064300, 
96.561600, 97.058900, 97.556200, 98.053500, 98.550800, 99.048100, 
99.545400, 100.042700, 100.540000, 101.037300, 101.534600, 102.031900, 
102.529200, 103.026500, 103.523800, 104.021100, 104.518400, 105.015700, 
105.513000, 106.010300, 106.507600, 107.004900, 107.502200, 107.999500, 
108.496800, 108.994100, 109.491400, 109.988700, 110.486000, 110.983300, 
111.480600, 111.977900, 112.475200, 112.972500, 113.469800, 113.967100, 
114.464400, 114.961700, 115.459000, 115.956300, 116.453600, 116.950900, 
117.448200, 117.945500, 118.442800, 118.940100, 119.437400, 119.934700, 
120.432000, 120.929300, 121.426600, 121.923900, 122.421200, 122.918500, 
123.415800, 123.913100, 124.410400, 124.907700, 125.405000, 125.902300, 
126.399600, 126.896900, 127.394200, 127.891500, 128.388800, 128.886100, 
129.383400, 129.880700, 130.378000, 130.875300, 131.372600, 131.869900, 
132.367200, 132.864500, 133.361800, 133.859100, 134.356400, 134.853700, 
135.351000, 135.848300, 136.345600, 136.842900, 137.340200, 137.837500, 
138.334800, 138.832100, 139.329400, 139.826700, 140.324000, 140.821300, 
141.318600, 141.815900, 142.313200, 142.810500, 143.307800, 143.805100, 
144.302400, 144.799700, 145.297000, 145.794300, 146.291600, 146.788900, 
147.286200, 147.783500, 148.280800, 148.778100, 149.275400, 149.772700, 
150.270000, 150.767300, 151.264600, 151.761900, 152.259200, 152.756500, 
153.253800, 153.751100, 154.248400, 154.745700, 155.243000, 155.740300, 
156.237600, 156.734900, 157.232200, 157.729500, 158.226800, 158.724100, 
159.221400, 159.718700, 160.216000, 160.713300, 161.210600, 161.707900, 
162.205200, 162.702500, 163.199800, 163.697100, 164.194400, 164.691700, 
165.189000, 165.686300, 166.183600, 166.680900, 167.178200, 167.675500, 
168.172800, 168.670100, 169.167400, 169.664700, 170.162000, 170.659300, 
171.156600, 171.653900, 172.151200, 172.648500, 173.145800, 173.643100, 
174.140400, 174.637700, 175.135000, 175.632300, 176.129600, 176.626900, 
177.124200, 177.621500, 178.118800, 178.616100, 179.113400, 179.610700, 
180.108000, 180.605300, 181.102600, 181.599900, 182.097200, 182.594500, 
183.091800, 183.589100, 184.086400, 184.583700, 185.081000, 185.578300, 
186.075600, 186.572900, 187.070200, 187.567500, 188.064800, 188.562100, 
189.059400, 189.556700, 190.054000, 190.551300, 191.048600, 191.545900, 
192.043200, 192.540500, 193.037800, 193.535100, 194.032400, 194.529700, 
195.027000, 195.524300, 196.021600, 196.518900, 197.016200, 197.513500, 
198.010800, 198.508100, 199.005400, 199.502700, 200.000000};
Double_t w[]={
1.000000, 0.526293, 0.446686, 0.437789, 0.366854, 0.333320, 
0.291931, 0.266464, 0.253870, 0.248706, 0.232788, 0.220736, 
0.209886, 0.202741, 0.188617, 0.182767, 0.178748, 0.169039, 
0.175911, 0.169098, 0.171256, 0.146728, 0.144543, 0.159470, 
0.153171, 0.151883, 0.144693, 0.136307, 0.140226, 0.135388, 
0.141317, 0.151121, 0.131209, 0.144039, 0.124688, 0.128020, 
0.119122, 0.121868, 0.122332, 0.119561, 0.115744, 0.102504, 
0.114726, 0.109518, 0.115418, 0.114860, 0.110026, 0.107693, 
0.103005, 0.115985, 0.108629, 0.105937, 0.101056, 0.101228, 
0.113305, 0.110302, 0.104696, 0.107447, 0.099095, 0.107378, 
0.103090, 0.111384, 0.090821, 0.109806, 0.093639, 0.096655, 
0.110416, 0.104446, 0.098530, 0.095105, 0.106641, 0.101804, 
0.091798, 0.094775, 0.104130, 0.088436, 0.119692, 0.099160, 
0.086163, 0.089250, 0.101898, 0.091786, 0.087346, 0.095312, 
0.107186, 0.085671, 0.093283, 0.091992, 0.114654, 0.086172, 
0.084559, 0.097862, 0.079417, 0.094737, 0.089678, 0.105265, 
0.099092, 0.087901, 0.086828, 0.084756, 0.077769, 0.084254, 
0.083262, 0.092935, 0.088858, 0.093377, 0.083569, 0.084771, 
0.084957, 0.084033, 0.095007, 0.076690, 0.087726, 0.097913, 
0.084119, 0.076361, 0.084172, 0.091068, 0.094564, 0.091406, 
0.088405, 0.090355, 0.085388, 0.084586, 0.085752, 0.084298, 
0.091294, 0.081491, 0.075024, 0.082543, 0.088048, 0.074621, 
0.078514, 0.077249, 0.078378, 0.092945, 0.075859, 0.077602, 
0.074592, 0.078181, 0.081527, 0.080865, 0.078854, 0.078237, 
0.085506, 0.089823, 0.072738, 0.078430, 0.077162, 0.081495, 
0.090878, 0.099417, 0.100966, 0.087960, 0.085556, 0.081661, 
0.078066, 0.089059, 0.073564, 0.092704, 0.073148, 0.098057, 
0.067906, 0.079452, 0.090567, 0.082454, 0.077851, 0.079694, 
0.091272, 0.078628, 0.096906, 0.098779, 0.088906, 0.070174, 
0.083822, 0.084241, 0.093237, 0.071062, 0.075771, 0.096405, 
0.098441, 0.086007, 0.069599, 0.078400, 0.083481, 0.081054, 
0.085552, 0.069582, 0.071336, 0.073207, 0.087913, 0.080125, 
0.075189, 0.067217, 0.073509, 0.099694, 0.080781, 0.073943, 
0.062822, 0.076611, 0.087672, 0.077632, 0.063959, 0.077754, 
0.084651, 0.097348, 0.063909, 0.055053, 0.087616, 0.074428, 
0.101165, 0.078446, 0.070683, 0.071162, 0.091516, 0.069641, 
0.070969, 0.069778, 0.083318, 0.066703, 0.074027, 0.067589, 
0.070620, 0.081307, 0.094406, 0.076188, 0.061663, 0.077561, 
0.076159, 0.071851, 0.074417, 0.076136, 0.069073, 0.075450, 
0.070218, 0.089709, 0.079974, 0.082077, 0.076979, 0.070497, 
0.076296, 0.087347, 0.073508, 0.088073, 0.067186, 0.059898, 
0.085495, 0.100152, 0.079468, 0.093783, 0.082680, 0.077272, 
0.081995, 0.068240, 0.071881, 0.073737, 0.072346, 0.079034, 
0.078721, 0.067518, 0.068196, 0.081738, 0.082814, 0.082480, 
0.069072, 0.066853, 0.081523, 0.073588, 0.082308, 0.091641, 
0.072747, 0.084317, 0.072190, 0.079372, 0.067424, 0.077450, 
0.062343, 0.090708, 0.065470, 0.086588, 0.071172, 0.066705, 
0.070670, 0.070407, 0.096111, 0.066738, 0.081249, 0.072882, 
0.075144, 0.060331, 0.074589, 0.076968, 0.085913, 0.072561, 
0.064645, 0.078742, 0.075670, 0.065984, 0.080932, 0.069898, 
0.065303, 0.096856, 0.057690, 0.065720, 0.066545, 0.068521, 
0.068278, 0.069245, 0.086643, 0.063401, 0.070933, 0.070752, 
0.066978, 0.058891, 0.070073, 0.078031, 0.082691, 0.101414, 
0.075814, 0.072790, 0.057622, 0.093002, 0.084660, 0.079216, 
0.070371, 0.070141, 0.076944, 0.067285, 0.078016, 0.077807, 
0.066668, 0.066459, 0.059962, 0.062774, 0.083450, 0.064554, 
0.067887, 0.064165, 0.072782, 0.067285, 0.052710, 0.096824, 
0.071931, 0.064190, 0.074442, 0.082647, 0.055797, 0.078632, 
0.061116, 0.063092, 0.049131, 0.074517, 0.069915, 0.079021, 
0.088656, 0.101045, 0.090432, 0.076460, 0.067451, 0.071381, 
0.069790, 0.077330, 0.077115, 0.084403, 0.065138, 0.056436, 
0.088024, 0.069893, 0.055985, 0.089655, 0.062911, 0.075311, 
0.086679, 0.093645, 0.068632, 0.064498, 0.057677, 0.081391, 
0.062781, 0.075467, 0.061314, 0.073394, 0.084462, 0.068470, 
0.071267, 0.060556, 0.072487, 0.063785, 0.079164, 0.070406, 
0.073394, 0.063168, 0.066968, 0.064125, 0.063954, 0.072504, 
0.058948, 0.057740, 0.083383, 0.067262};
wDD = 0.098024;
wND = 0.053452;
wSD = -1;

    Mmin = bin[0];
    Mmax = bin[nbin];
    if(M<Mmin || M>Mmax) return kTRUE;

    Int_t ibin=nbin-1;
    for(Int_t i=1; i<=nbin; i++) 
      if(M<=bin[i]) {
	ibin=i-1;
	//	printf("Mi> %f && Mi< %f\n", bin[i-1], bin[i]);
	break;
      }
    wSD=w[ibin];
    return kTRUE;
   
  }

  return kFALSE;
}


//______________________________________________________________________________
