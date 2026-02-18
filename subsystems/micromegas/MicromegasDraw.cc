#include "MicromegasDraw.h"

#include <sPhenixStyle.C>

#include <phool/RunnumberRange.h>
#include <qahtml/QADrawClient.h>
#include <qahtml/QADrawDB.h>

#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TLine.h>
#include <TPad.h>
#include <TPaveText.h>
#include <TProfile.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TText.h>
#include <TLegend.h>

#include <boost/format.hpp>

#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
{

 //! make canvas editable in creator, and non-editable in destructor
  class CanvasEditor
  {
    public:
    CanvasEditor( TCanvas* cv ):m_cv(cv)
    { if( m_cv ) m_cv->SetEditable(true); }

    ~CanvasEditor()
    { if( m_cv ) m_cv->SetEditable(false); }

    private:
    TCanvas* m_cv = nullptr;
  };

  //____________________________________________________________________________________________________
  TPad* create_transparent_pad( const std::string& name )
  {
    auto transparent = new TPad( (name+"_transparent").c_str(), "", 0, 0, 1, 1);
    transparent->SetFillStyle(4000);
    transparent->Draw();
    return transparent;
  }

  //____________________________________________________________________________________________________
  TPad* get_transparent_pad( TPad* parent, const std::string& name, bool clear = true)
  {
    // todo: test if can start from groot
    if( !parent ) return nullptr;
    const std::string transparent_name = name+"_transparent";
    auto out = dynamic_cast<TPad*>( parent->FindObject( transparent_name.c_str() ) );

    if( !out ) std::cout << "get_transparent_pad - " << transparent_name << " not found" << std::endl;
    if( out && clear ) out->Clear("D");
    return out;

  }

  //____________________________________________________________________________________________________
  // divide canvas, adjusting canvas positions to leave room for a banner at the top
  void divide_canvas( TCanvas* cv, int ncol, int nrow )
  {
    static constexpr double max_height = 0.94;

    cv->Divide( ncol, nrow );
    for( int i = 0; i < ncol*nrow; ++i )
    {
      auto pad = cv->GetPad( i+1 );
      int col = i%ncol;
      int row = i/ncol;
      const double xmin = double(col)/ncol;
      const double xmax = double(col+1)/ncol;

      const double ymin = max_height*(1. - double(row+1)/nrow);
      const double ymax = max_height*(1. - double(row)/nrow);
      pad->SetPad( xmin, ymin, xmax, ymax );
    }
  }

  //____________________________________________________________________________________________________
  void draw_range( TH1* h, const MicromegasDraw::range_list_t& range_list )
  {

    TLine line;
    line.SetLineColor(2);
    line.SetLineStyle(2);
    line.SetLineWidth(2);

    for( size_t i=0; i<range_list.size(); ++i )
    {
      auto x_min = h->GetXaxis()->GetBinLowEdge(i+1);
      auto x_max = h->GetXaxis()->GetBinUpEdge(i+1);

      // lower edge
      line.DrawLine(x_min, range_list[i].first, x_max, range_list[i].first );
      if( i>0 && range_list[i-1].first != range_list[i].first) { line.DrawLine(x_min, range_list[i-1].first, x_min, range_list[i].first ); }

      // upper edge
      line.DrawLine(x_min, range_list[i].second, x_max, range_list[i].second );
      if( i>0 && range_list[i-1].second != range_list[i].second) { line.DrawLine(x_min, range_list[i-1].second, x_min, range_list[i].second ); }

    }
  }

  //____________________________________________________________________________________________________
  // from histogram, get number of detectors in acceptable range
  int get_num_valid_detectors( TH1* h, const MicromegasDraw::range_list_t& range_list )
  {
    auto nbins = range_list.size();

    int out = 0;
    for( size_t ibin = 0; ibin < nbins; ++ibin )
    {
      // check range validity
      if( range_list[ibin].first >= range_list[ibin].second ) continue;

      // get histogram content and compare
      auto content = h->GetBinContent( ibin+1 );
      if( content >= range_list[ibin].first && content<= range_list[ibin].second ) { ++out; }
    }

    return out;
  }

  //! status flag
  enum class Status
  {
    status_unknown,
    status_bad,
    status_questionable,
    status_good
  };

  //! status strings
  const std::map<Status,std::string> status_string = {
    { Status::status_unknown, "unknown" },
    { Status::status_bad, "bad" },
    { Status::status_questionable, "questionable" },
    { Status::status_good, "good" }
  };

  //! statis color
  const std::map<Status,int> status_color = {
    { Status::status_unknown, kRed+2 },
    { Status::status_bad, kRed+2 },
    { Status::status_questionable, kOrange+2 },
    { Status::status_good, kGreen+2 }
  };

  //! get status based on how many detectors are in acceptable range
  Status get_status( int n_detectors, const MicromegasDraw::detector_range_t acceptable_range )
  {
    if( n_detectors < acceptable_range.first ) { return Status::status_bad; }
    else if( n_detectors < acceptable_range.second ) { return Status::status_questionable; }
    else { return Status::status_good; }
  }

  //! get combined status from subset
  Status get_combined_status( const std::vector<Status>& status_list )
  {
    // global  status is good if all is good
    if( std::all_of( status_list.begin(), status_list.end(), [](const Status& status){ return status == Status::status_good;}))
    { return Status::status_good; }

    // global status is bad if any is bad
    else if( std::any_of( status_list.begin(), status_list.end(), [](const Status& status){ return status == Status::status_bad;}))
    { return Status::status_bad; }

    // global status is unknown if any is unknown
    else if( std::any_of( status_list.begin(), status_list.end(), [](const Status& status){ return status == Status::status_unknown;}))
    { return Status::status_unknown; }

    // status is questionable in all other cases
    else return Status::status_questionable;
  }

}

//____________________________________________________________________________________________________
MicromegasDraw::MicromegasDraw(const std::string &name)
  : QADraw(name)
{ return; }

//____________________________________________________________________________________________________
int MicromegasDraw::Init()
{
  // get runnumber from DrawClient
  const auto cl = QADrawClient::instance();
  const auto runnumber = cl->RunNumber();

  std::cout << "MicromegasDraw::Init - runnumber: " << runnumber << std::endl;

  // initialize variables
  if( runnumber >= RunnumberRange::RUN3PP_FIRST )
  {
    // run3 pp cuts
    setup_cuts_run3_pp();

  } else {
    /*
     * for all other runnumbers assume run3 au+au.
     * this could be changed in the future
     */
    setup_cuts_run3_auau();
  }

  return 0;

}

