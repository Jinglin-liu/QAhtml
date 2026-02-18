// Jennifer James <jennifer.l.james@vanderbilt.edu>, McKenna Sleeth, Derek Anderson, Mariia Mitrankova

#include "BaseJetDrawer.h"

#include <qahtml/QADraw.h>
#include <qahtml/QADrawClient.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TLatex.h>
#include <TPad.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TText.h>
#include <iostream>
#include <sstream>

// ctor =======================================================================

// ----------------------------------------------------------------------------
//! Base component constructor
// ----------------------------------------------------------------------------
/*! Default constructor to initialize base
 *  Jet QA component. All argumentss are
 *  technically optional. 
 *
 *    \param name  name of component (e.g. "EventWiseRho")
 *    \param modu  name of QAhtml module (e.g. "JetDraw")
 *    \param type  type of jet being drawn, part of the histogram names
 *    \param pref  prefix of histograms being drawn,
 *    \param debug turn debugging on/off
 *    \param local turn local mode on/off
 */
BaseJetDrawer::BaseJetDrawer(const std::string& name,
                             const std::string& modu,
                             const std::string& type,
                             const std::string& pref,
                             const bool debug,
                             const bool local)
  : m_name(name)
  , m_module(modu)
  , m_jet_type(type)
  , m_hist_prefix(pref)
  , m_do_debug(debug)
  , m_do_local(local)
{}

// public methods to be implemented ===========================================

// ----------------------------------------------------------------------------
//! Run drawing for specified triggers and resolution parameters
// ----------------------------------------------------------------------------
/*! This method must be implemented by derived components.
 *  It should code how to loop over the provided the trigger
 *  and resolution indices and run the histogram drawing for
 *  the relevant combinations.
 */
int BaseJetDrawer::Draw(const std::vector<uint32_t> /*vecTrigToDraw*/,
                        const std::vector<uint32_t> /*vecResToDraw*/)
{
  return 0;
}

// ----------------------------------------------------------------------------
//! Make HTML pages for component
// ----------------------------------------------------------------------------
/*! This method must be implemented by derived components.
 *  It should code how to generate the relevant html pages
 *  from accumulated plots in `m_plots`.
 */
int BaseJetDrawer::MakeHtml(const std::vector<uint32_t> /*vecTrigToDraw*/,
                            const std::vector<uint32_t> /*vecResToDraw*/,
                            const QADraw& /*subsystem*/)
{
  return 0;
}

// other public methods =======================================================

// ----------------------------------------------------------------------------
//! Save plots to file
// ----------------------------------------------------------------------------
/*! Helper method to save all plots to a specified file.
 *  This is useful for debugging and quick testing when
 *  adjusting plotting details/etc.
 *
 *  \param[out] file file to write plots to
 */
void BaseJetDrawer::SaveToFile(TFile* file)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Saving plots to file:\n"
              << "       component = " << m_name   << "\n"
              << "       module    = " << m_module << "\n"
              << "       file      = " << file->GetName()
              << std::endl;
  }

  // check if you can cd into file
  //   - if not, exit
  const bool isGoodCD = file->cd();
  if (!isGoodCD)
  {
    if (m_do_debug)
    {
      std::cerr << PHWHERE << "WARNING: couldn't cd into output file!" << std::endl;
    }
    return;
  }

  // save & exit 
  std::size_t nWrite = m_plots.Save();
  if (m_do_debug)
  {
    std::cout << "  -- Saved " << m_name << " plots:\n"
              << "       "     << nWrite << " plots written."
              << std::endl;
  }
  return;
}

// protected methods to be implemented ========================================

// ----------------------------------------------------------------------------
//! Do histogram drawing for a specific trigger + jet resolution combination
// ----------------------------------------------------------------------------
/*! This method must be implemented by derived components.
 *  It should code how to actually draw histograms for a
 *  specific combination of trigger index and resolution
 *  index.
 */
void BaseJetDrawer::DoDrawing(const uint32_t /*trig*/, const uint32_t /*res*/)
{
  return;
}

// other protected methods ====================================================

// ----------------------------------------------------------------------------
//! Draw run and build info on a TPad
// ----------------------------------------------------------------------------
/*! By default, trigger and resolution aren't added to the
 *  text.  However, if trigger or resolution indices are
 *  provided, then that info will be added.
 *
 *  \param[in]  what the pad's associated QA component (e.g. JetKinematics)
 *  \param[out] pad  the pad to draw text on
 *  \param      trig trigger index (optional)
 *  \param      res  jet resolution index (optional)
 */
