#include "BCODraw.h"
#include <qahtml/QADrawClient.h>
#include <qahtml/QADrawDB.h>
#include <sPhenixStyle.C>

#include <TCanvas.h>
#include <TDatime.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TPad.h>
#include <TPaveText.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TText.h>

#include <boost/format.hpp>

#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

BCODraw::BCODraw(const std::string &name)
  : QADraw(name)
{
  memset(TC, 0, sizeof(TC));
  memset(transparent, 0, sizeof(transparent));
  memset(Pad, 0, sizeof(Pad));
  DBVarInit();
  return;
}

BCODraw::~BCODraw()
{
  /* delete db; */
  return;
}

int BCODraw::Draw(const std::string &what)
{
  /* SetsPhenixStyle(); */
  int iret = 0;
  int idraw = 0;
  if (what == "ALL" || what == "MVTX" || what == "mvtx")
  {
    iret += DrawMVTX();
    idraw++;
  }
  if (what == "ALL" || what == "INTT" || what == "intt")
  {
    iret += DrawINTT();
    idraw++;
  }
  if (what == "ALL" || what == "TPC")
  {
    iret += DrawTPC();
    idraw++;
  }
  if (what == "ALL" || what == "TPOT" || what == "ebdc39")
  {
    iret += DrawTPOT();
    idraw++;
  }
  if (!idraw)
  {
    std::cout << " Unimplemented Drawing option: " << what << std::endl;
    iret = -1;
  }

  return iret;
}

int BCODraw::MakeCanvas(const std::string &name, int num)
{
  QADrawClient *cl = QADrawClient::instance();
  int xsize = cl->GetDisplaySizeX();
  int ysize = cl->GetDisplaySizeY();
  // xpos (-1) negative: do not draw menu bar
  TC[num] = new TCanvas(name.c_str(), (boost::format("BCO Plots %d") % num).str().c_str(), -1, 0, (int) (xsize / 1.2), (int) (ysize / 1.2));
  gSystem->ProcessEvents();

  if (num == 0 || num == 2)
  {
    Pad[num][0] = new TPad((boost::format("mypad%d0") % num).str().c_str(), "put", 0.05, 0.25, 0.45, 0.75, 0);
    Pad[num][1] = new TPad((boost::format("mypad%d1") % num).str().c_str(), "a", 0.5, 0.25, 0.95, 0.75, 0);

    Pad[num][0]->Draw();
    Pad[num][1]->Draw();
  }
  else if (num == 4)
  {
    Pad[num][0] = new TPad((boost::format("mypad%d0") % num).str().c_str(), "put", 0.05, 0.2, 0.95, 0.8, 0);

    Pad[num][0]->Draw();
  }
  else if (num == 1)
  {
    Pad[num][0] = new TPad((boost::format("mypad%d0") % num).str().c_str(), "put", 0.05, 0.52, 0.32, 0.97, 0);
    Pad[num][1] = new TPad((boost::format("mypad%d1") % num).str().c_str(), "a", 0.37, 0.52, 0.64, 0.97, 0);
    Pad[num][2] = new TPad((boost::format("mypad%d2") % num).str().c_str(), "name", 0.69, 0.52, 0.96, 0.97, 0);
    Pad[num][3] = new TPad((boost::format("mypad%d3") % num).str().c_str(), "here", 0.05, 0.02, 0.32, 0.47, 0);
    Pad[num][4] = new TPad((boost::format("mypad%d4") % num).str().c_str(), "hi", 0.37, 0.02, 0.64, 0.47, 0);
    Pad[num][5] = new TPad((boost::format("mypad%d5") % num).str().c_str(), "hello", 0.69, 0.02, 0.96, 0.47, 0);

    Pad[num][0]->Draw();
    Pad[num][1]->Draw();
    Pad[num][2]->Draw();
    Pad[num][3]->Draw();
    Pad[num][4]->Draw();
    Pad[num][5]->Draw();
  }
  else if (num == 3)
  {
    Pad[num][0] = new TPad((boost::format("mypad%d0") % num).str().c_str(), "put", 0.05, 0.52, 0.22, 0.97, 0);
    Pad[num][1] = new TPad((boost::format("mypad%d1") % num).str().c_str(), "a", 0.28, 0.52, 0.45, 0.97, 0);
    Pad[num][2] = new TPad((boost::format("mypad%d2") % num).str().c_str(), "name", 0.55, 0.52, 0.72, 0.97, 0);
    Pad[num][3] = new TPad((boost::format("mypad%d3") % num).str().c_str(), "here", 0.78, 0.52, 0.95, 0.97, 0);
    Pad[num][4] = new TPad((boost::format("mypad%d4") % num).str().c_str(), "hi", 0.05, 0.02, 0.22, 0.47, 0);
    Pad[num][5] = new TPad((boost::format("mypad%d5") % num).str().c_str(), "hello", 0.28, 0.02, 0.45, 0.47, 0);
    Pad[num][6] = new TPad((boost::format("mypad%d6") % num).str().c_str(), "howdy", 0.55, 0.02, 0.72, 0.47, 0);
    Pad[num][7] = new TPad((boost::format("mypad%d7") % num).str().c_str(), "yo", 0.78, 0.02, 0.95, 0.47, 0);

    Pad[num][0]->Draw();
    Pad[num][1]->Draw();
    Pad[num][2]->Draw();
    Pad[num][3]->Draw();
    Pad[num][4]->Draw();
    Pad[num][5]->Draw();
    Pad[num][6]->Draw();
    Pad[num][7]->Draw();
  }
  else if (num == 5)
  {
    // copied fron MicromegasDraw
    auto cv = TC[num];
    static constexpr int ncol = 2;
    static constexpr int nrow = 2;
    static constexpr double max_height = 0.94;
    cv->Divide(ncol, nrow);

    for (int i = 0; i < ncol * nrow; ++i)
    {
      auto pad = cv->GetPad(i + 1);
      int col = i % ncol;
      int row = i / ncol;
      const double xmin = double(col) / ncol;
      const double xmax = double(col + 1) / ncol;

      const double ymin = max_height * (1. - double(row + 1) / nrow);
      const double ymax = max_height * (1. - double(row) / nrow);
      pad->SetPad(xmin, ymin, xmax, ymax);

      // assign
      Pad[num][i] = static_cast<TPad *>(pad);
    }
  }

  // this one is used to plot the run number on the canvas
  transparent[num] = new TPad((boost::format("transparent%d") % num).str().c_str(), "this does not show", 0, 0, 1, 1);
  transparent[num]->SetFillStyle(4000);
  transparent[num]->Draw();

  return 0;
}

