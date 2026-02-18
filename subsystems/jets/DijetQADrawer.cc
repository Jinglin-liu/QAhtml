// Jennifer James <jennifer.l.james@vanderbilt.edu>, McKenna Sleeth, Derek Anderson, Mariia Mitrankova

#include "DijetQADrawer.h"

#include <qahtml/QADrawClient.h>

// ctor/dtor ==================================================================

// ----------------------------------------------------------------------------
//! Default ctor
// ----------------------------------------------------------------------------
DijetQADrawer::DijetQADrawer(const std::string& name,
                             const std::string& modu,
                             const std::string& type,
                             const std::string& pref,
                             const bool debug)
  : BaseJetDrawer(name, modu, type, pref, debug) {};

// ----------------------------------------------------------------------------
//! Default dtor
// ----------------------------------------------------------------------------
DijetQADrawer::~DijetQADrawer() {};

// inherited public methods ===================================================

// ----------------------------------------------------------------------------
//! Run histogram drawing for each trigger and resolution
// ----------------------------------------------------------------------------
int DijetQADrawer::Draw(const std::vector<uint32_t> vecTrigToDraw,
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
int DijetQADrawer::MakeHtml(const std::vector<uint32_t> vecTrigToDraw,
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
        const std::string dir = dirRes + "/Dijets/" + name;
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
void DijetQADrawer::DoDrawing(const uint32_t trig, const uint32_t res)
{
  // emit debugging message
  if (m_do_debug)
  {
    switch (res)
    {
      case JetDrawDefs::JetRes::R02:
        std::cout << "  -- Drawing dijet histograms (trig = " << trig << ", R = 0.2)" << std::endl;
        break;
      case JetDrawDefs::JetRes::R03:
        std::cout << "  -- Drawing dijet histograms (trig = " << trig << ", R = 0.3)" << std::endl;
        break;
      case JetDrawDefs::JetRes::R04:
        std::cout << "  -- Drawing dijet histograms (trig = " << trig << ", R = 0.4)" << std::endl;
        break;
      case JetDrawDefs::JetRes::R05:
        std::cout << "  -- Drawing dijet histograms (trig = " << trig << ", R = 0.5)" << std::endl;
        break;
      default:
        std::cerr << "Warning: trying to draw dijet histograms with Unknown resolution" << std::endl;
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
      dynamic_cast<TH1*>(cl->getHisto(histName + "_Ajj")),
      "A_{jj} for all jet pairs",
      "A_{jj}",
      "Normalized counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_Ajj_l")),
      "A_{jj} for leading jet pair in event",
      "A_{jj}",
      "Normalized counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_Ajj_pt")),
      "A_{jj} vs. p_{T}^{jet,lead} for all jet pairs",
      "p_{T}^{jet,lead} [GeV]",
      "A_{jj}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_Ajj_pt_l")),
      "A_{jj} vs. p_{T}^{jet,lead} for leading jet pair in event",
      "p_{T}^{jet,lead} [GeV]",
      "A_{jj}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_dphi")),
      "|#Delta#varphi| for all jet pairs",
      "|#Delta#varphi| [rad.]",
      "Normalized counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_dphi_Ajj")),
      "A_{jj} vs. |#Delta#varphi| for all jet pairs",
      "|#Delta#varphi| [rad.]",
      "A_{jj}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_dphi_Ajj_l")),
      "A_{jj} vs. |#Delta#varphi| for leading jet pair in event",
      "|#Delta#varphi| [rad.]",
      "A_{jj}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_dphi_l")),
      "|#Delta#varphi| for leading jet pair in event",
      "|#Delta#varphi| [rad.]",
      "Normalized counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_dphi_pt")),
      "|#Delta#varphi| vs. p_{T}^{jet,lead} for all jet pairs",
      "p_{T}^{jet,lead} [GeV]",
      "|#Delta#varphi| [rad.]",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_dphi_pt_l")),
      "|#Delta#varphi| vs. p_{T}^{jet,lead} for leading jet pair",
      "p_{T}^{jet,lead} [GeV]",
      "|#Delta#varphi [rad.]",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_xj")),
      "x_{j} for all jet pairs",
      "x_{j}",
      "Normalized counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_xj_l")),
      "x_{j} for leading jet pair",
      "x_{j}",
      "Normalized counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_xj_pt")),
      "x_{j} vs. p_{T}^{jet,lead} for all jet pairs",
      "p_{T}^{jet,lead} [GeV]",
      "x_{j}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_xj_pt_l")),
      "x_{j} vs. p_{T}^{jet,lead} for leading jet pair",
      "p_{T}^{jet,lead} [GeV]",
      "x_{j}",
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

  // draw all pair ajj and xj hists on one page
  DrawHists("DijetAllPair_AjjAndXj", {0, 2, 10, 12}, hists, trig, res);

  // draw reference hists on relevant pads
  DrawHistOnPad( 0, 1, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad( 0, 1, hists, m_plots.GetBackPlotPad()); //draw current run on top of ref
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.40, kBlack, currRunMsg);
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.35, kRed, refRunMsg);

  DrawHistOnPad(10, 3, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(10, 3, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  // draw lead pair ajj and xj hists on one page
  DrawHists("DijetLeadPair_AjjAndXj", {1, 3, 11, 13}, hists, trig, res);

  // draw reference hists on relevant pads
  DrawHistOnPad( 1, 1, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad( 1, 1, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.40, kBlack, currRunMsg);
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.35, kRed, refRunMsg);

  DrawHistOnPad(11, 3, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(11, 3, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);
  
  // draw all pair dphi on one page
  DrawHists("DijetAllPair_DeltaPhi", {4, 5, 8}, hists, trig, res);

  // draw reference hists on relevant pads
  DrawHistOnPad(4, 1, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(4, 1, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.40, kBlack, currRunMsg);
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.35, kRed, refRunMsg);
  
  // draw lead pair dphi on one page
  DrawHists("DijetLeadPair_DeltaPhi", {7, 6, 9}, hists, trig, res);

  // draw reference hists on relevant pads
  DrawHistOnPad(7, 1, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(7, 1, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.40, kBlack, currRunMsg);
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.35, kRed, refRunMsg);

  return;
}
