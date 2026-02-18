#include <qahtml/QADrawClient.h>
#include <qahtml/jet/CaloJetDraw.h>
#include <sPhenixStyle.C>
#include <TFile.h>
#include <TSystem.h>
#include <iostream>

R__LOAD_LIBRARY(libqadrawjet.so)

// ----------------------------------------------------------------------------
//! Draw calo jet QA
// ----------------------------------------------------------------------------
/*! Short ROOT macro to test the Calo Jet QADrawClient.
 *  Reads histograms in `infile` and saves produced
 *  plots to `outfile`.
 *
 *  \param[in]  infile   input histogram ROOT file
 *  \param[out] outfile  output ROOT file to save plots to
 *  \param      do_debug turn on (true)/ off (false) debugging messages
 *  \param      do_html  turn on (true)/ off (false) trying to make html page
 */
void draw_calo_jet(const std::string& infile,
                   const std::string& outfile,
                   const bool do_debug = false,
                   const bool do_html = true)
{

  // set plotting style to sPHENIX
  SetsPhenixStyle();
  if (do_debug)
  {
    std::cout << " --- Testing CaloJetDraw\n"
              << " --- Style set to sPHENIX"
              << std::endl;
  }

  // make sure hist title disply is DEFINITELY on
  gStyle -> SetOptTitle(1);
  gROOT  -> ForceStyle();

  // create draw client
  QADrawClient* client = QADrawClient::instance();

  // read histograms from file
  client -> ReadHistogramsFromFile(infile);
  if (do_debug)
  {
    std::cout << " --- Histograms read" << std::endl;
  }

  // create instance of relevant module
  CaloJetDraw* jets = new CaloJetDraw();
  jets -> SetDoDebug(do_debug);
  if (!do_html)
  {
    jets -> SetDoLocal(true);
  }

  // register drawer
  client -> registerDrawer(jets);
  if (do_debug)
  {
    std::cout << " --- JetDraw client registered" << std::endl;
  }

  // make html page if needed
  if (do_html)
  {
    client -> MakeHtml("ALL");
    if (do_debug) std::cout << " --- HTML page made" << std::endl;
  }
  else
  {
    jets -> Draw("ALL");
    if (do_debug) std::cout << " --- Drew plots" << std::endl;

    // save histograms
    TFile* output = new TFile(outfile.data(), "recreate");
    jets -> SaveCanvasesToFile(output);
    if (do_debug)
    {
      std::cout << " --- Save plots to file" << std::endl;
    }
  }

  // delete QADrawClient instance
  delete client;
  if (do_debug)
  {
    std::cout << " --- Deleted QADrawClient, exiting" << std::endl;
  }

  // exit ROOT and return
  gSystem -> Exit(0);
  return;

}

// end ------------------------------------------------------------------------

