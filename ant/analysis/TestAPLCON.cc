#include "TestAPLCON.h"
#include "Particle.h"
#include "Track.h"
#include "plot/plotter.h"
#include "Track.h"
#include "plot/root_draw.h"
#include <string>
#include "utils/combinatorics.h"
#include <vector>
#include <numeric>
#include <functional>
#include <APLCON.hpp>
#include <iomanip>

using namespace std;
using namespace ant;

std::default_random_engine ant::analysis::TestAPLCON::FitParticle::generator;

TLorentzVector analysis::TestAPLCON::FitParticle::Make(const std::vector<double> &EkThetaPhi, const Double_t m) {
    const double E = EkThetaPhi[0] + m;
    const Double_t p = sqrt( E*E - m*m );
    TVector3 pv(1,0,0);
    pv.SetMagThetaPhi(p, EkThetaPhi[1], EkThetaPhi[2]);
    TLorentzVector l(pv, E);
    return l;
}

void analysis::TestAPLCON::FitParticle::Smear() {
    // set the sigmas here,
    // then the fitter knows them as well (because they're linked)

    Ek_Sigma = 0.02*Ek*pow(Ek,-0.36);
    Theta_Sigma = 2.5*TMath::DegToRad();
    if(Theta>20*TMath::DegToRad() && Theta<160*TMath::DegToRad()) {
        Phi_Sigma = Theta_Sigma/sin(Theta);
    }
    else {
        Phi_Sigma = 1*TMath::DegToRad();
    }

    // then artificially smear the values with gaussians
    using gauss_t = std::normal_distribution<double>;
    gauss_t gauss_Ek(0, Ek_Sigma);
    Ek += gauss_Ek(generator);
    gauss_t gauss_Theta(0, Theta_Sigma);
    Theta += gauss_Theta(generator);
    gauss_t gauss_Phi(0, Phi_Sigma);
    Phi += gauss_Phi(generator);
}

void analysis::TestAPLCON::FillIM(TH1D *h, const std::vector<analysis::TestAPLCON::FitParticle> &photons) {
    TLorentzVector sum(0,0,0,0);
    for(const auto& p : photons) {
        sum += FitParticle::Make(p, ParticleTypeDatabase::Photon.Mass());
    }
    h->Fill(sum.M());
}