//____________________________________________________________________________________________________
int MicromegasDraw::Draw(const std::string &what)
{
  /* SetsPhenixStyle(); */
  int iret = 0;
  int idraw = 0;

  // TPOT BCO QA
  if (what == "ALL" || what == "BCO" )
  {
    iret += draw_bco_info();
    idraw++;
  }

  // TPOT raw distributions for cluster QA
  if (what == "ALL" || what == "CLUSTERS_RAW")
  {
    iret += draw_raw_cluster_info();
    idraw++;
  }

  // TPOT average distributions for cluster QA
  if (what == "ALL" || what == "CLUSTERS_AVG")
  {
    iret += draw_average_cluster_info();
    idraw++;
  }

  // TPOT BCO QA summary
  if (what == "ALL" || what == "BCO_SUMMARY")
  {
    iret += draw_bco_summary();
    idraw++;
  }

  // TPOT Cluster QA summary
  if (what == "ALL" || what == "SUMMARY")
  {
    iret += draw_summary();
    idraw++;
  }

  if (!idraw)
    {
      std::cout << " Unimplemented Drawing option: " << what << std::endl;
      iret = -1;
    }
  return iret;
}

//____________________________________________________________________________________________________
TH1* MicromegasDraw::get_detector_average(TH2* source, double offset)
{
  auto h = new TH1F( Form("%s_average", source->GetName()), "", source->GetNbinsX(), source->GetXaxis()->GetXmin(), source->GetXaxis()->GetXmax());

  //
  for (int ix = 1; ix <= source->GetNbinsX(); ++ix)
  {
    std::unique_ptr<TH1> p( source->ProjectionY("proj", ix, ix ));
    h->SetBinContent(ix, p->GetMean()+offset );
    h->SetBinError(ix, p->GetMeanError() );
    h->GetXaxis()->SetBinLabel(ix, source->GetXaxis()->GetBinLabel(ix) );
  }

  h->SetMarkerStyle(20);

  return h;
}

//__________________________________________________________________________________
TCanvas* MicromegasDraw::get_canvas(const std::string& name, bool clear )
{
  auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( name.c_str() ) );
  if( !cv ) cv = create_canvas( name );
  if( cv && clear ) cv->Clear("D");
  return cv;
}

//__________________________________________________________________________________
TCanvas* MicromegasDraw::create_canvas(const std::string &name)
{

  if( Verbosity() )
  { std::cout << "MicromegasDraw::create_canvas - name: " << name << std::endl; }

  auto cl = QADrawClient::instance();
  int xsize = cl->GetDisplaySizeX();
  int ysize = cl->GetDisplaySizeY();

  // TPOT BCO QA
  if (name == "TPOT_BCO") {

    auto cv = new TCanvas(name.c_str(), "TPOT BCO information", -1, 0,xsize/1.2, ysize/1.2);
    gSystem->ProcessEvents();
    divide_canvas(cv, 2, 2);
    create_transparent_pad(name);
    cv->SetEditable(false);
    m_canvas.push_back( cv );
    return cv;

  // TPOT raw distributions for cluster QA
  } else if (name == "TPOT_CLUSTERS_RAW") {

    auto cv = new TCanvas(name.c_str(), "TPOT cluster raw distributions [expert]", -1, 0,xsize/1.2, ysize/1.2);
    gSystem->ProcessEvents();
    divide_canvas(cv, 2, 2);
    create_transparent_pad(name);
    cv->SetEditable(false);
    m_canvas.push_back( cv );
    return cv;

  // TPOT average distributions for cluster QA
  } else if (name == "TPOT_CLUSTERS_AVG") {

    auto cv = new TCanvas(name.c_str(), "TPOT cluster mean distributions", -1, 0,xsize/1.2, ysize/1.2);
    gSystem->ProcessEvents();
    divide_canvas(cv, 2, 2);
    create_transparent_pad(name);
    cv->SetEditable(false);
    m_canvas.push_back( cv );
    return cv;

  // TPOT BCO QA summary
  } else if (name == "TPOT_BCO_SUMMARY") {

    auto cv = new TCanvas(name.c_str(), "TPOT BCO summary", -1, 0,xsize/1.2, ysize/1.2);
    gSystem->ProcessEvents();
    cv->Divide(1,1);
    create_transparent_pad(name);
    cv->SetEditable(false);
    m_canvas.push_back( cv );
    return cv;

  // TPOT Cluster QA summary
  } else if (name == "TPOT_SUMMARY") {

    auto cv = new TCanvas(name.c_str(), "TPOT summary", -1, 0,xsize/1.2, ysize/1.2);
    gSystem->ProcessEvents();
    cv->Divide(1,1);
    create_transparent_pad(name);
    cv->SetEditable(false);
    m_canvas.push_back( cv );
    return cv;

  }

  return nullptr;
}

//__________________________________________________________________________________
void MicromegasDraw::draw_title( TPad* pad )
{
  if( !pad )
  {
    if( Verbosity() ) std::cout << "MicromegasDraw::draw_title - invalid pad" << std::endl;
    return;
  }

  pad->SetPad( 0, 0.95, 1, 1 );
  pad->Clear();
  TText PrintRun;
  PrintRun.SetTextSize(0.7);
  PrintRun.SetNDC();          // set to normalized coordinates
  PrintRun.SetTextAlign(23);  // center/top alignment

  std::ostringstream runnostream;
  auto cl = QADrawClient::instance();
  runnostream << Name() << " Run " << cl->RunNumber();

  pad->cd();
  PrintRun.DrawText(0.5, 0.5, runnostream.str().c_str());
}

