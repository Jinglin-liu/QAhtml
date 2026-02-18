#include <qahtml/QADrawClient.h>
#include <micromegas/MicromegasDraw.h>
#include <sPhenixStyle.C>
R__LOAD_LIBRARY(libqadrawmicromegas.so)

void draw_micromegas(const std::string &rootfile) {
  SetsPhenixStyle();
  QADrawClient *cl = QADrawClient::instance();
  /* cl->Verbosity(1); */
  QADraw *ex = new MicromegasDraw();

  cl->ReadHistogramsFromFile(rootfile);
  
  cl->registerDrawer(ex);
  /* cl->Print("ALL"); */
  cl->MakeHtml();
  delete cl;
  std::cout << "Done" << std::endl;
  gSystem->Exit(0);
  return ;
}
