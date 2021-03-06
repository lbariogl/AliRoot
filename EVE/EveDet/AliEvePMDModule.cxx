// $Id$
// Main authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/**************************************************************************
 * Copyright(c) 1998-2008, ALICE Experiment at CERN, all rights reserved. *
 * See http://aliceinfo.cern.ch/Offline/AliRoot/License.html for          *
 * full copyright notice.                                                 *
 **************************************************************************/

#include "AliEvePMDModule.h"

#include "AliPMDdigit.h"
#include "AliPMDddldata.h"

#include <TEveTrans.h>

#include <TClonesArray.h>
#include <TTree.h>
#include <TH1F.h>

const Float_t AliEvePMDModule::fgkRad        = 0.25;
const Float_t AliEvePMDModule::fgkSqRoot3    = 1.732050808;
const Float_t AliEvePMDModule::fgkZpos       = 0.;
Int_t         AliEvePMDModule::fgPreTotPads  = 0;
Int_t         AliEvePMDModule::fgCpvTotPads  = 0;
Int_t         AliEvePMDModule::fgPreTotAdc   = 0;
Int_t         AliEvePMDModule::fgCpvTotAdc   = 0;


//______________________________________________________________________________
// AliEvePMDModule
//


ClassImp(AliEvePMDModule)

AliEvePMDModule::AliEvePMDModule():
  fH1(0),
  fX(0.),
  fY(0.),
  fZ(0.),
  fNPads(0),
  fAdc(0)
{
  // Constructor.
}

AliEvePMDModule::~AliEvePMDModule()
{
  // Destructor.

  delete fH1;
}

// -------------------------------------------------------------------- //
void AliEvePMDModule::DisplayInit(Int_t ism)
{
  // Initialize display parameters for module ism.

  TString smodule = "Module";
  smodule+= ism;

  Float_t xism =0, yism = 0;
  Float_t dxism =0, dyism = 0;

  GenerateBox(ism,xism,yism,dxism,dyism);

  TEveFrameBox *pmdModBox = new TEveFrameBox();
  pmdModBox->SetAAQuadXY(xism, yism, 0, dxism, dyism);
  pmdModBox->SetFrameColor(Color_t(31));
  pmdModBox->SetFrameFill(kTRUE);
  SetFrame(pmdModBox);

  SetName(smodule.Data());
  SetOwnIds(kTRUE);
  Reset(TEveQuadSet::kQT_HexagonXY, kFALSE, 32);

  fH1 = new TH1F("fH1", smodule.Data(), 100, 0., 1000.);
  fH1->SetDirectory(0);
  fH1->GetXaxis()->SetTitle("Single Cell Edep (adc)");
}

// -------------------------------------------------------------------- //

void AliEvePMDModule::DisplayDigitsData(Int_t ism, TTree *pmdt)
{
  // Populate internal structures with data from pmdt for module ism.

  DisplayInit(ism);

  Int_t det, smn, irow, icol, adc;
  Int_t xpad = 0, ypad = 0;
  Float_t xpos, ypos;

  TClonesArray *digits = new TClonesArray("AliPMDdigit", 0);

  TBranch *branch = pmdt->GetBranch("PMDDigit");
  branch->SetAddress(&digits);

  AliPMDdigit  *pmddigit;

  branch->GetEntry(ism);
  Int_t nentries = digits->GetLast();
  //printf("%d\n", nentries);

  for (Int_t ient = 0; ient < nentries+1; ient++)
    {
      pmddigit = (AliPMDdigit*)digits->UncheckedAt(ient);

      det    = pmddigit->GetDetector();
      smn    = pmddigit->GetSMNumber();
      irow   = pmddigit->GetRow();
      icol   = pmddigit->GetColumn();
      adc    = (Int_t) pmddigit->GetADC();

      if(smn <12)
	{
	  xpad = icol;
	  ypad = irow;
	}
      else if(smn >=12 && smn < 24)
	{
	  xpad = irow;
	  ypad = icol;
	}

      RectGeomCellPos(smn, xpad, ypad, xpos, ypos);

      AddHexagon(xpos, ypos, fgkZpos, fgkRad);

      QuadValue(adc);

      QuadId(new AliPMDdigit(*pmddigit));
      // new TNamed(Form("Quad with idx=%d", ient),
      //  "This title is not confusing."));

      ++fNPads;
      fAdc += adc;

      if (det == 0)
	{
	  fgPreTotAdc += (Int_t) adc;
	  ++fgPreTotPads;
	}
      if (det == 1)
	{
	  fgCpvTotAdc += (Int_t) adc;
	  ++fgCpvTotPads;
	}
      fH1->Fill((Float_t)adc);

    }

  RefitPlex();

  RefMainTrans().SetPos(fX, fY, fZ);

  delete digits;
}

