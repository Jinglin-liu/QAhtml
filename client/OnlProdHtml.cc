#include "OnlProdHtml.h"

#include "TSystem.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <fstream>
#include <set>
#include <iterator>
#include <dirent.h>

using namespace std;

namespace {

  //___________________________________________________________________________
  vector<string> split(const char sep, const string& s)
  {    
    string str = s;
    std::vector<size_t> slashes_pos;
    
    if ( str[0] != sep ) 
      { 
	str.insert(str.begin(),sep);
      }
    
    if ( str[str.size()-1] != sep ) 
      {
	str.push_back(sep);
      }
    
    for (size_t i = 0 ; i < str.size() ; i++) 
      {
	if ( str[i] == sep ) 
	  { 
	    slashes_pos.push_back(i);
	  }
      }
    
    vector<string> parts;
    
    if ( slashes_pos.size() > 0 ) 
      {
	for (size_t i = 0 ; i < slashes_pos.size()-1 ; i++) 
	  {
	    parts.push_back(str.substr(slashes_pos[i]+1,
				       slashes_pos[i+1]-slashes_pos[i]-1));
	  }
      }  
    
    return parts;
  }

  //___________________________________________________________________________
  string join(const char sep, const vector<string>& parts)
  {
    string rv;
    for ( size_t i = 0; i < parts.size(); ++i ) 
      {
	rv += parts[i];
	if ( i+1 < parts.size() )
	  {
	    rv += sep;
	  }
      }
    return rv;
  }
}

//_____________________________________________________________________________
OnlProdHtml::OnlProdHtml(const char* topdir) : 
  fVerbosity(0),
  runtype("unknown")
{
  if ( topdir ) 
    {
      fHtmlDir = topdir;
    }
  else
    {
      fHtmlDir = "./";
    }
}

//_____________________________________________________________________________
void
OnlProdHtml::addMenu(const string& header, const string& path, 
		    const string& relfilename)
{
  ostringstream menufile;

  menufile << fHtmlRunDir << "/menu";

  ifstream in(menufile.str().c_str());

  if (!in.good())
    {
      if (verbosity())
	{
	  cout << "File " << menufile.str() << " does not exist."
	       << "I'm creating it now" << endl;
	}
      ofstream out(menufile.str().c_str());
      out.close();
    }
  else
    {
      if (verbosity())
	{
	  cout << "Reading file " << menufile.str() << endl;
	}
    }

  // we read back the old menu file...
  vector<string> lines;
  char str[1024];
  while (in.getline(str,1024,'\n'))
    {
      lines.push_back(str);
    }
  in.close();

  // ... we then append the requested new entry...
  ostringstream sline;
  sline << header << "/" << path << "/" << relfilename;

  lines.push_back(sline.str());

  // ... and we sort this out...
  sort(lines.begin(),lines.end());

  // ... and we remove duplicates lines...
  set<string> olines;
  copy(lines.begin(),lines.end(),
       insert_iterator<set<string> >(olines,olines.begin()));

  // ... and finally we write the full new menu file out.
  ofstream out(menufile.str().c_str());
  copy(olines.begin(),olines.end(),ostream_iterator<string>(out,"\n"));
  out.close();

  // --end of normal menu generation--

  // -- For those who do not have javascript (and thus the menu file
  // created by addMenu will be useless) or 
  // in case cgi script(s) won't be allowed for some reason, 
  // make a plain html menu file too.
  plainHtmlMenu(olines);
}