int BCODraw::DrawMVTX()
{
  std::cout << "BCO DrawMVTX() Beginning" << std::endl;
  QADrawClient *cl = QADrawClient::instance();

  const int nmvtxpacket = 12;

  std::ostringstream name;
  bool missingHisto = false;

  TH1 *mvtxrefgl1[nmvtxpacket];
  TH1 *feesstrobemvtx[nmvtxpacket];
  TH1 *feesll1mvtx[nmvtxpacket];
  for (int i = 0; i < nmvtxpacket; i++)
  {
    name.str("");
    name <<"h_MvtxPoolQA_RefGL1BCO_endpoint"<<i;
    mvtxrefgl1[i] = dynamic_cast<TH1 *>(cl->getHisto(name.str().c_str()));

    name.str("");
    name << "h_MvtxPoolQA_TagStBcoFEEsPacket" << i;
    feesstrobemvtx[i] = dynamic_cast<TH1 *>(cl->getHisto(name.str().c_str()));

    name.str("");
    name << "h_MvtxPoolQA_TagL1BcoFEEsPacket" << i;
    feesll1mvtx[i] = dynamic_cast<TH1 *>(cl->getHisto(name.str().c_str()));
    if (!feesstrobemvtx[i] || !feesll1mvtx[i])
    {
      missingHisto = true;
    }
  }
  if (!missingHisto) 
  {

    TH1 *feesstrobefrac[nmvtxpacket];
    for (int i = 0; i < nmvtxpacket; i++)
    {
      const int mvtxgl1 = mvtxrefgl1[i]->GetEntries();
      name.str("");
      name << "feesstrobefrac" << i;
      feesstrobefrac[i] = new TH1F(name.str().c_str(), ";FEEID;GL1 Strobe Tag Frac", 12, 0, 12);

      int feeid = 0;
      for (int j = 0; j < feesstrobemvtx[i]->GetXaxis()->GetNbins(); j++)
      {
        if (feesstrobemvtx[i]->GetBinContent(j + 1) == 0)
        {
          continue;
        }

        const int numbcosFee = feesstrobemvtx[i]->GetBinContent(j + 1);

        float frac = (float) numbcosFee / mvtxgl1;
        feesstrobefrac[i]->SetBinContent(feeid + 1, frac);
        feeid++;
      }
      feesstrobefrac[i]->GetYaxis()->SetRangeUser(0, 1.3);
    }

    TH1 *feesll1frac[nmvtxpacket];
    for (int i = 0; i < nmvtxpacket; i++)
    {
      const int mvtxgl1 = mvtxrefgl1[i]->GetEntries();

      name.str("");
      name << "feesl1frac" << i;
      feesll1frac[i] = new TH1F(name.str().c_str(), ";FEEID;GL1 LL1 Tag Frac", 12, 0, 12);
      int feeid = 0;
      for (int j = 0; j < feesll1mvtx[i]->GetXaxis()->GetNbins(); j++)
      {
        if (feesll1mvtx[i]->GetBinContent(j + 1) == 0)
        {
          continue;
        }

        const int numbcosFee = feesll1mvtx[i]->GetBinContent(j + 1);

        float frac = (float) numbcosFee / mvtxgl1;
        feesll1frac[i]->SetBinContent(feeid + 1, frac);
        feeid++;
      }
      feesll1frac[i]->GetYaxis()->SetRangeUser(0, 1.3);
    }

    if (!gROOT->FindObject("mvtx_evt_building_1"))
    {
      MakeCanvas("mvtx_evt_building_1", 0);
    }
    TC[0]->Clear("D");
    Pad[0][0]->cd();
    gStyle->SetOptStat(0);

    for (int i = 0; i < nmvtxpacket; i++)
    {
      feesstrobefrac[i]->SetLineColor(i + 1);
      feesstrobefrac[i]->SetMarkerColor(i + 1);
      if (i == 0)
      {
        TAxis *x = feesstrobefrac[i]->GetXaxis();
        TAxis *y = feesstrobefrac[i]->GetYaxis();
        x->SetTitle("FEEID");
        y->SetTitle("Strobe-GL1 Tagged Frac.");
        y->SetRangeUser(0, 1.3);
        feesstrobefrac[i]->DrawCopy("hist");
      }
      else
      {
        feesstrobefrac[i]->DrawCopy("histsame");
      }
    }
    myText(0.22, 0.69, feesstrobefrac[0]->GetLineColor(), "Felix.Endpoint 0.0");
    myText(0.22, 0.63, feesstrobefrac[1]->GetLineColor(), "Felix.Endpoint 0.1");
    myText(0.22, 0.57, feesstrobefrac[2]->GetLineColor(), "Felix.Endpoint 1.0");
    myText(0.22, 0.51, kBlack, "...");

    Pad[0][1]->cd();
    gStyle->SetOptStat(0);

    for (int i = 0; i < nmvtxpacket; i++)
    {
      feesll1frac[i]->SetLineColor(i + 1);
      feesll1frac[i]->SetMarkerColor(i + 1);
      if (i == 0)
      {
        TAxis *x = feesll1frac[i]->GetXaxis();
        TAxis *y = feesll1frac[i]->GetYaxis();
        x->SetTitle("FEEID");
        y->SetTitle("LL1-GL1 Tagged Frac.");
        y->SetRangeUser(0, 1.3);
        feesll1frac[i]->DrawCopy("hist");
      }
      else
      {
        feesll1frac[i]->DrawCopy("histsame");
      }
    }

    myText(0.22, 0.69, feesll1frac[0]->GetLineColor(), "Felix.Endpoint 0.0");
    myText(0.22, 0.63, feesll1frac[1]->GetLineColor(), "Felix.Endpoint 0.1");
    myText(0.22, 0.57, feesll1frac[2]->GetLineColor(), "Felix.Endpoint 1.0");
    myText(0.22, 0.51, kBlack, "...");
  }
  else
  {
    std::cout << "DrawMvtx() histogram was missing" << std::endl;
    // histogram is missing
    return -1;
  }

  TText PrintRun;
  PrintRun.SetTextFont(62);
  PrintRun.SetTextSize(0.04);
  PrintRun.SetNDC();          // set to normalized coordinates
  PrintRun.SetTextAlign(23);  // center/top alignment
  std::ostringstream runnostream1;
  std::string runstring1;
  runnostream1 << Name() << "_mvtx evt building Run " << cl->RunNumber() << ", build " << cl->build();
  runstring1 = runnostream1.str();
  transparent[0]->cd();
  PrintRun.DrawText(0.5, 1., runstring1.c_str());
  // transparent[1]->cd();
  // PrintRun.DrawText(0.5, 1., runstring1.c_str());

  TC[0]->Update();
  // TC[1]->Update();

  std::cout << "DrawMVTX Ending" << std::endl;
  return 0;
}

