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

#include "AliGenCocktailEventHeader.h"
#include "AliGenEventHeader.h"
#include <TList.h>

ClassImp(AliGenCocktailEventHeader)



AliGenCocktailEventHeader::AliGenCocktailEventHeader()
{
// Default Constructor
    fHeaders = 0x0;
}

AliGenCocktailEventHeader::AliGenCocktailEventHeader(const char* name):AliGenEventHeader(name)
{
// Constructor
    fHeaders = 0x0;
}

AliGenCocktailEventHeader::AliGenCocktailEventHeader(const AliGenCocktailEventHeader &header):
AliGenEventHeader(header)
{
// Copy Constructor
    header.Copy(*this);
}

AliGenCocktailEventHeader::~AliGenCocktailEventHeader()
{
// Constructor
    if (fHeaders) {
	fHeaders->Clear();
	delete fHeaders;
    }
}

void AliGenCocktailEventHeader::AddHeader(AliGenEventHeader* header)
{
// Add a header to the list
    if (!fHeaders) fHeaders = new TList();
    fHeaders->Add(header);
}

AliGenCocktailEventHeader& AliGenCocktailEventHeader::operator=(const  AliGenCocktailEventHeader& rhs)
{
// Assignment operator
    rhs.Copy(*this); 
    return (*this);
}
