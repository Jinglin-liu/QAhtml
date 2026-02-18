// Jennifer James <jennifer.l.james@vanderbilt.edu>, McKenna Sleeth, Derek Anderson, Mariia Mitrankova

#include "JetCstDrawer.h"

#include <qahtml/QADrawClient.h>

// ctor/dtor ==================================================================

// ----------------------------------------------------------------------------
//! Default ctor
// ----------------------------------------------------------------------------
JetCstDrawer::JetCstDrawer(const std::string& name,
                           const std::string& modu,
                           const std::string& type,
                           const std::string& pref,
                           const bool debug)
  : BaseJetDrawer(name, modu, type, pref, debug) {};

// ----------------------------------------------------------------------------
//! Default dtor
// ----------------------------------------------------------------------------
JetCstDrawer::~JetCstDrawer() {};

// inherited public methods ===================================================

// ----------------------------------------------------------------------------
//! Run histogram drawing for each trigger and resolution
// ----------------------------------------------------------------------------
int JetCstDrawer::Draw(const std::vector<uint32_t> vecTrigToDraw,
                       const std::vector<uint32_t> vecResToDraw)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Drawing component " << m_name << std::endl;
  }

  // loop over resolutions
  int nDraw = 0;
  for (const uint32_t resToDraw : vecResToDraw)
  {
    // add a row for resolution
    m_plots.AddRow();

    // now loop over triggers
    for (const uint32_t trigToDraw : vecTrigToDraw)
    {
      // add a column for trigger and draw hists
      m_plots.AddColumn();
      DoDrawing(trigToDraw, resToDraw);
      ++nDraw;
    }
  }
  return nDraw;
}

// ----------------------------------------------------------------------------
//! Generate HTML pages for each trigger and resolution
// ----------------------------------------------------------------------------
int JetCstDrawer::MakeHtml(const std::vector<uint32_t> vecTrigToDraw,
                           const std::vector<uint32_t> vecResToDraw,
                           const QADraw& subsystem)
{
  // emit debugging messages
  if (m_do_debug)
  {
    std::cout << "  -- Creating HTML pages for " << m_name << std::endl;
  }

  // instantiate draw client
  QADrawClient* cl = QADrawClient::instance();

  // loop over resolutions and triggers
  for (std::size_t iRes = 0; iRes < vecResToDraw.size(); ++iRes)
  {
    // now loop over triggers
    for (std::size_t iTrig = 0; iTrig < vecTrigToDraw.size(); ++iTrig)
    {
      // grab index & name of trigger being drawn
      const uint32_t idTrig = vecTrigToDraw[iTrig];
      const std::string nameTrig = JetDrawDefs::MapTrigToName().at(idTrig);

      // grab index & name of resolution being drawn
      const uint32_t idRes = vecResToDraw[iRes];
      const std::string nameRes = JetDrawDefs::MapResToName().at(idRes);
      const std::string dirRes = nameTrig + "/" + nameRes;
      const std::string fileRes = nameTrig + "_" + nameRes;

      // make html pages
      for (const auto& plot : m_plots.GetVecPlotPads(iRes, iTrig))
      {
        const std::string name = plot.canvas->GetName();
        const std::string dir = dirRes + "/Constituents/" + name;
        const std::string png = cl->htmlRegisterPage(subsystem, dir, name, "png");
        cl->CanvasToPng(plot.canvas, png);
      }
    }
  }
  return 0;
}

// inherited private methods ==================================================

// ----------------------------------------------------------------------------
//! Draw histograms for a given trigger and resolution
// ----------------------------------------------------------------------------
void JetCstDrawer::DoDrawing(const uint32_t trig, const uint32_t res)
{
  // emit debugging message
  if (m_do_debug)
  {
    switch (res)
    {
      case JetDrawDefs::JetRes::R02:
        std::cout << "  -- Drawing constituent histograms (trig = " << trig << ", R = 0.2)" << std::endl;
        break;
      case JetDrawDefs::JetRes::R03:
        std::cout << "  -- Drawing constituent histograms (trig = " << trig << ", R = 0.3)" << std::endl;
        break;
      case JetDrawDefs::JetRes::R04:
        std::cout << "  -- Drawing constituent histograms (trig = " << trig << ", R = 0.4)" << std::endl;
        break;
      case JetDrawDefs::JetRes::R05:
        std::cout << "  -- Drawing constituent histograms (trig = " << trig << ", R = 0.5)" << std::endl;
        break;
      default:
        std::cerr << "Warning: trying to constituent histograms for unknown resolution" << std::endl;
        return;
    }
  }

  // for hist names
  const std::string histName = m_hist_prefix
                             + "_"
                             + JetDrawDefs::MapTrigToTag().at(trig)
                             + "_"
                             + m_jet_type
                             + "_"
                             + JetDrawDefs::MapResToTag().at(res);

  // connect to draw client
  QADrawClient* cl = QADrawClient::instance();

  // grab histograms to draw and set options
  JetDrawDefs::VHistAndOpts1D hists = {
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ncsts_cemc")),
      "Jet N Constituents in CEMC",
      "N_{CEMC} Constituents",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ncsts_ihcal")),
      "Jet N Constituents in IHCal",
      "N_{IHCal} Constituents",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ncsts_ohcal")),
      "Jet N Constituents in OHCal",
      "N_{OHCal} Constituents",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ncsts_total")),
      "Jet N Constituents",
      "N_{Total} Constituents",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_ncstsvscalolayer")),
      "Jet N Constituents vs Calo Layer",
      "Calo Layer",
      "N Constituents",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_efracjet_cemc")),
      "Jet E Fraction in CEMC",
      "Jet E_{CEMC} Fraction",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_efracjet_ihcal")),
      "Jet E Fraction in IHCal",
      "Jet E_{IHCal} Fraction",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_efracjet_ohcal")),
      "Jet E Fraction in OHCal",
      "Jet E_{OHcal} Fraction",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_efracjetvscalolayer")),
      "Jet E Fraction vs Calo Layer",
      "Calo Layer",
      "Jet E Fraction",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    }
  };

  // reference histograms, using same index as hists
  auto refs = BuildRefHists(hists);
  std::string currRunMsg = "Current Run " + std::to_string(cl->RunNumber());
  std::string refRunMsg  = "Reference Run " + refRunNum;

  // draw 1d ncst hists on a page
  DrawHists("JetCsts_NCsts", {0, 1, 2, 3}, hists, trig, res);

  // draw e fraction reference hists on relevant pads
  DrawHistOnPad(0, 1, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(0, 1, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  DrawHistOnPad(1, 2, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(1, 2, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(2, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(2, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  DrawHistOnPad(2, 3, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(2, 3, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  DrawHistOnPad(3, 4, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(3, 4, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(4, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(4, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  // draw e fraction on a page
  DrawHists("JetCsts_EFrac", {5, 6, 7}, hists, trig, res);

  // draw e fraction reference hists on relevant pads
  DrawHistOnPad(5, 1, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(5, 1, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  DrawHistOnPad(6, 2, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(6, 2, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(2, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(2, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  DrawHistOnPad(7, 3, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(7, 3, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  // draw ncst and e faction vs. calo layer on a page
  DrawHists("JetCsts_VsCaloLayer", {4, 8}, hists, trig, res);
  return;
}
