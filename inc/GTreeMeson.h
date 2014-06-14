#ifndef __GTreeMeson_h__
#define __GTreeMeson_h__


#include <TClonesArray.h>
#include <TLorentzVector.h>

#include "GTree.h"


#define MASS_PI0    134.9766
#define MASS_ETA    547.853
#define MASS_ETAP   957.78
#define MASS_PROTON 938.272046


#define GTreeMeson_MaxEntries       16
#define GTreeMeson_MaxSubEntries    16

using namespace std;

class  GTreeMeson   : public GTree
{
private:
    UInt_t              nParticles;
    TClonesArray*       particles;
    UChar_t             nPhotons[GTreeMeson_MaxEntries];
    UChar_t             nChargedPi[GTreeMeson_MaxEntries];
    //Int_t               pdg[GTreeMeson_MaxSubEntries * GTreeMeson_MaxSubEntries];
    TClonesArray*       subParticles;

protected:
    virtual void    SetBranchAdresses();
    virtual void    SetBranches();

public:
    GTreeMeson(GTreeManager *Manager, const TString& _Name);
    virtual ~GTreeMeson();

            void            AddParticle(const TLorentzVector& subParticle0, const TLorentzVector& subParticle1, const Int_t pdg0, const Int_t pdg1);
            //void            AddParticle(const Int_t nSubParticles, TLorentzVector** subParticle_list, const Int_t* pdg_list);
            void            AddParticle(const Int_t nSubPhotons, TLorentzVector** subPhotons_list, const Int_t nSubChargedPi, TLorentzVector** subChargedPi_list);
    /*inline  void            AddParticle( const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1);
    inline  void            AddParticle(const GTreeMeson_DecayType type, const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1, const TLorentzVector& _SubParticle2);
    inline  void            AddParticle(const GTreeMeson_DecayType type, const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1, const TLorentzVector& _SubParticle2, const TLorentzVector& _SubParticle3);
    inline  void            AddParticle(const GTreeMeson_DecayType type, const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1, const TLorentzVector& _SubParticle2, const TLorentzVector& _SubParticle3, const TLorentzVector& _SubParticle4);
    inline  void            AddParticle(const GTreeMeson_DecayType type, const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1, const TLorentzVector& _SubParticle2, const TLorentzVector& _SubParticle3, const TLorentzVector& _SubParticle4, const TLorentzVector& _SubParticle5);*/
    virtual void            Clear()     {nParticles = 0; particles->Clear(); subParticles->Clear();}// subParticles[1]->Clear(); subParticles[2]->Clear(); subParticles[3]->Clear(); subParticles[4]->Clear(); subParticles[5]->Clear();}
            UInt_t          GetNParticles()                const	{return nParticles;}
            TLorentzVector& Meson(const Int_t particle)             {return *((TLorentzVector*)particles->At(particle));}
    const   TLorentzVector& Meson(const Int_t particle) const       {return *((TLorentzVector*)particles->At(particle));}
            TLorentzVector& Particle(const Int_t particle)          {return Meson(particle);}
    const   TLorentzVector& Particle(const Int_t particle) const    {return Meson(particle);}
            TLorentzVector& SubParticle(const Int_t meson, const Int_t subParticle)       {return *((TLorentzVector*)(((TClonesArray*)subParticles->At(subParticle))->At(subParticle)));}
    const   TLorentzVector& SubParticle(const Int_t meson, const Int_t subParticle) const {return *((TLorentzVector*)(((TClonesArray*)subParticles->At(subParticle))->At(subParticle)));}
};