int BCODraw::DrawINTT()
{
  std::cout << "BCO DrawINTT() Beginning" << std::endl;
  QADrawClient *cl = QADrawClient::instance();
  const int ninttpackets = 8;
  const int ninttfees = 14;
  TH1 *h_allfeestagged[ninttpackets];
  TH1 *h_perfee[ninttpackets][ninttfees];
  TH1 *h_gl1ref[ninttpackets];
  std::ostringstream name;
  bool missinghisto = false;

  for (int i = 0; i < ninttpackets; i++)
  {
    name.str("");
    name << "h_InttPoolQA_RefGL1BCO_server" << i;
    h_gl1ref[i] = dynamic_cast<TH1 *>(cl->getHisto(name.str().c_str()));

    name.str("");
    name << "h_InttPoolQA_TagBCOAllFees_Server" << i;
    h_allfeestagged[i] = dynamic_cast<TH1 *>(cl->getHisto(name.str().c_str()));
    for (int j = 0; j < ninttfees; j++)
    {
      name.str("");
      name << "h_InttPoolQA_TagBCO_server" << i << "_fee" << j;
      h_perfee[i][j] = dynamic_cast<TH1 *>(cl->getHisto(name.str().c_str()));
      if (!h_allfeestagged[i] || !h_perfee[i][j])
      {
        missinghisto = true;
      }
    }
    if (!h_gl1ref[i])
    {
      missinghisto = true;
    }
  }

  if (!missinghisto)
  {
    int inttgl1[ninttpackets] = {0};
    float allfeestagged[ninttpackets] = {0};
    float perfee[ninttpackets][14] = {{0}};
    TGraph *grs[ninttpackets];

    for (int i = 0; i < ninttpackets; i++)
    {
      inttgl1[i] = h_gl1ref[i]->GetEntries();

      allfeestagged[i] = h_allfeestagged[i]->GetEntries();
      allfeestagged[i] /= inttgl1[i];
      float x[14];
      for (int j = 0; j < 14; j++)
      {
        perfee[i][j] = h_perfee[i][j]->GetEntries();

        perfee[i][j] /= inttgl1[i];
        x[j] = j;
      }

      grs[i] = new TGraph(14, x, perfee[i]);
    }

    TH1 *packetallfees = new TH1F("inttpacketallfees", ";Server;Frac. GL1 Tagged", 8, 0, 8);
    for (int i = 0; i < ninttpackets; i++)
    {
      packetallfees->SetBinContent(i + 1, allfeestagged[i]);
    }

    if (!gROOT->FindObject("intt_evt_building_1"))
    {
      MakeCanvas("intt_evt_building_1", 2);
    }

    TC[2]->Clear("D");
    Pad[2][0]->cd();
    gStyle->SetOptStat(0);

    packetallfees->GetYaxis()->SetRangeUser(0, 1.2);
    packetallfees->DrawCopy("hist");

    Pad[2][1]->cd();
    gStyle->SetOptStat(0);

    for (int i = 0; i < ninttpackets; i++)
    {
      grs[i]->SetMarkerColor(i + 1);
      grs[i]->SetLineColor(i + 1);
      if (i == 0)
      {
        TAxis *x = grs[i]->GetXaxis();
        TAxis *y = grs[i]->GetYaxis();
        x->SetTitle("FEEID");
        y->SetTitle("Frac. GL1 Tagged");
        y->SetRangeUser(0, 1.2);
        grs[i]->Draw("ap");
      }
      else
      {
        grs[i]->Draw("psame");
      }
    }

    myText(0.22, 0.6, grs[0]->GetMarkerColor(), "Server 0");
    myText(0.22, 0.55, grs[1]->GetMarkerColor(), "Server 1");
    myText(0.22, 0.5, grs[2]->GetMarkerColor(), "Server 2");
    myText(0.22, 0.45, kBlack, "...");
  }
  else
  {
    // histogram is missing
    return -1;
  }

  TText PrintRun;
  PrintRun.SetTextFont(62);
  PrintRun.SetTextSize(0.04);
  PrintRun.SetNDC();          // set to normalized coordinates
  PrintRun.SetTextAlign(23);  // center/top alignment
  std::ostringstream runnostream1;
  std::string runstring1;
  runnostream1 << Name() << "_intt evt building Run " << cl->RunNumber() << ", build " << cl->build();
  runstring1 = runnostream1.str();
  transparent[2]->cd();
  PrintRun.DrawText(0.5, 1., runstring1.c_str());

  TC[2]->Update();

  std::cout << "DrawINTT Ending" << std::endl;
  return 0;
}