// -------------------------------------------------------------------- //

void AliEvePMDModule::DisplayRawData(Int_t ism, TObjArray *ddlcont)
{
  // Populate internal structures with data from ddlcont for module ism.

  DisplayInit(ism);

  if (ism > 23) ism -= 24;

  Int_t det, smn, irow, icol, adc;
  Int_t xpad = 0, ypad = 0;
  Float_t xpos, ypos;

  Int_t nentries = ddlcont->GetEntries();
  //printf("%d\n", nentries);

  for (Int_t ient = 0; ient < nentries; ient++)
    {
      AliPMDddldata *pmdddl = (AliPMDddldata*)ddlcont->UncheckedAt(ient);

      det    = pmdddl->GetDetector();
      smn    = pmdddl->GetSMN();
      if (smn != ism) continue;
      irow   = pmdddl->GetRow();
      icol   = pmdddl->GetColumn();
      adc    = pmdddl->GetSignal();

      if(smn <12)
	{
	  xpad = icol;
	  ypad = irow;
	}
      else if(smn >=12 && smn < 24)
	{
	  xpad = irow;
	  ypad = icol;
	}

      RectGeomCellPos(smn, xpad, ypad, xpos, ypos);

      AddHexagon(xpos, ypos, fgkZpos, fgkRad);

      QuadValue(adc);

      QuadId(new AliPMDddldata(*pmdddl));
      //new TNamed(Form("Quad with idx=%d", ient),
      //  "This title is not confusing."));

      ++fNPads;
      fAdc += adc;

      if (det == 0)
	{
	  fgPreTotAdc += (Int_t) adc;
	  ++fgPreTotPads;
	}
      if (det == 1)
	{
	  fgCpvTotAdc += (Int_t) adc;
	  ++fgCpvTotPads;
	}

      fH1->Fill((Float_t) adc);
    }

  RefitPlex();

  RefMainTrans().SetPos(fX, fY, fZ);

}

// -------------------------------------------------------------------- //

void AliEvePMDModule::RectGeomCellPos(Int_t ism, Int_t xpad, Int_t ypad,
				Float_t &xpos, Float_t &ypos)
{
  // This routine finds the cell eta,phi for the new PMD rectangular
  // geometry in ALICE
  // Authors : Bedanga Mohanty and Dipak Mishra - 29.4.2003
  // modified by B. K. Nandi for change of coordinate sys
  //
  // SMA  ---> Supermodule Type A           ( SM - 0)
  // SMAR ---> Supermodule Type A ROTATED   ( SM - 1)
  // SMB  ---> Supermodule Type B           ( SM - 2)
  // SMBR ---> Supermodule Type B ROTATED   ( SM - 3)
  //
  // ism   : Serial module number from 0 to 23 for each plane


  // Corner positions (x,y) of the 24 unit moudles in ALICE PMD

  const Double_t kXcorner[24] =
    {
      74.8833,  53.0045, 31.1255,    //Type-A
      74.8833,  53.0045, 31.1255,    //Type-A
      -74.8833, -53.0044, -31.1255,  //Type-AR
      -74.8833, -53.0044, -31.1255,  //Type-AR
      8.9165, -33.7471,            //Type-B
      8.9165, -33.7471,            //Type-B
      8.9165, -33.7471,            //Type-B
      -8.9165, 33.7471,            //Type-BR
      -8.9165, 33.7471,            //Type-BR
      -8.9165, 33.7471,            //Type-BR
    };


  const Double_t kYcorner[24] =
    {
      86.225,  86.225,  86.225,      //Type-A
      37.075,  37.075,  37.075,      //Type-A
      -86.225, -86.225, -86.225,     //Type-AR
      -37.075, -37.075, -37.075,     //Type-AR
      86.225,  86.225,               //Type-B
      61.075,  61.075,               //Type-B
      35.925,  35.925,               //Type-B
      -86.225, -86.225,              //Type-BR
      -61.075, -61.075,              //Type-BR
      -35.925, -35.925               //Type-BR
    };


  //  const Float_t kSqroot3      = 1.732050808;  // sqrt(3.);
  //  const Float_t kCellRadius   = 0.25;

  //
  //Every even row of cells is shifted and placed
  //in geant so this condition
  //
  Float_t shift = 0.0;
  if(ypad%2 == 0)
    {
      shift = 0.0;
    }
  else
    {
      shift = 0.25;
    }


  if(ism < 6)
    {
      ypos = kYcorner[ism] - (Float_t) xpad*fgkRad*2.0 - shift;
      xpos = kXcorner[ism] - (Float_t) ypad*fgkSqRoot3*fgkRad;
    }
  else if(ism >=6 && ism < 12)
    {
      ypos = kYcorner[ism] + (Float_t) xpad*fgkRad*2.0 + shift;
      xpos = kXcorner[ism] + (Float_t) ypad*fgkSqRoot3*fgkRad;
    }
  else if(ism >= 12 && ism < 18)
    {
      ypos = kYcorner[ism] - (Float_t) xpad*fgkRad*2.0 - shift;
      xpos = kXcorner[ism] - (Float_t) ypad*fgkSqRoot3*fgkRad;
    }
  else if(ism >= 18 && ism < 24)
    {
      ypos = kYcorner[ism] + (Float_t) xpad*fgkRad*2.0 + shift;
      xpos = kXcorner[ism] + (Float_t) ypad*fgkSqRoot3*fgkRad;
    }

}

