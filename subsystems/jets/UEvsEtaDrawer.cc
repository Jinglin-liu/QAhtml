// Jennifer James <jennifer.l.james@vanderbilt.edu>, McKenna Sleeth, Derek Anderson, Mariia Mitrankova, Jinglin Liu

#include "UEvsEtaDrawer.h"
#include "JetDrawDefs.h"
#include <qahtml/QADrawClient.h>

// ctor/dtor ==================================================================

// ----------------------------------------------------------------------------
//! Default ctor
// ----------------------------------------------------------------------------
UEvsEtaDrawer::UEvsEtaDrawer(const std::string& name,
                               const std::string& modu,
                               const std::string& type,
                               const std::string& pref,
                               const bool debug)
  : BaseJetDrawer(name, modu, type, pref, debug) {};

// ----------------------------------------------------------------------------
//! Default dtor
// ----------------------------------------------------------------------------
UEvsEtaDrawer::~UEvsEtaDrawer() {};

// inherited public methods ===================================================

// ----------------------------------------------------------------------------
//! Run histogram drawing for each trigger
// ----------------------------------------------------------------------------
int UEvsEtaDrawer::Draw(const std::vector<uint32_t> vecTrigToDraw,
                         const std::vector<uint32_t> /*vecResToDraw*/)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Drawing component " << m_name << std::endl;
  }

  // create 1 row to hold trigger vectors
  m_plots.AddRow();

  // loop over triggers
  int nDraw = 0;
  for (const uint32_t trigToDraw : vecTrigToDraw)
  {
    m_plots.AddColumn();
    DoDrawing(trigToDraw, 0);
    ++nDraw;
  }
  return nDraw;
}

// ----------------------------------------------------------------------------
//! Generate HTML pages for each trigger
// ----------------------------------------------------------------------------
int UEvsEtaDrawer::MakeHtml(const std::vector<uint32_t> vecTrigToDraw,
                             const std::vector<uint32_t> /*vecResToDraw*/,
                             const QADraw& subsystem)
{
  // emit debugging messages
  if (m_do_debug)
  {
    std::cout << "  -- Creating HTML pages for " << m_name << std::endl;
  }

  // instantiate draw client
  QADrawClient* cl = QADrawClient::instance();

  // loop over triggers to draw
  for (std::size_t iTrig = 0; iTrig < vecTrigToDraw.size(); ++iTrig)
  {
    // grab index & name of trigger being drawn
    const uint32_t idTrig = vecTrigToDraw[iTrig];
    const std::string nameTrig = JetDrawDefs::MapTrigToName().at(idTrig);

    // make html pages
    for (const auto& plot : m_plots.GetVecPlotPads(0, iTrig))
    {
      const std::string name = plot.canvas->GetName();
      const std::string dir = nameTrig + "/" + name;
      const std::string png = cl->htmlRegisterPage(subsystem, dir, name, "png");
      cl->CanvasToPng(plot.canvas, png);
    }
  }

  // return w/o error
  return 0;
}

// inherited private methods ==================================================

// ----------------------------------------------------------------------------
//! Draw histograms for a given trigger
// ----------------------------------------------------------------------------
void UEvsEtaDrawer::DoDrawing(const uint32_t trig, const uint32_t /*res*/)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Drawing UE vs eta histograms (trig = " << trig << ")" << std::endl;
  }

  // for histogram names
  const std::string histName = m_hist_prefix
                             + "_"
                             + JetDrawDefs::MapTrigToTag().at(trig);

  // connect to draw client
  QADrawClient* cl = QADrawClient::instance();

  // grab histograms to draw and set options
  JetDrawDefs::VHistAndOpts1D hists = {
    // v2, psi2
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_v2_cent")),
      "v_{2} in centrality bins",
      "Centrality",
      "v_{2}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_psi2_cent")),
      "#Psi_{2} in centrality bins",
      "Centrality",
      "#Psi_{2}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    // emcal
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_emcaleta_cent0_20")),
      "UE vs. #eta, centrality (0, 20]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_emcaleta_cent20_50")),
      "UE vs. #eta, centrality (20, 50]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_emcaleta_cent50_100")),
      "UE vs. #eta, centrality (50, 100]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    // ihcal
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ihcaleta_cent0_20")),
      "UE vs. #eta, centrality (0, 20]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ihcaleta_cent20_50")),
      "UE vs. #eta, centrality (20, 50]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ihcaleta_cent50_100")),
      "UE vs. #eta, centrality (50, 100]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    // ohcal
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ohcaleta_cent0_20")),
      "UE vs. #eta, centrality (0, 20]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ohcaleta_cent20_50")),
      "UE vs. #eta, centrality (20, 50]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ohcaleta_cent50_100")),
      "UE vs. #eta, centrality (50, 100]",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    // all centrality plots
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_emcaleta")),
      "UE vs. #eta, EMCal",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ihcaleta")),
      "UE vs. #eta, IHCal",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ohcaleta")),
      "UE vs. #eta, OHCal",
      "#eta",
      "Underlying Event",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    }
  };

  /* Drop centrality dependent plots until centrality info is fixed
  // draw v2 psi2 plots on one page
  DrawHists("UEvsEta_v2psi2", {0, 1}, hists, trig);

  // draw emcal plots on one page
  DrawHists("UEvsEta_EMCal", {2, 3, 4}, hists, trig);

  // draw ihcal plots on one page
  DrawHists("UEvsEta_IHCal", {5, 6, 7}, hists, trig);

  // draw ohcal plots on one page
  DrawHists("UEvsEta_OHCal", {8, 9, 10}, hists, trig);
  */

  // draw all centrality plots on one page
  DrawHists("UEvsEta_AllCent", {11, 12, 13}, hists, trig);

  return;
}