int BCODraw::DrawTPC()
{
  std::cout << "BCO DrawTPC() Beginning" << std::endl;
  QADrawClient *cl = QADrawClient::instance();

  const int ntpcpackets = 2;
  const int ntpcebdcs = 24;

  auto h_tpcrefgl1 = dynamic_cast<TH1 *>(cl->getHisto("h_TpcPoolQA_RefGL1BCO"));
  auto h_tpctagall = dynamic_cast<TH1 *>(cl->getHisto("h_TpcPoolQA_TagBCOAllPackets"));

  if (h_tpcrefgl1 && h_tpctagall)
  {
    const int nTpcGL1BCOs = h_tpcrefgl1->GetEntries();
    TH1 *tpcTagEbdc[ntpcebdcs][ntpcpackets];
    int nTpcTagEbdc[ntpcebdcs][ntpcpackets];
    float tpcTagEbdcFrac[ntpcebdcs][ntpcpackets];
    // float tpcEbdcPacket[ntpcpackets];
    // TGraph *tpcEbdcFracGr[ntpcebdcs];
    for (int i = 0; i < ntpcebdcs; i++)
    {
      for (int j = 0; j < ntpcpackets; j++)
      {
        tpcTagEbdc[i][j] = (dynamic_cast<TH1 *>(cl->getHisto((boost::format("h_TpcPoolQA_TagBCO_ebdc%i_packet%i") % (i) % (j)).str())));
        nTpcTagEbdc[i][j] = tpcTagEbdc[i][j]->GetEntries();
        tpcTagEbdcFrac[i][j] = (float) nTpcTagEbdc[i][j] / nTpcGL1BCOs;
        // tpcEbdcPacket[j] = j;
      }
      // tpcEbdcFracGr[i] = new TGraph(ntpcpackets, tpcEbdcPacket, tpcTagEbdcFrac[i]);
    }

    TH1 *tpchistosummary = new TH1F("tpchistosummary", ";Packet ID;Fraction Tagged GL1 BCOs", 49, 0, 49);
    for (int i = 0; i < 48; i++)
    {
      int ebdc = i / 2;
      tpchistosummary->SetBinContent(i + 1, tpcTagEbdcFrac[ebdc][i % 2]);
    }
    tpchistosummary->SetBinContent(49, h_tpctagall->GetEntries() / nTpcGL1BCOs);

    if (!gROOT->FindObject("tpc_evt_building_1"))
    {
      MakeCanvas("tpc_evt_building_1", 4);
    }
    TC[4]->Clear("D");
    Pad[4][0]->cd();
    gStyle->SetOptStat(0);
    tpchistosummary->GetYaxis()->SetRangeUser(0, 1);
    tpchistosummary->GetXaxis()->SetBinLabel(1, "4000");
    tpchistosummary->GetXaxis()->SetBinLabel(2, "4001");
    tpchistosummary->GetXaxis()->SetBinLabel(3, "4010");
    tpchistosummary->GetXaxis()->SetBinLabel(4, "4011");
    tpchistosummary->GetXaxis()->SetBinLabel(5, "4020");
    tpchistosummary->GetXaxis()->SetBinLabel(6, "4021");
    tpchistosummary->GetXaxis()->SetBinLabel(7, "4030");
    tpchistosummary->GetXaxis()->SetBinLabel(8, "4031");
    tpchistosummary->GetXaxis()->SetBinLabel(9, "4040");
    tpchistosummary->GetXaxis()->SetBinLabel(10, "4041");
    tpchistosummary->GetXaxis()->SetBinLabel(11, "4050");
    tpchistosummary->GetXaxis()->SetBinLabel(12, "4051");
    tpchistosummary->GetXaxis()->SetBinLabel(13, "4060");
    tpchistosummary->GetXaxis()->SetBinLabel(14, "4061");
    tpchistosummary->GetXaxis()->SetBinLabel(15, "4070");
    tpchistosummary->GetXaxis()->SetBinLabel(16, "4071");
    tpchistosummary->GetXaxis()->SetBinLabel(17, "4080");
    tpchistosummary->GetXaxis()->SetBinLabel(18, "4081");
    tpchistosummary->GetXaxis()->SetBinLabel(19, "4090");
    tpchistosummary->GetXaxis()->SetBinLabel(20, "4091");
    tpchistosummary->GetXaxis()->SetBinLabel(21, "4100");
    tpchistosummary->GetXaxis()->SetBinLabel(22, "4101");
    tpchistosummary->GetXaxis()->SetBinLabel(23, "4110");
    tpchistosummary->GetXaxis()->SetBinLabel(24, "4111");
    tpchistosummary->GetXaxis()->SetBinLabel(25, "4120");
    tpchistosummary->GetXaxis()->SetBinLabel(26, "4121");
    tpchistosummary->GetXaxis()->SetBinLabel(27, "4130");
    tpchistosummary->GetXaxis()->SetBinLabel(28, "4131");
    tpchistosummary->GetXaxis()->SetBinLabel(29, "4140");
    tpchistosummary->GetXaxis()->SetBinLabel(30, "4141");
    tpchistosummary->GetXaxis()->SetBinLabel(31, "4150");
    tpchistosummary->GetXaxis()->SetBinLabel(32, "4151");
    tpchistosummary->GetXaxis()->SetBinLabel(33, "4160");
    tpchistosummary->GetXaxis()->SetBinLabel(34, "4161");
    tpchistosummary->GetXaxis()->SetBinLabel(35, "4170");
    tpchistosummary->GetXaxis()->SetBinLabel(36, "4171");
    tpchistosummary->GetXaxis()->SetBinLabel(37, "4180");
    tpchistosummary->GetXaxis()->SetBinLabel(38, "4181");
    tpchistosummary->GetXaxis()->SetBinLabel(39, "4190");
    tpchistosummary->GetXaxis()->SetBinLabel(40, "4191");
    tpchistosummary->GetXaxis()->SetBinLabel(41, "4200");
    tpchistosummary->GetXaxis()->SetBinLabel(42, "4201");
    tpchistosummary->GetXaxis()->SetBinLabel(43, "4210");
    tpchistosummary->GetXaxis()->SetBinLabel(44, "4211");
    tpchistosummary->GetXaxis()->SetBinLabel(45, "4220");
    tpchistosummary->GetXaxis()->SetBinLabel(46, "4221");
    tpchistosummary->GetXaxis()->SetBinLabel(47, "4230");
    tpchistosummary->GetXaxis()->SetBinLabel(48, "4231");
    tpchistosummary->GetXaxis()->SetBinLabel(49, "All");
    tpchistosummary->GetXaxis()->SetLabelSize(0.03);
    tpchistosummary->GetXaxis()->LabelsOption("v");
    tpchistosummary->DrawCopy("hist");
    gPad->SetRightMargin(0.15);
  }
  else
  {
    // histogram is missing
    return -1;
  }

  TText PrintRun;
  PrintRun.SetTextFont(62);
  PrintRun.SetTextSize(0.04);
  PrintRun.SetNDC();          // set to normalized coordinates
  PrintRun.SetTextAlign(23);  // center/top alignment
  std::ostringstream runnostream1;
  std::string runstring1;
  runnostream1 << Name() << "_tpc evt building Run " << cl->RunNumber() << ", build " << cl->build();
  runstring1 = runnostream1.str();
  transparent[4]->cd();
  PrintRun.DrawText(0.5, 1., runstring1.c_str());

  TC[4]->Update();

  std::cout << "DrawTPC Ending" << std::endl;
  return 0;
}