//_____________________________________________________________________________
void
OnlProdHtml::plainHtmlMenu(const set<string>& olines)
{
  ostringstream htmlmenufile;

  htmlmenufile << fHtmlRunDir << "/menu.html";

  // First get the list of directories found in menu file above (contained
  // in olines set). The olines are of the form:
  // D1/D2/TITLE/link (where link is generally somefile.gif)
  // The dir in this case is D1/D2, which is why we look for 2 slashes
  // below (the one before TITLE and the one before link).
  set<string> dirlist;
  set<string>::const_iterator it;
  for ( it = olines.begin(); it != olines.end(); ++it ) 
    {
      const string& line = *it;
      string::size_type pos = line.find_last_of('/');
      pos = line.substr(0,pos).find_last_of('/');
      string dir = line.substr(0,pos);
      vector<string> parts = split('/',dir);
      for ( size_t i = 0; i <= parts.size(); ++i ) 
	{
	  dir = join('/',parts);
	  dirlist.insert(dir);
	  parts.pop_back();
	}
    }

  // We now generate the menu.html file.
  ofstream out(htmlmenufile.str().c_str());
  if (!out.good())
    {
      cerr << " cannot open output file "
	   << htmlmenufile.str() << endl;
      return;
    }

  for ( it = dirlist.begin(); it != dirlist.end(); ++it ) 
    {
      // in the example above, dir is D1/D2
      const string& dir = *it;
      int nslashes = count(dir.begin(),dir.end(),'/')+1;
      string name = dir;
      string::size_type pos = dir.find_last_of('/');
      if ( pos < dir.size() )
	{
	  name = dir.substr(pos+1);
	}
      else
	{
	  out << "<HR><BR>\n";
	}
      out << "<H" << nslashes << ">" << name
	  << "</H" << nslashes << "><BR>\n";

      // We then loop on all the olines, and for those matching the
      // dir pattern, we generate link <A HREF="link">TITLE</A>
      set<string>::const_iterator it2;
      for ( it2 = olines.begin(); it2 != olines.end(); ++it2 ) 
	{
	  const string& line = *it2; 
	  pos = line.find_last_of('/');
	  pos = line.substr(0,pos).find_last_of('/');
	  string ldir = line.substr(0,pos);
	  if ( ldir == dir ) // we get a matching line
	    {
	      string sline = line.substr(dir.size()+1);
	      // in the example above, sline is TITLE/link...
	      pos = sline.find('/');
	      // ...which we split at the slash pos
	      if ( pos < sline.size() )
		{
		  out << "<A HREF=\"" 
		      << sline.substr(pos+1) << "\">"
		      << sline.substr(0,pos) << "</A><BR>\n";
		}
	    }
	}
    }
  out.close();
}

//_____________________________________________________________________________
void
OnlProdHtml::namer(const string& header, 
		   const string& basefilename,
		   const string& ext, 
		   string& fullfilename,
		   string& filename)
{
  ostringstream sfilename;

  sfilename << header << "_";
  if (!basefilename.empty())
    {
      sfilename << basefilename << "_";
    }
  sfilename << runNumber() << "." << ext;

  ostringstream sfullfilename;

  sfullfilename << fHtmlRunDir << "/" << sfilename.str();

  fullfilename = sfullfilename.str();
  filename = sfilename.str();

  if (verbosity())
    {
      cout << "namer: header=" << header
	   << " basefilename=" << basefilename << " ext=" << ext
	   << endl
	   << "fullfilename=" << fullfilename
	   << " filename=" << filename
	   << endl;
    }
}

//_____________________________________________________________________________
string
OnlProdHtml::registerPage(const string& header,
			  const string& path,
			  const string& basefilename,
			  const string& ext)
{
  string fullfilename;
  string filename;
  static string saveheader = "";
  if (saveheader != header)
    {
      runInit();
      saveheader = header;
    }
  namer(header,basefilename,ext,fullfilename,filename);
  addMenu(header,path,filename);
  return fullfilename;
}

//_____________________________________________________________________________
void
OnlProdHtml::runInit()
{
  // Check if html output directory for this run exist.
  // If not create it.
  // Then check (and create if necessary) the "menu" template file.

  ostringstream fulldir;
  fulldir << fHtmlDir << "/" << runtype << "/"
	  << runRange() << "/" << runNumber();

  fHtmlRunDir = fulldir.str();
  DIR *htdir = opendir(fulldir.str().c_str());
  if (!htdir)
    {
      vector<string> mkdirlist;
      mkdirlist.push_back(fulldir.str());
      string updir = fulldir.str();
      string::size_type pos1;
      while ((pos1 = updir.rfind("/") ) != string::npos)
        {
          updir.erase(pos1, updir.size());
          htdir = opendir(updir.c_str());
          if (!htdir)
            {
              mkdirlist.push_back(updir);
            }
          else
            {
              closedir(htdir);
              break;
            }
        }
      while (mkdirlist.rbegin() != mkdirlist.rend())
        {
          string md = *(mkdirlist.rbegin());
          if (verbosity())
            {
              cout << "Trying to create dir " << md << endl;
            }
          if (gSystem->mkdir(md.c_str(), S_IRWXU | S_IRWXG | S_IRWXO))
            {
              cout << "Error creating directory " << md << endl;
              fHtmlRunDir = fHtmlDir;
              break;
            }
          mkdirlist.pop_back();
        }
    }
  else
    {
      closedir(htdir);
    }
  if (verbosity())
    {
      cout << "OK. fHtmlRunDir=" << fHtmlRunDir << endl;
    }
}

//_____________________________________________________________________________
string
OnlProdHtml::runRange()
{
  const int range = 1000;
  int start = runNumber()/range;

  ostringstream s;

  s << "run_" << setw(10) << setfill('0') << start*range
    << "_" << setw(10) << setfill('0') << (start+1)*range;

  return s.str();
}

void
OnlProdHtml:: RunType(const std::string &rtyp)
{
  runtype = rtyp;
  transform(runtype.begin(), runtype.end(), runtype.begin(), (int(*)(int))tolower);
  return;
}