//____________________________________________________________________________________________________
int MicromegasDraw::draw_bco_info()
{
  auto cl = QADrawClient::instance();

  static constexpr int fill_style = 3002;
  static constexpr int fill_color = 42;

  // GLI BCO matching rate
  auto h_gl1_raw = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_packet_stat"));
  if (h_gl1_raw)
  {
    auto h_gl1= new TH1F("h_gl1", "Match Rate", m_npackets_active+1, 0, m_npackets_active+1);
    h_gl1->SetStats(0);
    h_gl1->GetXaxis()->SetTitle("Packet");
    h_gl1->GetYaxis()->SetTitle("GL1 BCO drop rate");
    h_gl1->SetTitle("GL1 BCO drop rate by packet");

    h_gl1->GetXaxis()->SetBinLabel(1, "5001");
    h_gl1->GetXaxis()->SetBinLabel(2, "5002");
    h_gl1->GetXaxis()->SetBinLabel(3, "all");

    h_gl1->SetBinContent(1,1.-double(h_gl1_raw->GetBinContent(2))/h_gl1_raw->GetBinContent(1));
    h_gl1->SetBinContent(2,1.-double(h_gl1_raw->GetBinContent(3))/h_gl1_raw->GetBinContent(1));
    h_gl1->SetBinContent(3,1.-double(h_gl1_raw->GetBinContent(4))/h_gl1_raw->GetBinContent(1));

    auto cv = get_canvas("TPOT_BCO");
    CanvasEditor cv_edit(cv);
    cv->cd(1);
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
      legend_gl1->AddEntry((TObject*)0, Form("%s: %5.3g", h_gl1->GetXaxis()->GetBinLabel(i), h_gl1->GetBinContent(i)), "");
    }
    legend_gl1->Draw();

    gPad->Update();
    draw_range( h_gl1, m_gl1_drop_rate_range );

  }

  // per packet BCO drop
  auto h_waveform_bco_dropped = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_waveform_count_dropped_bco"));
  auto h_waveform_pool_dropped = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_waveform_count_dropped_pool"));
  auto h_waveform_total = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_waveform_count_total"));
  if (h_waveform_bco_dropped && h_waveform_pool_dropped && h_waveform_total )
  {
    auto h_drop= new TH1F("h_drop", "Drop Rate", m_npackets_active+1, 0, m_npackets_active+1);
    h_drop->SetStats(0);
    h_drop->GetXaxis()->SetBinLabel(1, "5001");
    h_drop->GetXaxis()->SetBinLabel(2, "5002");
    h_drop->GetXaxis()->SetBinLabel(3, "all");
    h_drop->GetXaxis()->SetTitle("Packet");
    h_drop->GetYaxis()->SetTitle("Waveform Drop Rate");
    h_drop->SetTitle("Fraction of Dropped Waveforms by packet");

    const double total1 = h_waveform_total->GetBinContent(1);
    if(total1>0)
    { h_drop->SetBinContent(1, double(h_waveform_bco_dropped->GetBinContent(1)+ h_waveform_pool_dropped->GetBinContent(1))/total1); }

    const double total2 = h_waveform_total->GetBinContent(2);
    if( total2>0 )
    { h_drop->SetBinContent(2, double(h_waveform_bco_dropped->GetBinContent(2)+ h_waveform_pool_dropped->GetBinContent(2))/total2); }

    if( total1+total2>0)
    { h_drop->SetBinContent(3, double(h_waveform_bco_dropped->GetBinContent(1)+ h_waveform_pool_dropped->GetBinContent(1)+h_waveform_bco_dropped->GetBinContent(2)+ h_waveform_pool_dropped->GetBinContent(2))/(total1+total2) ); }

    auto cv = get_canvas("TPOT_BCO");
    CanvasEditor cv_edit(cv);
    cv->cd(3);
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
    draw_range( h_drop, m_waveform_drop_rate_range );

    for (int i = 1; i <= h_drop->GetNbinsX(); ++i)
    {
      legend_drop->AddEntry((TObject*)0, Form("%s: %5.3g", h_drop->GetXaxis()->GetBinLabel(i), h_drop->GetBinContent(i)), "");
    }
    legend_drop->Draw();
  }

  // per FEE BCO drop
  auto h_fee_waveform_bco_dropped = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_dropped_bco"));
  auto h_fee_waveform_pool_dropped = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_dropped_pool"));
  auto h_fee_waveform_total = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_total"));
  if (h_fee_waveform_bco_dropped && h_fee_waveform_pool_dropped && h_fee_waveform_total )
  {
    auto h_drop= new TH1F("h_drop_fee", "Drop Rate", m_nfee_max, 0, m_nfee_max);
    h_drop->SetStats(0);
    h_drop->GetXaxis()->SetTitle("FEE ID");
    h_drop->GetYaxis()->SetTitle("Waveform Drop Rate");
    h_drop->SetTitle("Fraction of Dropped Waveforms by fee");

    for( int i = 0; i< m_nfee_max; ++i )
    {
      const double total = h_fee_waveform_total->GetBinContent(i+1);
      if( total > 0 )
      { h_drop->SetBinContent(i+1, double(h_fee_waveform_bco_dropped->GetBinContent(i+1)+ h_fee_waveform_pool_dropped->GetBinContent(i+1))/total); }
    }

    auto cv = get_canvas("TPOT_BCO");
    CanvasEditor cv_edit(cv);
    cv->cd(4);
    h_drop->SetMinimum(1e-5);
    h_drop->SetMaximum(1.1);
    h_drop->SetFillStyle(fill_style);
    h_drop->SetFillColor(fill_color);
    h_drop->Draw();

    gPad->SetLogy();

    gPad->Update();
    draw_range( h_drop, m_fee_waveform_drop_rate_range );

  }

  {
    // title
    auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( "TPOT_BCO" ) );
    CanvasEditor cv_edit(cv);
    auto transparent = get_transparent_pad( cv, "TPOT_BCO");
    draw_title(transparent);
  }

  return 0;
}