ant::analysis::TestAPLCON::TestAPLCON(const mev_t energy_scale) :
    hf("TestAPLCON"),
    fitter("TestAPLCON"),
    photons(nPhotons)
{


    const HistogramFactory::BinSettings energy_bins(1000,0,energy_scale);
    const HistogramFactory::BinSettings tagger_bins(2000,0.0,2000);
    const HistogramFactory::BinSettings ntaggerhits_bins(100);
    const HistogramFactory::BinSettings veto_bins(1000,0,10.0);
    const HistogramFactory::BinSettings particle_bins(10,0,10);
    const HistogramFactory::BinSettings particlecount_bins(16,0,16);
    const HistogramFactory::BinSettings pull_bins(50,-3,3);
    const HistogramFactory::BinSettings chisqare_bins(100,0,30);
    const HistogramFactory::BinSettings probability_bins(100,0,1);
    const HistogramFactory::BinSettings iterations_bins(15,0,15);
    const HistogramFactory::BinSettings im_bins(200,IM-100,IM+100);
    const HistogramFactory::BinSettings vertex_bins(200,-10,10);

    banana = hf.Make2D(
                "PID Bananas",
                "Cluster Energy [MeV]",
                "Veto Energy [MeV]",
                energy_bins,
                veto_bins,
                "pid"
                );

    particles = hf.Make1D(
                "Identified particles",
                "Particle Type",
                "#",
                particle_bins,
                "ParticleTypes"
                );
    tagger = hf.Make1D(
                "Tagger Spectrum",
                "Photon Beam Energy",
                "#",
                tagger_bins,
                "TaggerSpectrum"
                );

    ntagged = hf.Make1D(
                "Tagger Hits",
                "Tagger Hits / event",
                "#",
                ntaggerhits_bins,
                "nTagged"
                );

    cbesum = hf.Make1D(
                "CB Energy Sum",
                "E [MeV]",
                "#",
                energy_bins,
                "esum"
                );

    for( auto& t : ParticleTypeDatabase::DetectableTypes() ) {
        numParticleType[t]= hf.Make1D("Number of "+t->PrintName(),
                                      "number of "+t->PrintName()+"/ event",
                                      "",
                                      particlecount_bins);
    }

    // setup fitter for nPhotons

    fitter.LinkVariable("Beam",    beam.Link(),       beam.LinkSigma());
    fitter.LinkVariable("Proton",  proton.Link(),     proton.LinkSigma());

    vector<string> photon_names;
    for(size_t i=0;i<nPhotons;i++) {
        stringstream s_photon;
        s_photon << "Photon" << (i+1);
        photon_names.push_back(s_photon.str());
        fitter.LinkVariable(s_photon.str(), photons[i].Link(), photons[i].LinkSigma());
    }
    vector<string> all_names = {"Beam", "Proton"};
    all_names.insert(all_names.end(),photon_names.begin(),photon_names.end());

    fitter.Settings().MaxIterations = 50;

    // Constraint: Incoming 4-vector = Outgoing 4-vector
    auto EnergyMomentumBalance = [] (const vector< vector<double> >& particles) -> vector<double>
    {
        const TLorentzVector target(0,0,0, ParticleTypeDatabase::Proton.Mass());
        // assume first particle is beam photon
        TLorentzVector diff = target + FitParticle::Make(particles[0], ParticleTypeDatabase::Photon.Mass());
        // assume second particle outgoing proton
        diff -= FitParticle::Make(particles[1], ParticleTypeDatabase::Proton.Mass());
        // subtract the rest, assumed to be photons
        for(size_t i=2;i<particles.size();i++) {
            diff -= FitParticle::Make(particles[i], ParticleTypeDatabase::Photon.Mass());
        }

        return {diff.X(), diff.Y(), diff.Z(), diff.T()};

    };
    fitter.AddConstraint("EnergyMomentumBalance", all_names, EnergyMomentumBalance);

    // Constraint: Invariant mass of nPhotons equals constant IM,
    // make lambda catch also this with [&] specification
    auto RequireIM = [&] (const vector< vector<double> >& photons) -> double
    {
        TLorentzVector sum(0,0,0,0);
        for(const auto& p : photons) {
            sum += FitParticle::Make(p, ParticleTypeDatabase::Photon.Mass());
        }
        return sum.M() - IM;
    };
    if(includeIMconstraint)
        fitter.AddConstraint("RequireIM", photon_names, RequireIM);

    // Constraint: Vertex position in z direction: v_z (positive if upstream)
    // if the photon originated from (0,0,v_z) instead of origin,
    // the corrected angle theta' is given by
    // tan(theta') = (R sin(theta))/(R cos(theta) - v_z)
    // R is the CB radius, 10in aka 25.4cm

    auto VertexConstraint = [&] (vector< vector<double> >& photons) -> double
    {
        constexpr double R = 25.4;
        // last element in photons is vz (scalar has dimension 1)
        // see AddConstraint below
        const double v_z = photons.back()[0];
        photons.resize(photons.size()-1); // get rid of last element
        // correct each photon's theta angle,
        // then calculate invariant mass of all photons
        TLorentzVector sum(0,0,0,0);
        for(auto& p : photons) {
            const double theta = p[1]; // second element is theta
            const double theta_p = std::atan2( R*sin(theta), R*cos(theta) - v_z);
            p[1] = theta_p;
            sum += FitParticle::Make(p, ParticleTypeDatabase::Photon.Mass());
        }
        return sum.M() - IM;
    };

    if(includeVertexFit) {
        fitter.AddUnmeasuredVariable("v_z"); // default value 0
        fitter.AddConstraint("VertexConstraint", photon_names + std::vector<string>{"v_z"}, VertexConstraint);
    }

    static_assert(!(includeIMconstraint && includeVertexFit), "Do not enable Vertex and IM Fit at the same time");

    // make fitter histograms
    chisquare   = hf.Make1D("ChiSqare","ChiSquare","#",chisqare_bins,"chisquare");
    probability = hf.Make1D("Probability","Probability","#",probability_bins,"probability");
    iterations = hf.Make1D("Number of iterations","Iterations","#",iterations_bins,"iterations");



    for(const auto& varname : fitter.VariableNames()) {
        pulls[varname] = hf.Make1D("Pull "+varname,
                                   "Pull", "#",
                                   pull_bins,
                                   "pull_"+varname);
    }

    stringstream ng;
    ng << nPhotons << "g";
    im_true = hf.Make1D("IM "+ng.str()+" true","IM","#",im_bins,"im_true");
    im_smeared = hf.Make1D("IM "+ng.str()+" smeared","IM","#",im_bins,"im_smeared");
    im_fit = hf.Make1D("IM "+ng.str()+" fit","IM","#",im_bins,"im_fit");

    vertex_z_before =  hf.Make1D("Vertex Z Before","v_z / cm","#",vertex_bins,"vertex_z_before");
    vertex_z_after =  hf.Make1D("Vertex Z After","v_z / cm","#",vertex_bins,"vertex_z_after");


    cout.precision(3);
    APLCON::PrintFormatting::Width = 11;
}


