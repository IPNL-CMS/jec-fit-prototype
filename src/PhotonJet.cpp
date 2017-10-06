#include <PhotonJet.hpp>

#include <cmath>
#include <memory>

#include <TFile.h>
#include <TGraphErrors.h>

using namespace std::string_literals;


PhotonJet::PhotonJet(std::string const &fileName, Method method)
{
    std::string methodLabel;
    
    if (method == Method::PtBal)
        methodLabel = "PtBal";
    else if (method == Method::MPF)
        methodLabel = "MPF";
    
    TFile inputFile(fileName.c_str());
    
    if (inputFile.IsZombie())
    {
        std::ostringstream message;
        message << "Failed to open file \"" << fileName << "\".";
        throw std::runtime_error(message.str());
    }
    
    std::unique_ptr<TGraphErrors> extrapRatio(dynamic_cast<TGraphErrors *>(
      inputFile.Get(("resp_"s + methodLabel + "chs_extrap_a30_eta00_13").c_str())));
    
    inputFile.Close();
    
    
    bins.reserve(extrapRatio->GetN());
    
    for (int i = 0; i < extrapRatio->GetN(); ++i)
    {
        double x, y;
        extrapRatio->GetPoint(i, x, y);
        
        PtBin bin;
        bin.ptPhoton = x;
        bin.balanceRatio = y;
        bin.unc2 = pow(extrapRatio->GetErrorY(i), 2);
        
        bins.push_back(bin);
    }
}


unsigned PhotonJet::GetDim() const
{
    return bins.size();
}


double PhotonJet::Eval(JetCorrBase const &corrector, Nuisances const &nuisances) const
{
    double chi2 = 0.;
    
    for (auto const &bin: bins)
    {
        // Correct the balance ratio and photon pt for the potential offset in the photon pt scale
        double const balanceRatioCorr = bin.balanceRatio / (1 + nuisances.photonScale);
        double const ptPhoton = bin.ptPhoton * (1 + nuisances.photonScale);
        
        // Assume that pt of the jet is the same as pt of the photon
        chi2 += std::pow(balanceRatioCorr - 1 / corrector.Eval(ptPhoton), 2) / bin.unc2;
    }
    
    return chi2;
}