//____________________________________________________________________________________________________
int MicromegasDraw::draw_raw_cluster_info()
{
  auto cl = QADrawClient::instance();
  auto draw_profile = []( TH2* h, double offset = 0 )
  {
    std::unique_ptr<TProfile> p(h->ProfileX());
    auto hp = new TH1F(Form( "%s_p", h->GetName()), "", h->GetNbinsX(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax() );
    for( int i = 0; i < p->GetNbinsX(); ++i )
    {
      hp->SetBinContent(i+1, p->GetBinContent(i+1)+offset );
      hp->SetBinError(i+1, p->GetBinError(i+1) );
    }
    hp->SetMarkerStyle(20);
    hp->Draw("p same");
  };

  // cluster multiplicity
  auto h_cluster_multiplicity = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_multiplicity"));
  if( h_cluster_multiplicity )
  {
    auto cv = get_canvas("TPOT_CLUSTERS_RAW");
    CanvasEditor cv_edit(cv);
    cv->cd(1);
    h_cluster_multiplicity->SetTitle("Cluster Multiplicity");
    h_cluster_multiplicity->GetXaxis()->SetTitle("Chamber");
    h_cluster_multiplicity->GetYaxis()->SetTitle("Cluster Multiplicity");
    h_cluster_multiplicity->DrawCopy("COLZ");
    /* correct profile by 0.5 due to incorrect binning, with integer values at bin edges rather than bin center */
    draw_profile(h_cluster_multiplicity, -0.5);
    gPad->Update();
  }

  // cluster size
  auto h_cluster_size = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_size"));
  if( h_cluster_size )
  {
    auto cv = get_canvas("TPOT_CLUSTERS_RAW");
    CanvasEditor cv_edit(cv);
    cv->cd(2);
    h_cluster_size->SetTitle("Cluster Size");
    h_cluster_size->GetXaxis()->SetTitle("Chamber");
    h_cluster_size->GetYaxis()->SetTitle("Cluster Size");
    h_cluster_size->DrawCopy("COLZ");
    /* correct profile by 0.5 due to incorrect binning, with integer values at bin edges rather than bin center */
    draw_profile(h_cluster_size, -0.5);
    gPad->Update();
  }

  // cluster charge
  auto h_cluster_charge = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_charge"));
  if( h_cluster_charge )
  {
    auto cv = get_canvas("TPOT_CLUSTERS_RAW");
    CanvasEditor cv_edit(cv);
    cv->cd(3);
    h_cluster_charge->SetTitle("Cluster Charge");
    h_cluster_charge->GetXaxis()->SetTitle("Chamber");
    h_cluster_charge->GetYaxis()->SetTitle("Cluster Charge");
    h_cluster_charge->DrawCopy("COLZ");
    draw_profile(h_cluster_charge);
    gPad->Update();
  }

  {
    // title
    auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( "TPOT_CLUSTERS_RAW" ) );
    CanvasEditor cv_edit(cv);
    auto transparent = get_transparent_pad( cv, "TPOT_CLUSTERS_RAW");
    draw_title(transparent);
  }
  return 0;
}

//____________________________________________________________________________________________________
int MicromegasDraw::draw_average_cluster_info()
{
  auto cl = QADrawClient::instance();

  // cluster multiplicity
  auto h_cluster_multiplicity_raw = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_multiplicity"));
  if( h_cluster_multiplicity_raw )
  {
    auto cv = get_canvas("TPOT_CLUSTERS_AVG");
    CanvasEditor cv_edit(cv);
    cv->cd(1);

    // per chamber averaged distributions
    /* correct profile by 0.5 due to incorrect binning,
     * with integer values at bin edges rather than bin center
     */
    auto h_cluster_multiplicity = get_detector_average(h_cluster_multiplicity_raw, -0.5);
    h_cluster_multiplicity->SetTitle("Cluster Multiplicity");
    h_cluster_multiplicity->GetXaxis()->SetTitle("Chamber");
    h_cluster_multiplicity->GetYaxis()->SetTitle("#LTCluster Multiplicity#GT");
    h_cluster_multiplicity->SetStats(0);

//     h_cluster_multiplicity->SetMinimum(0.001);
//     h_cluster_multiplicity->SetMaximum(5);

    h_cluster_multiplicity->SetMinimum(0);
    h_cluster_multiplicity->SetMaximum(5);

    h_cluster_multiplicity->DrawCopy("P");
//     gPad->SetLogy(true);
    gPad->Update();

    draw_range( h_cluster_multiplicity, m_cluster_multiplicity_range );
    gPad->Update();
  }

  // cluster size
  auto h_cluster_size_raw = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_size"));
  if( h_cluster_size_raw )
  {
    auto cv = get_canvas("TPOT_CLUSTERS_AVG");
    CanvasEditor cv_edit(cv);
    cv->cd(2);

    // per chamber averaged distributions
    /* correct profile by 0.5 due to incorrect binning,
     * with integer values at bin edges rather than bin center
     */
    auto h_cluster_size = get_detector_average(h_cluster_size_raw, -0.5);
    h_cluster_size->SetTitle("Cluster Size");
    h_cluster_size->GetXaxis()->SetTitle("Chamber");
    h_cluster_size->GetYaxis()->SetTitle("#LTCluster Size#GT");
    h_cluster_size->SetStats(0);
    h_cluster_size->SetMinimum(0);
    h_cluster_size->SetMaximum(8);
    h_cluster_size->DrawCopy("P");
    gPad->Update();
    draw_range( h_cluster_size, m_cluster_size_range );
    gPad->Update();
  }

  // cluster charge
  auto h_cluster_charge_raw = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_charge"));
  if( h_cluster_charge_raw )
  {
    auto cv = get_canvas("TPOT_CLUSTERS_AVG");
    CanvasEditor cv_edit(cv);
    cv->cd(3);

    // per chamber averaged distributions
    auto h_cluster_charge = get_detector_average(h_cluster_charge_raw);
    h_cluster_charge->SetTitle("Cluster Charge");
    h_cluster_charge->GetXaxis()->SetTitle("Chamber");
    h_cluster_charge->GetYaxis()->SetTitle("#LTCluster Charge#GT");
    h_cluster_charge->SetStats(0);
    h_cluster_charge->SetMinimum(0);
    h_cluster_charge->SetMaximum(1000);
    h_cluster_charge->DrawCopy("P");
    gPad->Update();
    draw_range( h_cluster_charge, m_cluster_charge_range );
    gPad->Update();
  }

  // estimated efficiency
  auto h_cluster_count_ref = static_cast<TH1*>(cl->getHisto("h_MicromegasClusterQA_clustercount_ref"));
  auto h_cluster_count_found = static_cast<TH1*>(cl->getHisto("h_MicromegasClusterQA_clustercount_found"));
  if( h_cluster_count_ref && h_cluster_count_found )
  {
    auto cv = get_canvas("TPOT_CLUSTERS_AVG");
    CanvasEditor cv_edit(cv);
    cv->cd(4);

    // efficiency histogram
    auto efficiency = static_cast<TH1*>(h_cluster_count_found->Clone("efficiency"));
    efficiency->Divide(h_cluster_count_found, h_cluster_count_ref, 1, 1, "B" );
    efficiency->SetMarkerStyle(20);
    efficiency->SetMinimum(0);
    efficiency->SetMaximum(1);
    efficiency->SetTitle("Efficiency Estimate");
    efficiency->GetXaxis()->SetTitle("Chamber");
    efficiency->GetYaxis()->SetTitle("Efficiency Estimate");
    efficiency->SetStats(0);
    efficiency->DrawCopy( "P" );
    gPad->Update();
    draw_range( efficiency, m_efficiency_range );
    gPad->Update();
  }

  {
    // title
    auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( "TPOT_CLUSTERS_AVG" ) );
    CanvasEditor cv_edit(cv);
    auto transparent = get_transparent_pad( cv, "TPOT_CLUSTERS_AVG");
    draw_title(transparent);
  }
  return 0;
}