// -------------------------------------------------------------------- //

void AliEvePMDModule::GenerateBox(Int_t ism, Float_t &xism, Float_t &yism,
                                  Float_t &dxism, Float_t &dyism)
{
  // Generate bounding-box.

  const Float_t kDia     = 0.50;

  const Double_t kXcorner[24] =
    {
      74.8833,  53.0045, 31.1255,    //Type-A
      74.8833,  53.0045, 31.1255,    //Type-A
      -74.8833, -53.0044, -31.1255,  //Type-AR
      -74.8833, -53.0044, -31.1255,  //Type-AR
      8.9165, -33.7471,            //Type-B
      8.9165, -33.7471,            //Type-B
      8.9165, -33.7471,            //Type-B
      -8.9165, 33.7471,            //Type-BR
      -8.9165, 33.7471,            //Type-BR
      -8.9165, 33.7471,            //Type-BR
    };


  const Double_t kYcorner[24] =
    {
      86.225,  86.225,  86.225,      //Type-A
      37.075,  37.075,  37.075,      //Type-A
      -86.225, -86.225, -86.225,     //Type-AR
      -37.075, -37.075, -37.075,     //Type-AR
      86.225,  86.225,               //Type-B
      61.075,  61.075,               //Type-B
      35.925,  35.925,               //Type-B
      -86.225, -86.225,              //Type-BR
      -61.075, -61.075,              //Type-BR
      -35.925, -35.925               //Type-BR
    };


  if (ism > 23) ism -= 24;


  if (ism < 6)
    {
      xism  = kXcorner[ism] + fgkRad;
      yism  = kYcorner[ism] + fgkRad;
      dxism = -fgkRad*fgkSqRoot3*48.;
      dyism = -kDia*96. - fgkRad;
  }
  if (ism >= 6 && ism < 12)
    {
      xism  = kXcorner[ism] - fgkRad;
      yism  = kYcorner[ism] - fgkRad;
      dxism = fgkRad*fgkSqRoot3*48.;
      dyism = kDia*96. + fgkRad;
  }
  if (ism >= 12 && ism < 18)
    {
      xism  = kXcorner[ism] + fgkRad;
      yism  = kYcorner[ism] + fgkRad;
      dxism = -fgkRad*fgkSqRoot3*96.;
      dyism = -kDia*48. - fgkRad;
  }
  if (ism >= 18 && ism < 24)
    {
      xism  = kXcorner[ism] - fgkRad;
      yism  = kYcorner[ism] - fgkRad;
      dxism = fgkRad*fgkSqRoot3*96.;
      dyism = kDia*48. + fgkRad;
  }

}

// -------------------------------------------------------------------- //

void AliEvePMDModule::SetPosition(Float_t x, Float_t y, Float_t z)
{
  // Set position of module.

  fX = x;
  fY = y;
  fZ = z;
}

// -------------------------------------------------------------------- //
