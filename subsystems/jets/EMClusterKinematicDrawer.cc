// Jennifer James <jennifer.l.james@vanderbilt.edu>, McKenna Sleeth, Derek Anderson, Mariia Mitrankova

#include "EMClusterKinematicDrawer.h"

#include <qahtml/QADrawClient.h>

// ctor/dtor ==================================================================

// ----------------------------------------------------------------------------
//! Default ctor
// ----------------------------------------------------------------------------
EMClusterKinematicDrawer::EMClusterKinematicDrawer(const std::string& name,
                                             const std::string& modu,
                                             const std::string& type,
                                             const std::string& pref,
                                             const bool debug)
  : BaseJetDrawer(name, modu, type, pref, debug) {};

// ----------------------------------------------------------------------------
//! Default dtor
// ----------------------------------------------------------------------------
EMClusterKinematicDrawer::~EMClusterKinematicDrawer() {};

// inherited public methods ===================================================

// ----------------------------------------------------------------------------
//! Run histogram drawing for each trigger
// ----------------------------------------------------------------------------
int EMClusterKinematicDrawer::Draw(const std::vector<uint32_t> vecTrigToDraw,
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
int EMClusterKinematicDrawer::MakeHtml(const std::vector<uint32_t> vecTrigToDraw,
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
void EMClusterKinematicDrawer::DoDrawing(const uint32_t trig, const uint32_t /*res*/)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Drawing rho histograms (trig = " << trig << ")" << std::endl;
  }

  // for histogram names
  const std::string histName = m_hist_prefix
                             + "_"
                             + JetDrawDefs::MapTrigToTag().at(trig)
                             + "_"
                             + m_jet_type;  // n.b. this should be the cluster type

  // connect to draw client
  QADrawClient* cl = QADrawClient::instance();

  // grab histogram do draw and set options
  JetDrawDefs::VHistAndOpts1D hists = {
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_chi2")),
      "Cluster #chi^{2}",
      "#chi^{2}",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_chi2_energy")),
      "Cluster #chi^{2} vs. E_{T}",
      "E_{T} [GeV]",
      "#chi^{2}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_chi2_eta")),
      "Cluster #chi^{2} vs. #eta",
      "#eta",
      "#chi^{2}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_chi2_phi")),
      "Cluster #chi^{2} vs. #varphi",
      "#varphi [rad.]",
      "#chi^{2}",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_energy")),
      "Cluster E_{T}",
      "E_{T} [GeV]",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_energy_eta")),
      "Cluster E_{T} vs. #eta",
      "#eta",
      "E_{T} [GeV]",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_energy_phi")),
      "Cluster E_{T} vs. #varphi",
      "#varphi [rad.]",
      "E_{T} [GeV]",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_eta_phi_with_cuts")),
      "Cluster #varphi vs. #eta (after #chi^{2} and E_{T} cuts)",
      "#eta",
      "#varphi [rad.]",
      "Counts",
      0.8,
      0.25,
      false,
      true,
      false
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_eta_with_cuts")),
      "Cluster #eta (after #chi^{2} and E_{T} cuts)",
      "#eta",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    },
    {
      dynamic_cast<TH1*>(cl->getHisto(histName + "_phi_with_cuts")),
      "Cluster #varphi (after #chi^{2} and E_{T} cuts)",
      "#varphi [rad.]",
      "Normalized Counts",
      "",
      0.8,
      0.25,
      true,
      false,
      true
    }
  };

  // reference histograms, using same index as hists
  auto refs = BuildRefHists(hists);
  std::string currRunMsg = "Current Run " + std::to_string(cl->RunNumber());
  std::string refRunMsg  = "Reference Run " + refRunNum;

  // draw chi2 plots on one page
  DrawHists("EMClusterKinematics_Chi2", {0, 1, 2, 3}, hists, trig);

  // draw reference hists on relevant pads
  DrawHistOnPad(0, 1, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(0, 1, hists, m_plots.GetBackPlotPad()); //draw current run on top of ref
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  // draw et plots on one page
  DrawHists("EMClusterKinematics_Energy", {5, 6, 4}, hists, trig);

  // draw reference hists on relevant pads
  DrawHistOnPad(4, 3, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(4, 3, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(3, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  // draw eta/phi plots on one page
  DrawHists("EMClusterKinematics_EtaPhi", {8, 9, 7}, hists, trig);

  // draw reference hists on relevant pads
  DrawHistOnPad(8, 1, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(8, 1, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(1, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);
  
  DrawHistOnPad(9, 2, refs, m_plots.GetBackPlotPad());
  DrawHistOnPad(9, 2, hists, m_plots.GetBackPlotPad());
  DrawTextOnPad(2, m_plots.GetBackPlotPad(), 0.60, 0.80, kBlack, currRunMsg);
  DrawTextOnPad(2, m_plots.GetBackPlotPad(), 0.60, 0.75, kRed, refRunMsg);

  return;
}