//____________________________________________________________________________________________________
int MicromegasDraw::draw_bco_summary()
{
  auto cl = QADrawClient::instance();

  Status status_gl1_drop_rate = Status::status_unknown;
  Status status_waveform_drop_rate = Status::status_unknown;
  Status status_fee_waveform_drop_rate = Status::status_unknown;

  // GLI BCO matching rate
  auto h_gl1_raw = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_packet_stat"));

  // dropped waveforms per packet
  auto h_waveform_bco_dropped = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_waveform_count_dropped_bco"));
  auto h_waveform_pool_dropped = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_waveform_count_dropped_pool"));
  auto h_waveform_total = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_waveform_count_total"));

  // dropped waveforms per fee
  auto h_fee_waveform_bco_dropped = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_dropped_bco"));
  auto h_fee_waveform_pool_dropped = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_dropped_pool"));
  auto h_fee_waveform_total = static_cast<TH1*>(cl->getHisto("h_MicromegasBCOQA_fee_waveform_count_total"));

  // check histogram valiity
  if( !( h_gl1_raw ||
    (h_waveform_bco_dropped&&h_waveform_pool_dropped&&h_waveform_total) ||
    (h_fee_waveform_bco_dropped&&h_fee_waveform_pool_dropped&&h_fee_waveform_total)) )
  {
    std::cout << "MicromegasDraw::draw_bco_summary - no histograms found." << std::endl;
    return 0;
  }

  auto cv = get_canvas("TPOT_BCO_SUMMARY" );
  CanvasEditor cv_edit(cv);

  cv->cd(1);
  auto text = new TPaveText(0.1,0.1,0.9,0.9, "NDC" );
  text->SetFillColor(0);
  text->SetFillStyle(0);
  text->SetBorderSize(0);
  text->SetTextAlign(11);

  text->AddText( "TPOT BCO summary:" );

  if (h_gl1_raw)
  {
    std::unique_ptr<TH1> h_gl1(new TH1F("h_gl1_summary", "Match Rate", m_npackets_active+1, 0, m_npackets_active+1));
    h_gl1->SetBinContent(1,1.-double(h_gl1_raw->GetBinContent(2))/h_gl1_raw->GetBinContent(1));
    h_gl1->SetBinContent(2,1.-double(h_gl1_raw->GetBinContent(3))/h_gl1_raw->GetBinContent(1));
    h_gl1->SetBinContent(3,1.-double(h_gl1_raw->GetBinContent(4))/h_gl1_raw->GetBinContent(1));
    const auto ngood = get_num_valid_detectors( h_gl1.get(), m_gl1_drop_rate_range );
    status_gl1_drop_rate = get_status( ngood, m_packet_gl1_drop_rate_range );
    text->AddText( Form("Number of packets with gl1 drop rate in acceptable range: %i/%i - %s",ngood, m_npackets_active+1, status_string.at(status_gl1_drop_rate).c_str()))
      ->SetTextColor(status_color.at(status_gl1_drop_rate));
  } else {
    text->AddText("Number of packets with gl1 drop rate in acceptable range: unknown (missing histograms)")
      ->SetTextColor(status_color.at(status_gl1_drop_rate));
  }

  // per packet BCO drop
  if (h_waveform_bco_dropped && h_waveform_pool_dropped && h_waveform_total )
  {
    std::unique_ptr<TH1> h_drop( new TH1F("h_drop_summary", "Drop Rate", m_npackets_active+1, 0, m_npackets_active+1) );
    const double total1 = h_waveform_total->GetBinContent(1);
    if(total1>0)
    { h_drop->SetBinContent(1, double(h_waveform_bco_dropped->GetBinContent(1)+ h_waveform_pool_dropped->GetBinContent(1))/total1); }

    const double total2 = h_waveform_total->GetBinContent(2);
    if( total2>0 )
    { h_drop->SetBinContent(2, double(h_waveform_bco_dropped->GetBinContent(2)+ h_waveform_pool_dropped->GetBinContent(2))/total2); }

    if( total1+total2>0)
    { h_drop->SetBinContent(3, double(h_waveform_bco_dropped->GetBinContent(1)+ h_waveform_pool_dropped->GetBinContent(1)+h_waveform_bco_dropped->GetBinContent(2)+ h_waveform_pool_dropped->GetBinContent(2))/(total1+total2) ); }

    const auto ngood = get_num_valid_detectors( h_drop.get(), m_waveform_drop_rate_range );
    status_waveform_drop_rate = get_status( ngood, m_packet_wf_drop_rate_range );
    text->AddText( Form("Number of packets with waveform drop rate in acceptable range: %i/%i - %s",ngood, m_npackets_active+1, status_string.at(status_waveform_drop_rate).c_str()))
      ->SetTextColor(status_color.at(status_waveform_drop_rate));
  } else {
    text->AddText( "Number of packets with waveform drop rate in acceptable range: unknown (missing histograms)" )
      ->SetTextColor(status_color.at(status_waveform_drop_rate));
  }

  // per FEE BCO drop
  if (h_fee_waveform_bco_dropped && h_fee_waveform_pool_dropped && h_fee_waveform_total )
  {
    std::unique_ptr<TH1> h_drop( new TH1F("h_drop_fee_summary", "Drop Rate", m_nfee_max, 0, m_nfee_max));
    for( int i = 0; i< m_nfee_max; ++i )
    {
      const double total = h_fee_waveform_total->GetBinContent(i+1);
      if( total > 0 )
      { h_drop->SetBinContent(i+1, double(h_fee_waveform_bco_dropped->GetBinContent(i+1)+ h_fee_waveform_pool_dropped->GetBinContent(i+1))/total); }
    }
    const auto ngood = get_num_valid_detectors( h_drop.get(), m_fee_waveform_drop_rate_range );
    status_fee_waveform_drop_rate = get_status( ngood, m_fee_wf_drop_rate_range );
    text->AddText( Form("Number of fee with waveform drop rate in acceptable range: %i/%i - %s",ngood, m_nfee_active, status_string.at(status_fee_waveform_drop_rate).c_str()))
      ->SetTextColor(status_color.at(status_fee_waveform_drop_rate));
  } else {
    text->AddText( "Number of fee with waveform drop rate in acceptable range: unknown (missing histograms)" )
      ->SetTextColor(status_color.at(status_fee_waveform_drop_rate));
  }

  // global status
  const auto status_global = get_combined_status({ status_gl1_drop_rate, status_waveform_drop_rate, status_fee_waveform_drop_rate});
  text->AddText( Form("Overall status (BCO): %s",status_string.at(status_global).c_str()))
    ->SetTextColor(status_color.at(status_global));

  text->Draw();

  auto transparent = get_transparent_pad( cv, "TPOT_BCO_SUMMARY");
  draw_title(transparent);

  cv->Update();
  return 0;
}

