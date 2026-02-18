// Jennifer James <jennifer.l.james@vanderbilt.edu>, McKenna Sleeth, Derek Anderson, Mariia Mitrankova

#ifndef BASE_JET_DRAWER_H
#define BASE_JET_DRAWER_H

#include "JetDrawDefs.h"
#include "TFile.h"
#include <string>

class QADraw;
class TFile;

// ============================================================================
//! Base class for jet QA components
// ============================================================================
/*! Base class to define common functionality across
 *  the different components of the jet QA. Each jet
 *  QA module should have a corresponding "drawer"
 *  class inheriting from this one.
 */
class BaseJetDrawer
{
  public:

    // ctor/dtor
    BaseJetDrawer(const std::string& name = "Base",
                  const std::string& modu = "JetDraw",
                  const std::string& type = "towersub1_antikt",
                  const std::string& pref = "h_eventwisecalorho",
                  const bool debug = false,
                  const bool local = false);
    ~BaseJetDrawer() {};

    // public methods to be implemented
    virtual int Draw(const std::vector<uint32_t> vecTrigToDraw,
                     const std::vector<uint32_t> vecResToDraw);
    virtual int MakeHtml(const std::vector<uint32_t> vecTrigToDraw,
                         const std::vector<uint32_t> vecResToDraw,
                         const QADraw& subsystem);

    // setters
    void SetDoDebug(const bool debug) {m_do_debug = debug;}
    void SetDoLocal(const bool local) {m_do_local = local;}
    void SetName(const std::string& name) {m_name = name;}
    void SetJetType(const std::string& type) {m_jet_type = type;}
    void SetHistPrefix(const std::string& prefix) {m_hist_prefix = prefix;}

    // getters
    bool GetDoDebug() const {return m_do_debug;}
    bool GetDoLocal() const {return m_do_local;}
    std::string GetName() const {return m_name;}
    std::string GetJetType() const {return m_jet_type;}
    std::string GetHistPrefix() const {return m_hist_prefix;}

    // other public methods
    void SaveToFile(TFile* file);

  protected:

    ///! name of component
    std::string m_name;

    ///! module name
    std::string m_module;

    ///! type of jet input (used in histogram names)
    std::string m_jet_type;

    ///! prefix of qa histograms
    std::string m_hist_prefix;

    ///! turn debugging statements on/off
    bool m_do_debug;

    ///! turn local drawing mode on/off
    bool m_do_local;

    ///! matrix of PlotPads (canvas + pads)
    JetDrawDefs::PlotPadMatrix m_plots;

    // protected methods to be implemented
    virtual void DoDrawing(const uint32_t trig, const uint32_t res);

    // other protected methods
    void DrawRunAndBuild(const std::string& what,
                         TPad* pad,
                         const int trig = -1,
                         const int res = -1);
    void DrawHists(const std::string& what,
                   const std::vector<std::size_t>& indices,
                   const JetDrawDefs::VHistAndOpts1D& hists,
                   const int trig = -1,
                   const int res = -1);
    void DrawHistOnPad(const std::size_t iHist,
                       const std::size_t iPad,
                       const JetDrawDefs::VHistAndOpts1D& hists,
                       JetDrawDefs::PlotPads& plot);
    void DrawEmptyHistogram(const std::string& what = "histogram");
    void MakeCanvas(const std::string& name, const int nHist);
    void UpdatePadStyle(const JetDrawDefs::HistAndOpts& hist);
    void UpdateTitle(const JetDrawDefs::HistAndOpts& hist);
    void NormalizeHist(const JetDrawDefs::HistAndOpts& hist);

    JetDrawDefs::VHistAndOpts1D BuildRefHists(const JetDrawDefs::VHistAndOpts1D& hists);

    void DrawTextOnPad(const std::size_t iPad,
                       JetDrawDefs::PlotPads& plot,
                       double x,
                       double y,
                       int color,
                       const std::string& text,
                       double tsize = 0.04);

    // QA reference file path
    std::string refFilePath;
    std::string refRunNum;

};  // end BaseJetDrawer

#endif