void BaseJetDrawer::DrawRunAndBuild(const std::string& what,
                                    TPad* pad,
                                    const int trig,
                                    const int res)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Drawing run and build info for " << what << std::endl;
  }

  // connect to draw client
  QADrawClient* cl = QADrawClient::instance();

  // Get build and dbtag from production info histogram
  TH1I* prodHist = dynamic_cast<TH1I*>(cl->getHisto("h_QAHistManagerDef_ProductionInfo"));
  std::string prodInfo = prodHist->GetTitle();

  // Generate run string from client
  std::ostringstream runnostream;
  runnostream << cl->RunNumber() << ", " << prodInfo;

  // prepend module name, component, and other info as needed
  std::string runstring = m_name;
  //runstring.append("_" + what); // this is redundant, keeping in case needed
  if (trig > -1)
  {
    runstring.append("_" + JetDrawDefs::MapTrigToName().at(trig));
  }
  if (res > -1)
  {
    runstring.append("_" + JetDrawDefs::MapResToName().at(res));
  }

  // now add run
  runstring += ", Run ";
  runstring += runnostream.str();

  // create TText for info
  TText PrintRun;
  PrintRun.SetTextFont(62);
  PrintRun.SetTextSize(0.25);
  PrintRun.SetNDC();          // set to normalized coordinates
  PrintRun.SetTextAlign(22);  // center/center alignment

  // and finally draw on pad
  pad->cd();
  PrintRun.DrawText(0.50, 0.70, runstring.data());
}

// ----------------------------------------------------------------------------
//! Draw histograms on canvas
// ----------------------------------------------------------------------------
/*! By default, trigger and resolution aren't added to the
 *  canvas names or run/build text. However, if trigger or
 *  resolution indices are provided, then that info will
 *  be added.
 *
 *  \param what    the histograms' associated QA component (e.g. JetSeed)
 *  \param indices the indices of the histograms to draw
 *  \param hists   the histograms to select from
 *  \param trig    trigger index (optional)
 *  \param res     jet resolution index (optional)
 */
void BaseJetDrawer::DrawHists(const std::string& what,
                              const std::vector<std::size_t>& indices,
                              const JetDrawDefs::VHistAndOpts1D& hists,
                              const int trig,
                              const int res)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Drawing histograms for " << what << std::endl;
  }

  // form canvas name
  std::string canName = what;
  if (trig > -1)
  {
    canName.append("_" + JetDrawDefs::MapTrigToTag().at(trig));
  }
  if (res > -1)
  {
    canName.append("_" + JetDrawDefs::MapResToTag().at(res));
  }

  // if canvas doesn't exist yet, create it
  if (!gROOT->FindObject(canName.data()))
  {
    MakeCanvas(canName, indices.size());
  }

  // draw selected histograms
  //   - n.b. COLZ does nothing for 1D histograms
  for (std::size_t iPad = 0; iPad < indices.size(); ++iPad)
  {
    m_plots.GetBackPlotPad().histPad->cd(iPad + 1);
    if (hists.at(indices[iPad]).hist)
    {
      // apply global style ONCE each pad
      gStyle->SetOptTitle(1);
      gROOT->ForceStyle();
      gPad->UseCurrentStyle();
      gPad->Update();

      UpdatePadStyle(hists.at(indices[iPad]));
      hists.at(indices[iPad]).hist->DrawCopy("COLZ");
    }
    else
    {
      DrawEmptyHistogram(hists.at(indices[iPad]).title);
    }
  }

  // add run/build info to canvas
  DrawRunAndBuild(what, m_plots.GetBackPlotPad().runPad, trig, res);
  m_plots.GetBackPlotPad().canvas->Update();
}

// ----------------------------------------------------------------------------
//! Draw a histogram on a pad
// ----------------------------------------------------------------------------
/*! Draw a particular histogram (entry iHist in provided hist vector)
 *  on a particular pad (pad iPad in canvas). Note that
 *    1. the indices of pads in a TCanvas from 1 (not 0!) on up, and
 *    2. that this function assumes something has already been drawn
 *        on the pad.
 *
 *  \param[in]  iHist index of histogram in vector `hists` to draw
 *  \param[in]  iPad  index of pad to draw histogram on
 *  \param[in]  hists vector histograms containing histogram to be drawn
 *  \param[out] plot  canvas containing pad to be drawn on
 */