//_____________________________________________________________________________________________________
int BCODraw::DrawTPOT()
{
  std::cout << "BCO DrawTPOT() Beginning" << std::endl;
  auto cl = QADrawClient::instance();

  // get canvas or create
  if (!gROOT->FindObject("tpot_evt_building_1"))
  {
    MakeCanvas("tpot_evt_building_1", 5);
  }

  // fill style and color
  static constexpr int fill_style = 3002;
  static constexpr int fill_color = 42;

  // maximum number of packets
  static constexpr int m_npackets_active = 2;

  //! number of fee boards
  static constexpr int m_nfee_max = 26;

  // clear
  TC[5]->Clear("D");

  // GLI BCO matching rate
  auto h_gl1_raw = static_cast<TH1 *>(cl->getHisto("h_MicromegasBCOQA_packet_stat"));
  if (h_gl1_raw)
  {
    auto h_gl1 = new TH1F("h_gl1", "Match Rate", m_npackets_active + 1, 0, m_npackets_active + 1);
    h_gl1->SetStats(0);
    h_gl1->GetXaxis()->SetTitle("Packet");
    h_gl1->GetYaxis()->SetTitle("GL1 BCO drop rate");
    h_gl1->SetTitle("GL1 BCO drop rate by packet");

    h_gl1->GetXaxis()->SetBinLabel(1, "5001");
    h_gl1->GetXaxis()->SetBinLabel(2, "5002");
    h_gl1->GetXaxis()->SetBinLabel(3, "all");

    h_gl1->SetBinContent(1, 1. - double(h_gl1_raw->GetBinContent(2)) / h_gl1_raw->GetBinContent(1));
    h_gl1->SetBinContent(2, 1. - double(h_gl1_raw->GetBinContent(3)) / h_gl1_raw->GetBinContent(1));
    h_gl1->SetBinContent(3, 1. - double(h_gl1_raw->GetBinContent(4)) / h_gl1_raw->GetBinContent(1));

    Pad[5][0]->cd();
    h_gl1->SetMinimum(1e-5);
    h_gl1->SetMaximum(1.1);
    h_gl1->SetFillStyle(fill_style);
    h_gl1->SetFillColor(fill_color);
    h_gl1->Draw();
    gPad->SetLogy();

    auto legend_gl1 = new TLegend(0.65, 0.6, 0.85, 0.84);
    legend_gl1->SetHeader("Values", "C");
    legend_gl1->SetTextSize(0.045);
    legend_gl1->SetBorderSize(0);
    legend_gl1->SetFillStyle(0);

    for (int i = 1; i <= h_gl1->GetNbinsX(); ++i)
    {
      legend_gl1->AddEntry((TObject *) 0, Form("%s: %5.3g", h_gl1->GetXaxis()->GetBinLabel(i), h_gl1->GetBinContent(i)), "");
    }
    legend_gl1->Draw();

    gPad->Update();
  }

  // per packet BCO drop
  auto h_waveform_bco_dropped = static_cast<TH1 *>(cl->getHisto("h_MicromegasBCOQA_waveform_count_dropped_bco"));
  auto h_waveform_pool_dropped = static_cast<TH1 *>(cl->getHisto("h_MicromegasBCOQA_waveform_count_dropped_pool"));
  auto h_waveform_total = static_cast<TH1 *>(cl->getHisto("h_MicromegasBCOQA_waveform_count_total"));
  if (h_waveform_bco_dropped && h_waveform_pool_dropped && h_waveform_total)
  {
    auto h_drop = new TH1F("h_drop", "Drop Rate", m_npackets_active + 1, 0, m_npackets_active + 1);
    h_drop->SetStats(0);
    h_drop->GetXaxis()->SetBinLabel(1, "5001");
    h_drop->GetXaxis()->SetBinLabel(2, "5002");
    h_drop->GetXaxis()->SetBinLabel(3, "all");
    h_drop->GetXaxis()->SetTitle("Packet");
    h_drop->GetYaxis()->SetTitle("Waveform Drop Rate");
    h_drop->SetTitle("Fraction of Dropped Waveforms by packet");

    const double total1 = h_waveform_total->GetBinContent(1);
    if (total1 > 0)
    {
      h_drop->SetBinContent(1, double(h_waveform_bco_dropped->GetBinContent(1) + h_waveform_pool_dropped->GetBinContent(1)) / total1);
    }

    const double total2 = h_waveform_total->GetBinContent(2);
    if (total2 > 0)
    {
      h_drop->SetBinContent(2, double(h_waveform_bco_dropped->GetBinContent(2) + h_waveform_pool_dropped->GetBinContent(2)) / total2);
    }

    if (total1 + total2 > 0)
    {
      h_drop->SetBinContent(3, double(h_waveform_bco_dropped->GetBinContent(1) + h_waveform_pool_dropped->GetBinContent(1) + h_waveform_bco_dropped->GetBinContent(2) + h_waveform_pool_dropped->GetBinContent(2)) / (total1 + total2));
    }

    Pad[5][2]->cd();
    h_drop->SetMinimum(1e-5);
    h_drop->SetMaximum(1.1);
    h_drop->SetFillStyle(fill_style);
    h_drop->SetFillColor(fill_color);
    h_drop->Draw();

    gPad->SetLogy();

    auto legend_drop = new TLegend(0.56, 0.6, 0.85, 0.84);
    legend_drop->SetHeader("Values", "C");
    legend_drop->SetTextSize(0.045);
    legend_drop->SetBorderSize(0);
    legend_drop->SetFillStyle(0);

    gPad->Update();

    for (int i = 1; i <= h_drop->GetNbinsX(); ++i)
    {
      legend_drop->AddEntry((TObject *) 0, Form("%s: %5.3g", h_drop->GetXaxis()->GetBinLabel(i), h_drop->GetBinContent(i)), "");
    }
    legend_drop->Draw();
  }

  // per FEE BCO drop
  auto h_fee_waveform_bco_dropped = static_cast<TH1 *>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_dropped_bco"));
  auto h_fee_waveform_pool_dropped = static_cast<TH1 *>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_dropped_pool"));
  auto h_fee_waveform_total = static_cast<TH1 *>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_total"));
  if (h_fee_waveform_bco_dropped && h_fee_waveform_pool_dropped && h_fee_waveform_total)
  {
    auto h_drop = new TH1F("h_drop_fee", "Drop Rate", m_nfee_max, 0, m_nfee_max);
    h_drop->SetStats(0);
    h_drop->GetXaxis()->SetTitle("FEE ID");
    h_drop->GetYaxis()->SetTitle("Waveform Drop Rate");
    h_drop->SetTitle("Fraction of Dropped Waveforms by fee");

    for (int i = 0; i < m_nfee_max; ++i)
    {
      const double total = h_fee_waveform_total->GetBinContent(i + 1);
      if (total > 0)
      {
        h_drop->SetBinContent(i + 1, double(h_fee_waveform_bco_dropped->GetBinContent(i + 1) + h_fee_waveform_pool_dropped->GetBinContent(i + 1)) / total);
      }
    }

    Pad[5][3]->cd();
    h_drop->SetMinimum(1e-5);
    h_drop->SetMaximum(1.1);
    h_drop->SetFillStyle(fill_style);
    h_drop->SetFillColor(fill_color);
    h_drop->Draw();

    gPad->SetLogy();
    gPad->Update();
  }

  std::ostringstream runnostream1;
  runnostream1 << Name() << "_tpot evt building Run " << cl->RunNumber();
  const auto runstring1 = runnostream1.str();
  transparent[5]->cd();

  TText PrintRun;
  PrintRun.SetTextFont(62);
  PrintRun.SetTextSize(0.04);
  PrintRun.SetNDC();          // set to normalized coordinates
  PrintRun.SetTextAlign(23);  // center/top alignment
  PrintRun.DrawText(0.5, 1., runstring1.c_str());

  TC[5]->Update();
  return 0;
}