//____________________________________________________________________________________________________
int MicromegasDraw::draw_summary()
{
  auto cl = QADrawClient::instance();

  Status status_cluster_multiplicity = Status::status_unknown;
  Status status_cluster_size = Status::status_unknown;
  Status status_cluster_charge = Status::status_unknown;
  Status status_efficiency = Status::status_unknown;

  // load histograms
  auto h_cluster_count_ref = static_cast<TH1*>(cl->getHisto("h_MicromegasClusterQA_clustercount_ref"));
  auto h_cluster_count_found = static_cast<TH1*>(cl->getHisto("h_MicromegasClusterQA_clustercount_found"));
  auto h_cluster_multiplicity_raw = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_multiplicity"));
  auto h_cluster_size_raw = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_size"));
  auto h_cluster_charge_raw = static_cast<TH2*>(cl->getHisto("h_MicromegasClusterQA_cluster_charge"));

  if( !((h_cluster_count_ref&&h_cluster_count_found)||h_cluster_multiplicity_raw||h_cluster_size_raw||h_cluster_charge_raw))
  {
    std::cout << "MicromegasDraw::draw_summary - no histograms found." << std::endl;
    return 0;
  }

  auto cv = get_canvas("TPOT_SUMMARY" );
  CanvasEditor cv_edit(cv);

  cv->cd(1);
  auto text = new TPaveText(0.1,0.1,0.9,0.9, "NDC" );
  text->SetFillColor(0);
  text->SetFillStyle(0);
  text->SetBorderSize(0);
  text->SetTextAlign(11);

  text->AddText( "TPOT summary:" );

  if( h_cluster_multiplicity_raw )
  {
    std::unique_ptr<TH1> h_cluster_multiplicity( get_detector_average(h_cluster_multiplicity_raw, -0.5) );
    const auto ngood = get_num_valid_detectors( h_cluster_multiplicity.get(), m_cluster_multiplicity_range );
    status_cluster_multiplicity = get_status( ngood, m_detector_cluster_mult_range );
    text->AddText( Form("Number of detectors with cluster multiplicity in acceptable range: %i/%i - %s",ngood, m_nfee_active, status_string.at(status_cluster_multiplicity).c_str()))
      ->SetTextColor(status_color.at(status_cluster_multiplicity));
  } else {
    text->AddText("Number of detectors with cluster multiplicity in acceptable range: unknown (missing histograms)" )
      ->SetTextColor(status_color.at(status_cluster_multiplicity));
  }

  if( h_cluster_size_raw )
  {
    std::unique_ptr<TH1> h_cluster_size( get_detector_average(h_cluster_size_raw, -0.5) );
    const auto ngood = get_num_valid_detectors( h_cluster_size.get(), m_cluster_size_range );
    status_cluster_size = get_status( ngood, m_detector_cluster_size_range );
    text->AddText( Form("Number of detectors with cluster size in acceptable range: %i/%i - %s",ngood, m_nfee_active, status_string.at(status_cluster_size).c_str()))
      ->SetTextColor(status_color.at(status_cluster_size));
  } else {
    text->AddText( "Number of detectors with cluster size in acceptable range: unknown (missing histograms)" )
      ->SetTextColor(status_color.at(status_cluster_size));
  }

  if( h_cluster_charge_raw )
  {
    std::unique_ptr<TH1> h_cluster_charge( get_detector_average(h_cluster_charge_raw) );
    const auto ngood = get_num_valid_detectors( h_cluster_charge.get(), m_cluster_charge_range );
    status_cluster_charge = get_status( ngood, m_detector_cluster_charge_range );
    text->AddText( Form("Number of detectors with cluster charge in acceptable range: %i/%i - %s",ngood,m_nfee_active,status_string.at(status_cluster_charge).c_str()))
      ->SetTextColor(status_color.at(status_cluster_charge));
  } else {
    text->AddText( "Number of detectors with cluster charge in acceptable range: unknown (missing histograms)" )
      ->SetTextColor(status_color.at(status_cluster_charge));
  }

  if( h_cluster_count_ref&&h_cluster_count_found )
  {
    std::unique_ptr<TH1> efficiency(static_cast<TH1*>(h_cluster_count_found->Clone("efficiency_summary")));
    efficiency->Divide(h_cluster_count_found, h_cluster_count_ref, 1, 1, "B" );
    const auto ngood = get_num_valid_detectors( efficiency.get(), m_efficiency_range );
    status_efficiency = get_status( ngood, m_detector_efficiency_range );
    text->AddText( Form("Number of detectors with efficiency estimate in acceptable range: %i/%i - %s",ngood,m_nfee_active,status_string.at(status_efficiency).c_str()))
      ->SetTextColor(status_color.at(status_efficiency));
  } else {
    text->AddText( "Number of detectors with efficiency estimate in acceptable range: unknown (missing histograms)" )
      ->SetTextColor(status_color.at(status_efficiency));
  }

  // global status
  const auto status_global = get_combined_status({ status_cluster_multiplicity, status_cluster_size, status_cluster_charge, status_efficiency });
  text->AddText( Form("Overall status: %s",status_string.at(status_global).c_str()))
    ->SetTextColor(status_color.at(status_global));

  text->Draw();

  // title
  auto transparent = get_transparent_pad( cv, "TPOT_SUMMARY");
  draw_title(transparent);
  cv->Update();

  return 0;
}