void BaseJetDrawer::DrawHistOnPad(const std::size_t iHist,
                                  const std::size_t iPad,
                                  const JetDrawDefs::VHistAndOpts1D& hists,
                                  JetDrawDefs::PlotPads& plot)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Drawing histogram " << iHist << " on pad " << iPad << std::endl;
  }

  // draw histogram
  plot.histPad->cd(iPad);
  if (hists.at(iHist).hist)
  {
    // update pad style and draw
    UpdatePadStyle(hists.at(iHist));
    hists[iHist].hist->DrawCopy("SAME");
  }
  else
  {
    std::cerr << "Warning: trying to draw missing histogram " << iHist << " on pad " << iPad << std::endl;
  }
  plot.canvas->Update();
}

// ----------------------------------------------------------------------------
//! Draw empty histogram on current pad
// ----------------------------------------------------------------------------
/*! Helper function to draw an empty histogram on the current
 *  pad. Used when a histogram is missing.
 *
 *  \param[in] what what's missing (e.g. a histogram)
 */
void BaseJetDrawer::DrawEmptyHistogram(const std::string& what)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Drawing empty message for '" << what << "'" << std::endl;
  }

  // make empty hist name
  std::string name(gPad->GetName());
  name += "_is_missing";

  // set up hist/text
  TH1D* hEmpty = new TH1D(name.data(), "", 10, 0, 10);
  TLatex* lEmpty = new TLatex();

  // set up message
  const std::string message = what + " is missing";

  // and draw them on current pad
  hEmpty->DrawCopy();
  lEmpty->DrawLatex(0.3, 0.5, message.data());
}

// ----------------------------------------------------------------------------
//! Create canvas to draw on
// ----------------------------------------------------------------------------
/*! Creates a TCanvas to hold QA histograms and run info.
 *
 *  \param name  name of canvas
 *  \param nhist number of histograms to draw
 */ 
void BaseJetDrawer::MakeCanvas(const std::string& name, const int nHist)
{
  // emit debugging message
  if (m_do_debug)
  {
    std::cout << "  -- Making canvas " << name << std::endl;
  }

  // instantiate draw client & grab display size
  QADrawClient* cl = QADrawClient::instance();
  int xsize = cl->GetDisplaySizeX();
  int ysize = cl->GetDisplaySizeY();

  // create canvas
  //   - n.b. xpos (-1) negative means do not draw menu bar
  TCanvas* canvas;
  if (m_do_local)
  {
    canvas = new TCanvas(name.data(), "", 950, 950);
  }
  else
  {
    canvas = new TCanvas(name.data(), "", -1, 0, (int) (xsize / 1.2), (int) (ysize / 1.2));
  }
  canvas->UseCurrentStyle();
  gSystem->ProcessEvents();

  // create pad for histograms
  const std::string histPadName = name + "_hist";
  TPad* histPad = new TPad(histPadName.data(), "for histograms", 0.0, 0.0, 1.0, 0.9);
  histPad->SetFillStyle(4000);
  canvas->cd();
  histPad->Draw();

  // divide hist pad into appropriate no. of pads
  const int nRow = std::min(nHist, 2);
  const int nCol = (nHist / 2) + (nHist % 2);
  const float bRow = 0.01;
  const float bCol = 0.01;
  if (nHist > 1)
  {
    histPad->Divide(nRow, nCol, bRow, bCol);
  }

  // create pad for run number
  const std::string runPadName = name + "_run";
  TPad* runPad = new TPad(runPadName.data(), "for run and build", 0.0, 0.9, 1.0, 1.0);
  runPad->SetFillStyle(4000);
  canvas->cd();
  runPad->Draw();

  // add canvas/pads to matrix
  m_plots.AppendPlotPad( {canvas, histPad, runPad} );

  // return w/o error
  return;
}

// ----------------------------------------------------------------------------
//! Update style of current pad based on options
// ----------------------------------------------------------------------------
void BaseJetDrawer::UpdatePadStyle(const JetDrawDefs::HistAndOpts& hist)
{
  gPad->SetRightMargin(hist.margin);
  gPad->SetLogy(hist.logy);
  gPad->SetLogz(hist.logz);

  //Histogram title settings
  UpdateTitle(hist);
  gPad->SetTopMargin(0.14); //make place for titles
  gStyle->SetTitleAlign(13);
  gStyle->SetTitleX(0.01);
  gStyle->SetTitleY(0.99);

  hist.hist->SetMarkerSize(hist.marker);

  NormalizeHist(hist);
}

