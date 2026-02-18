// Jennifer James <jennifer.l.james@vanderbilt.edu>, McKenna Sleeth, Derek Anderson, Mariia Mitrankova

#include "CaloJetDraw.h"
#include "BaseJetDrawer.h"
#include "BeamBackgroundDrawer.h"
#include "DijetQADrawer.h"
#include "EventRhoDrawer.h"
#include "JetCstDrawer.h"
#include "JetKinematicDrawer.h"
#include "JetSeedDrawer.h"
#include "EMClusterKinematicDrawer.h"
#include "StatusMapDrawer.h"
#include "UEvsEtaDrawer.h"
#include <map>
#include <memory>

// ctor/dtor ==================================================================

// ----------------------------------------------------------------------------
//! Default ctor
// ----------------------------------------------------------------------------
/*! Which components to implement are set here. Currently
 *  implemented ones:
 *    - "BEAM" = draw beam background plots,
 *    - "STATUS" = draw calo status map plots,
 *    - "EMCLUSTER" = draw emcluster kinematic plots,
 *    - "RHO" = draw event-wise rho plots,
 *    - "KINEMATIC" = draw jet kinematic plots,
 *    - "CONSTITUENTS" = draw jet calorimeter constituent plots,
 *    - "SEED" = draw jet seed plots (AuAu only),
 *    - "DIJET" = draw dijet qa plots,
 *    - "UEETA" = draw underlying event vs eta plots,
 *    - "ALL" = draw all of the above.
 *
 *  Note that "ALL" doesn't have an associated drawer class, since
 *  it just indicates to run all of the above drawers.
 */
CaloJetDraw::CaloJetDraw(const std::string& name,
                         const std::string& type,
                         const bool debug)
  : BaseJetDraw(name, type, debug)
{
  // for beam background plots
  m_drawers["BEAM"] = std::make_unique<BeamBackgroundDrawer>("BeamBackground",
                                                             name,
                                                             "",
                                                             "h_beambackgroundfilterandqa",
                                                             debug);

  // for status map plots
  m_drawers["STATUS"] = std::make_unique<StatusMapDrawer>("CaloStatusMap",
                                                          name,
                                                          "towerinfo_calib",
                                                          "h_calostatusmapper",
                                                          debug);

  // for photon kinematic plots
  m_drawers["EMCLUSTER"] = std::make_unique<EMClusterKinematicDrawer>("EMClusterKinematics",
                                                                name,
                                                                "emcal_cluster",
                                                                "h_emclusterkinematics",
                                                                debug);

  // for event-wise rho plots
  m_drawers["RHO"] = std::make_unique<EventRhoDrawer>("EventRho",
                                                      name,
                                                      type,
                                                      "h_eventwisecalorho",
                                                      debug);

  // for jet kinematic plots
  m_drawers["KINEMATIC"] = std::make_unique<JetKinematicDrawer>("JetKinematics",
                                                                name,
                                                                type,
                                                                "h_jetkinematiccheck",
                                                                debug);
  // for dijet plots
  m_drawers["DIJET"] = std::make_unique<DijetQADrawer>("DijetQA",
                                                       name,
                                                       type,
                                                       "h_dijetqa",
                                                       debug);

  // for jet seed plots
  if (!m_is_pp)
  {
    m_drawers["SEED"] = std::make_unique<JetSeedDrawer>("JetSeed",
                                                        name,
                                                        type,
                                                        "h_jetseedcount",
                                                        debug);
  }

  // for calo jet constituent plots
  m_drawers["CONSTITUENTS"] = std::make_unique<JetCstDrawer>("JetCst",
                                                             name,
                                                             type,
                                                             "h_constituentsinjets",
                                                             debug);

  // for underlying event vs eta plots
  if (!m_is_pp)
  {
    m_drawers["UEETA"] = std::make_unique<UEvsEtaDrawer>("UEvsEta",
                                                        name,
                                                        type,
                                                        "h_uevsetacent",
                                                        debug);
  }


}

// ----------------------------------------------------------------------------
//! Default dtor
// ----------------------------------------------------------------------------
CaloJetDraw::~CaloJetDraw() {};