//________________________________________________________________
int MicromegasDraw::MakeHtml(const std::string &what)
{
  int iret = Draw(what);
  if (iret) // on error no html output please
  {
    return iret;
  }

  auto cl = QADrawClient::instance();
  std::string pngfile;

  // raw cluster information (experts)
  if (what == "ALL" || what == "BCO")
  {
    auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( "TPOT_BCO" ) );
    if( cv )
    {
      pngfile = cl->htmlRegisterPage(*this, "tpot_bco", "1", "png");
      cl->CanvasToPng(cv, pngfile);
    }
  }

  // raw cluster information (experts)
  if (what == "ALL" || what == "CLUSTERS_RAW")
  {
    auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( "TPOT_CLUSTERS_RAW" ) );
    if( cv )
    {
      pngfile = cl->htmlRegisterPage(*this, "tpot_raw_cluster_info", "2", "png");
      cl->CanvasToPng(cv, pngfile);
    }
  }

  // average cluster information
  if (what == "ALL" || what == "CLUSTERS_AVG")
  {
    auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( "TPOT_CLUSTERS_AVG" ) );
    if( cv )
    {
      pngfile = cl->htmlRegisterPage(*this, "tpot_average_cluster_info", "3", "png");
      cl->CanvasToPng(cv, pngfile);
    }
  }

  // summary page
  if (what == "ALL" || what == "BCO_SUMMARY")
  {
    auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( "TPOT_BCO_SUMMARY" ) );
    if( cv )
    {
      pngfile = cl->htmlRegisterPage(*this, "tpot_bco_summary", "4", "png");
      cl->CanvasToPng(cv, pngfile);
    }
  }

  // summary page
  if (what == "ALL" || what == "SUMMARY")
  {
    auto cv = dynamic_cast<TCanvas*>( gROOT->FindObject( "TPOT_SUMMARY" ) );
    if( cv )
    {
      pngfile = cl->htmlRegisterPage(*this, "tpot_summary", "5", "png");
      cl->CanvasToPng(cv, pngfile);
    }
  }

  return 0;
}

//________________________________________________________________
void MicromegasDraw::setup_cuts_run3_pp()
{

  std::cout << "MicromegasDraw::setup_cuts_run3_pp" << std::endl;

  // acceptable gl1 drop rate
  m_gl1_drop_rate_range = range_list_t(m_npackets_active+1, {0, 0.01});

  // acceptable numbers of good packets for g1l drop rate
  m_packet_gl1_drop_rate_range = {3,3};

  // acceptable per packet waveform drop rate
  m_waveform_drop_rate_range = range_list_t(m_npackets_active+1, {0, 0.05});

  // acceptable numbers of good packets for waveform drop rate
  m_packet_wf_drop_rate_range = {3,3};

  // acceptable per fee waveform drop rate
  m_fee_waveform_drop_rate_range =
  {
    {0, 0.05}, {0, 0.05}, {0, 0.00}, {0, 0.00}, {0, 0.00},
    {0, 0.05}, {0, 0.05}, {0, 0.05}, {0, 0.05}, {0, 0.05},
    {0, 0.00}, {0, 0.00}, {0, 0.05}, {0, 0.00}, {0, 0.05},
    {0, 0.05}, {0, 0.00}, {0, 0.00}, {0, 0.05}, {0, 0.05},
    {0, 0.00}, {0, 0.05}, {0, 0.00}, {0, 0.05}, {0, 0.05},
    {0, 0.05}
  };

  // acceptable numbers of good fee for waveform drop rate
  m_fee_wf_drop_rate_range = {8,13};

  //! acceptable cluster multiplicity range
  m_cluster_multiplicity_range ={
    {1.2,5}, // SCOP
    {1.2,5}, // SCIP
    {1.2,5}, // NCIP
    {1.2,5}, // NCOP
    {1.2,5}, // SEIP
    {1.2,5}, // NEIP
    {1.2,5}, // SWIP
    {1.2,5}, // NWIP
    {0.5,5}, // SCOZ
    {1.2,5}, // SCIZ
    {1.2,5}, // NCIZ
    {1.2,5}, // NCOZ
    {1.2,5}, // SEIZ
    {1.2,5}, // NEIZ
    {1.2,5}, // SWIZ
    {1.2,5} // NWIZ
  };

  //! acceptable numbers of good detectors for cluster multiplicity
  m_detector_cluster_mult_range = {8,15};

  //! acceptable cluster size range
  m_cluster_size_range =
  {
    {2., 3.5}, // SCOP
    {2., 3.5}, // SCIP
    {2., 3.5}, // NCIP
    {2., 3.5}, // NCOP
    {1., 3.5}, // SEIP
    {2., 3.5}, // NEIP
    {2., 3.5}, // SWIP
    {2., 3.5}, // NWIP
    {1.5,3.}, // SCOZ
    {1.5,3.}, // SCIZ
    {1.5,3.}, // NCIZ
    {1.5,3.}, // NCOZ
    {1.,3.}, // SEIZ
    {1.5,3.}, // NEIZ
    {1.5,3.}, // SWIZ
    {1.5,3.} // NWIZ
  };

  //! acceptable numbers of good detectors for cluster size
  m_detector_cluster_size_range = {8,15};

  //! acceptable cluster charge range
  m_cluster_charge_range =
  {
    {400,800}, // SCOP
    {400,800}, // SCIP
    {350,800}, // NCIP
    {400,800}, // NCOP
    {200,800}, // SEIP
    {400,800}, // NEIP
    {400,800}, // SWIP
    {400,800}, // NWIP
    {350,750}, // SCOZ
    {350,750}, // SCIZ
    {350,750}, // NCIZ
    {350,750}, // NCOZ
    {200,750}, // SEIZ
    {350,750}, // NEIZ
    {350,750}, // SWIZ
    {350,750} // NWIZ
  };

  // acceptable numbers of good detectors for cluster charge
  m_detector_cluster_charge_range = {8,15};

  // acceptable efficiency range
  m_efficiency_range =
  {
    {0.65,1.0}, // SCOP
    {0.65,1.0}, // SCIP
    {0.60,1.0}, // NCIP
    {0.60,1.0}, // NCOP
    {0.60,1.0}, // SEIP
    {0.65,1.0}, // NEIP
    {0.50,1.0}, // SWIP
    {0.65,1.0}, // NWIP
    {0.40,1.0}, // SCOZ
    {0.70,1.0}, // SCIZ
    {0.70,1.0}, // NCIZ
    {0.70,1.0}, // NCOZ
    {0.70,1.0}, // SEIZ
    {0.70,1.0}, // NEIZ
    {0.70,1.0}, // SWIZ
    {0.70,1.0}  // NWIZ
  };

  // acceptable numbers of good detectors for efficiency estimate
  m_detector_efficiency_range = {9,15};
}