void ant::analysis::TestAPLCON::ProcessEvent(const ant::Event &event)
{
    for(auto& track : event.Tracks()) {
        banana->Fill(track->ClusterEnergy(), track->VetoEnergy());
    }

    for(auto& particle : event.Particles()) {
        particles->Fill(particle->Type().PrintName().c_str(), 1);
    }

    ntagged->Fill(event.TaggerHits().size());

    cbesum->Fill(event.Trigger().CBEenergySum());

    for( auto& t : ParticleTypeDatabase::DetectableTypes() ) {
        try {
            numParticleType.at(t)->Fill(event.ParticleType(*t).size());
        } catch (...) {}
    }

    for(const auto& taggerhit : event.TaggerHits()) {
        tagger->Fill(taggerhit->PhotonEnergy());

        // find the photons and one proton
        size_t foundPhotons = 0;
        for(const refMCParticle & p : event.MCTrue()) {
            if(p->Type() == ParticleTypeDatabase::Proton) {
                proton.SetFromVector(*p);
            }
            else if(foundPhotons<nPhotons && p->Type() == ParticleTypeDatabase::Photon) {
                photons[foundPhotons].SetFromVector(*p);
                foundPhotons++;
           }
        }
        if(foundPhotons != nPhotons)
            continue;

        beam.SetFromVector(taggerhit->PhotonBeam());

        FillIM(im_true, photons);

        // smear the MC true data
        proton.Smear();
        for(auto& photon : photons)
            photon.Smear();
        beam.Smear();

        FillIM(im_smeared, photons);

        // let APLCON do the work
        const APLCON::Result_t& result = fitter.DoFit();

        //cout << result << endl;

        if(result.Status != APLCON::Result_Status_t::Success) {
            //cout << result << endl;
            continue;
        }

        for(const auto& it_map : result.Variables) {
            const string& varname = it_map.first;
            const APLCON::Result_Variable_t& var = it_map.second;
            pulls.at(varname)->Fill(var.Pull);
        }
        chisquare->Fill(result.ChiSquare);
        probability->Fill(result.Probability);
        iterations->Fill(result.NIterations);

        if(includeVertexFit) {
            vertex_z_after->Fill(result.Variables.at("v_z").Value.After);
            vertex_z_before->Fill(result.Variables.at("v_z").Value.Before);
        }

        FillIM(im_fit, photons);
    }

}

void ant::analysis::TestAPLCON::Finish()
{

}

void ant::analysis::TestAPLCON::ShowResult()
{
    //canvas c("TestAPLCON: Overview");
    //c << drawoption("colz") << banana << particles << tagger << ntagged << cbesum << endc;

    canvas c_pulls("TestAPLCON: Pulls");
    for( auto& p : pulls ) {
        c_pulls << p.second;
    }
    c_pulls << endc;


    canvas c_fitter("TestAPLCON: Fitter");
    c_fitter << chisquare << probability << iterations
             << im_true << im_smeared << im_fit
             << vertex_z_before << vertex_z_after << endc;

}