/*void    GTreeMeson::AddParticle(const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1)
{
    nPhotons[nParticles]   = 0;
    new((*particles)[nParticles]) TLorentzVector(_SubParticle0 + _SubParticle1);
    //new((*subParticles[0])[nParticles]) TLorentzVector(_SubParticle0);
    //new((*subParticles[1])[nParticles]) TLorentzVector(_SubParticle1);
    nParticles++;
}/*
void    GTreeMeson::AddParticle(const GTreeMeson_DecayType type, const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1, const TLorentzVector& _SubParticle2)
{
    if(type != Decay_2ChargedPi1Gamma)
    {
        cout << "Given Wrong type or wrong # of subParticles in GTreeMeson::AddParticle" << endl;
        return;
    }

    decayType[nParticles]   = type;
    new((*particles)[nParticles]) TLorentzVector(_SubParticle0 + _SubParticle1 + _SubParticle2);
    new((*subParticles[0])[nParticles]) TLorentzVector(_SubParticle0);
    new((*subParticles[1])[nParticles]) TLorentzVector(_SubParticle1);
    new((*subParticles[2])[nParticles]) TLorentzVector(_SubParticle2);
    nParticles++;
}
void    GTreeMeson::AddParticle(const GTreeMeson_DecayType type, const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1, const TLorentzVector& _SubParticle2, const TLorentzVector& _SubParticle3)
{
    if(type != Decay_1Pi02ChargedPi)
    {
        cout << "Given Wrong type or wrong # of subParticles in GTreeMeson::AddParticle" << endl;
        return;
    }

    decayType[nParticles]   = type;
    new((*particles)[nParticles]) TLorentzVector(_SubParticle0 + _SubParticle1 + _SubParticle2 + _SubParticle3);
    new((*subParticles[0])[nParticles]) TLorentzVector(_SubParticle0);
    new((*subParticles[1])[nParticles]) TLorentzVector(_SubParticle1);
    new((*subParticles[2])[nParticles]) TLorentzVector(_SubParticle2);
    new((*subParticles[3])[nParticles]) TLorentzVector(_SubParticle3);
    nParticles++;
}
void    GTreeMeson::AddParticle(const GTreeMeson_DecayType type, const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1, const TLorentzVector& _SubParticle2, const TLorentzVector& _SubParticle3, const TLorentzVector& _SubParticle4)
{
    if(type != Decay_2Pi01ChargedPi && type != Decay_1Pi02ChargedPi1Gamma)
    {
        cout << "Given Wrong type or wrong # of subParticles in GTreeMeson::AddParticle" << endl;
        return;
    }

    decayType[nParticles]   = type;
    new((*particles)[nParticles]) TLorentzVector(_SubParticle0 + _SubParticle1 + _SubParticle2 + _SubParticle3 + _SubParticle4);
    new((*subParticles[0])[nParticles]) TLorentzVector(_SubParticle0);
    new((*subParticles[1])[nParticles]) TLorentzVector(_SubParticle1);
    new((*subParticles[2])[nParticles]) TLorentzVector(_SubParticle2);
    new((*subParticles[3])[nParticles]) TLorentzVector(_SubParticle3);
    new((*subParticles[4])[nParticles]) TLorentzVector(_SubParticle4);
    nParticles++;
}
void    GTreeMeson::AddParticle(const GTreeMeson_DecayType type, const TLorentzVector& _SubParticle0, const TLorentzVector& _SubParticle1, const TLorentzVector& _SubParticle2, const TLorentzVector& _SubParticle3, const TLorentzVector& _SubParticle4, const TLorentzVector& _SubParticle5)
{
    if(type != Decay_3Pi0 && type != Decay_2Pi01Eta)
    {
        cout << "Given Wrong type or wrong # of subParticles in GTreeMeson::AddParticle" << endl;
        return;
    }

    decayType[nParticles]   = type;
    new((*particles)[nParticles]) TLorentzVector(_SubParticle0 + _SubParticle1 + _SubParticle2 + _SubParticle3 + _SubParticle4 + _SubParticle5);
    new((*subParticles[0])[nParticles]) TLorentzVector(_SubParticle0);
    new((*subParticles[1])[nParticles]) TLorentzVector(_SubParticle1);
    new((*subParticles[2])[nParticles]) TLorentzVector(_SubParticle2);
    new((*subParticles[3])[nParticles]) TLorentzVector(_SubParticle3);
    new((*subParticles[4])[nParticles]) TLorentzVector(_SubParticle4);
    new((*subParticles[5])[nParticles]) TLorentzVector(_SubParticle5);
    nParticles++;
}*/

#endif