//________________________________________________________________
void MicromegasDraw::setup_cuts_run3_auau()
{

  std::cout << "MicromegasDraw::setup_cuts_run3_auau" << std::endl;

  // acceptable gl1 drop rate
  m_gl1_drop_rate_range = range_list_t(m_npackets_active+1, {0, 0.01});

  // acceptable numbers of good packets for g1l drop rate
  m_packet_gl1_drop_rate_range = {3,3};

  // acceptable per packet waveform drop rate
  m_waveform_drop_rate_range = range_list_t(m_npackets_active+1, {0, 0.05});

  // acceptable numbers of good packets for waveform drop rate
  m_packet_wf_drop_rate_range = {3,3};

  // acceptable per fee waveform drop rate
  m_fee_waveform_drop_rate_range =
  {
    {0, 0.05}, {0, 0.05}, {0, 0.00}, {0, 0.00}, {0, 0.00},
    {0, 0.05}, {0, 0.05}, {0, 0.05}, {0, 0.05}, {0, 0.05},
    {0, 0.00}, {0, 0.00}, {0, 0.05}, {0, 0.00}, {0, 0.05},
    {0, 0.05}, {0, 0.00}, {0, 0.00}, {0, 0.05}, {0, 0.05},
    {0, 0.00}, {0, 0.05}, {0, 0.00}, {0, 0.05}, {0, 0.05},
    {0, 0.05}
  };

  // acceptable numbers of good fee for waveform drop rate
  m_fee_wf_drop_rate_range = {8,13};

  //! acceptable cluster multiplicity range
  m_cluster_multiplicity_range ={
    {1.5,3.}, // SCOP
    {1.5,3.}, // SCIP
    {1.5,3.}, // NCIP
    {1.5,3.}, // NCOP
    {1.5,3.}, // SEIP
    {1.5,3.}, // NEIP
    {1.5,3.}, // SWIP
    {1.5,3.}, // NWIP
    {1.6,3.2}, // SCOZ
    {1.6,3.2}, // SCIZ
    {1.6,3.2}, // NCIZ
    {1.6,3.2}, // NCOZ
    {1.6,3.2}, // SEIZ
    {1.6,3.2}, // NEIZ
    {1.6,3.2}, // SWIZ
    {1.6,3.2} // NWIZ
  };

  //! acceptable numbers of good detectors for cluster multiplicity
  m_detector_cluster_mult_range = {8,15};

  //! acceptable cluster size range
  m_cluster_size_range =
  {
    {2., 3.5}, // SCOP
    {2., 3.5}, // SCIP
    {2., 3.5}, // NCIP
    {2., 3.5}, // NCOP
    {2., 3.5}, // SEIP
    {2., 3.5}, // NEIP
    {2., 3.5}, // SWIP
    {2., 3.5}, // NWIP
    {1.5,3.}, // SCOZ
    {1.5,3.}, // SCIZ
    {1.5,3.}, // NCIZ
    {1.5,3.}, // NCOZ
    {1.5,3.}, // SEIZ
    {1.5,3.}, // NEIZ
    {1.5,3.}, // SWIZ
    {1.5,3.} // NWIZ
  };

  //! acceptable numbers of good detectors for cluster size
  m_detector_cluster_size_range = {8,15};

  //! acceptable cluster charge range
  m_cluster_charge_range =
  {
    {450,800}, // SCOP
    {450,800}, // SCIP
    {350,800}, // NCIP
    {450,800}, // NCOP
    {400,800}, // SEIP
    {450,800}, // NEIP
    {400,800}, // SWIP
    {450,800}, // NWIP
    {400,750}, // SCOZ
    {400,750}, // SCIZ
    {400,750}, // NCIZ
    {400,750}, // NCOZ
    {350,750}, // SEIZ
    {400,750}, // NEIZ
    {400,750}, // SWIZ
    {400,750} // NWIZ
  };

  // acceptable numbers of good detectors for cluster charge
  m_detector_cluster_charge_range = {8,15};

  // acceptable efficiency range
  m_efficiency_range =
  {
    {0.65,1.0}, // SCOP
    {0.65,1.0}, // SCIP
    {0.60,1.0}, // NCIP
    {0.65,1.0}, // NCOP
    {0.65,1.0}, // SEIP
    {0.65,1.0}, // NEIP
    {0.65,1.0}, // SWIP
    {0.65,1.0}, // NWIP
    {0.40,1.0}, // SCOZ
    {0.70,1.0}, // SCIZ
    {0.70,1.0}, // NCIZ
    {0.70,1.0}, // NCOZ
    {0.70,1.0}, // SEIZ
    {0.70,1.0}, // NEIZ
    {0.70,1.0}, // SWIZ
    {0.70,1.0}  // NWIZ
  };

  // acceptable numbers of good detectors for efficiency estimate
  m_detector_efficiency_range = {9,15};
}