// ----------------------------------------------------------------------------
//! Update histogram and axis titles based on options
// ----------------------------------------------------------------------------
void BaseJetDrawer::UpdateTitle(const JetDrawDefs::HistAndOpts& hist)
{
  // set histogram and axis titles to those in each drawer,
  // otherwise keep original titles from coresoftware
  const bool doQAtitle = true; 
  if (hist.hist && doQAtitle)
  {
    // make hist title
    const std::string title = hist.title
                            + ";"
                            + hist.titlex
                            + ";"
                            + hist.titley
                            + ";"
                            + hist.titlez;

    hist.hist->SetTitle(title.data());
  }
}

// ----------------------------------------------------------------------------
//! Normalized histogram based on options
// ----------------------------------------------------------------------------
void BaseJetDrawer::NormalizeHist(const JetDrawDefs::HistAndOpts& hist)
{
  const bool doNorm = hist.norm;
  if (hist.hist && hist.hist->Integral() != 0 && doNorm)
  {
      hist.hist->Scale(1.0 / hist.hist->Integral());
  }
}

// ----------------------------------------------------------------------------
//! Build reference histograms
// ----------------------------------------------------------------------------
JetDrawDefs::VHistAndOpts1D BaseJetDrawer::BuildRefHists(const JetDrawDefs::VHistAndOpts1D& hists)
{
  // connect to draw client, get run number
  QADrawClient* cl = QADrawClient::instance();
  int runNum = cl->RunNumber();
  const bool is_pp = JetDrawDefs::IsPP(runNum);

  if (m_do_debug)
  {
    std::cout << std::boolalpha
              << "  -- Reading Reference File\n" 
              << "  -- Drawers: run is pp? : " << is_pp 
              << std::endl;
  }

  // get reference run info according to run number
  JetDrawDefs::RefRunInfo ref = JetDrawDefs::GetRefRunInfo(runNum);
  refFilePath = ref.file;
  refRunNum = ref.run;

  // read reference file 
  TFile *refFile = TFile::Open(refFilePath.c_str(), "READ");
  if (!refFile || refFile->IsZombie()) {
    std::cerr << "Error: Could not open reference file: " << refFilePath << std::endl;
    return {};
  }

  // reference histograms
  JetDrawDefs::VHistAndOpts1D refs(hists.size());
  for (size_t i = 0; i < hists.size(); i++)
  {
    const auto &ho = hists[i];
    // skip empty and non 1D histograms
    if (!ho.hist || ho.hist->GetDimension() != 1) continue;

    TH1* refh_tmp = dynamic_cast<TH1*>(refFile->Get(ho.hist->GetName()));
    if (!refh_tmp) continue;

    TH1* refh = (TH1*)refh_tmp->Clone();
    refh->SetDirectory(nullptr);  // detach from TFile

    refs[i] = JetDrawDefs::HistAndOpts{
      refh,
      ho.title,
      ho.titlex,
      ho.titley,
      ho.titlez,
      1.0, //ho.marker, smaller marker size
      ho.margin,
      ho.logy,
      ho.logz,
      ho.norm
    };

    // set reference hists style
    refs[i].hist->SetMarkerColor(kRed);
    refs[i].hist->SetLineColor(kRed);
  }
  
  refFile->Close();
  delete refFile;

  return refs;
}


// ----------------------------------------------------------------------------
//! Draw text on a pad
// ----------------------------------------------------------------------------
/*! Same note as DrawHistOnPad
 *
 *  \param[in]  iPad  index of pad to draw histogram on
 *  \param[out] plot  canvas containing pad to be drawn on
 *  \           ...   other text options
 */
void BaseJetDrawer::DrawTextOnPad(const std::size_t iPad, 
                                  JetDrawDefs::PlotPads& plot,
                                  double x, 
                                  double y, 
                                  int color, 
                                  const std::string& text, 
                                  double tsize)
{
  plot.histPad->cd(iPad);
  
  TLatex l;
  l.SetTextAlign(22);
  l.SetTextSize(tsize);
  l.SetNDC();
  l.SetTextColor(color);
  l.DrawLatex(x, y, text.c_str());

  plot.canvas->Update();
}