int BCODraw::MakeHtml(const std::string &what)
{
  int iret = Draw(what);
  if (iret)  // on error no html output please
  {
    return iret;
  }

  QADrawClient *cl = QADrawClient::instance();
  std::string pngfile;

  // Register the canvas png file to the menu and produces the png file.
  if (what == "ALL" || what == "MVTX" || what == "mvtx")
  {
    pngfile = cl->htmlRegisterPage(*this, "mvtx_evt_building_1", "1", "png");
    cl->CanvasToPng(TC[0], pngfile);
  }
  if (what == "ALL" || what == "INTT" || what == "intt")
  {
    pngfile = cl->htmlRegisterPage(*this, "intt_evt_building_1", "3", "png");
    cl->CanvasToPng(TC[2], pngfile);
  }
  if (what == "ALL" || what == "TPC")
  {
    pngfile = cl->htmlRegisterPage(*this, "tpc_evt_building_1", "5", "png");
    cl->CanvasToPng(TC[4], pngfile);
  }
  if (what == "ALL" || what == "TPOT" || what == "ebdc39")
  {
    pngfile = cl->htmlRegisterPage(*this, "tpot_evt_building_1", "6", "png");
    cl->CanvasToPng(TC[5], pngfile);
  }
  return 0;
}

int BCODraw::DBVarInit()
{
  /* db = new QADrawDB(this); */
  /* db->DBInit(); */
  return 0;
}
void BCODraw::myText(Double_t x, Double_t y, Color_t color,
                     const char *text, Double_t tsize, double angle)
{
  TLatex l;  // l.SetTextAlign(12);
  l.SetTextSize(tsize);
  l.SetNDC();
  l.SetTextColor(color);
  if (angle > 0) l.SetTextAngle(angle);
  l.DrawLatex(x, y, text);
}
