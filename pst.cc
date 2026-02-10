
// ___________________________________________________________________________
// Includes and defines

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "pst.h"

const std::string kFontsPathName = "/Users/ken/github/pst/fonts";

// ___________________________________________________________________________

int main(int argc, char** argv)
{
  const char* filename = nullptr;
  std::string fontname = "Helvetica-Narrow";
  double fontsize = 6.0;
  bool have_file_name = false;

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
      switch (argv[i][1])
      {
	case 'f':
	  fontname = &argv[i][2];
	  break;
	case 's':
	  fontsize = std::stod(&argv[i][2]);
	  break;
	default:
	  std::cout << "Unrecognized option " << argv[i][1] << "\n";
      }
    else {
      filename = argv[i];
      have_file_name = true;
    }
  }

  if (!have_file_name) {
    std::cout << "Usage: pst [-ffontname] [-ssize] treefile\n";
    return 1;
  }

  font mainfont;
  if (!mainfont.load(fontname, kFontsPathName)) {
    std::cout << "Unable to load font " << fontname << "\n";
    return 2;
  }

  std::ifstream ifp(filename);
  if (!ifp) {
    std::cout << "Unable to read tree from file " << filename << "\n";
    return 3;
  }
  auto tree = ps_restore_tree(ifp);
  ifp.close();

  std::string outname = std::string(filename) + ".ps";
  std::ofstream ofp(outname);
  if (!ofp) {
    std::cout << "Unable to write file " << outname << "\n";
    return 4;
  }

  ofp << std::fixed;

  ofp << "%!PS-Adobe-\n";
  ofp << "\n";
  ofp << "/bc {dup stringwidth -2 div exch -2 div exch rmt show} def\n";
  ofp << "/bk {0 setgray} def\n\n";
  ofp << "/cp {closepath} def\n";
  ofp << "/ct {curveto} def\n";
  ofp << "/er {gsave 1 setgray fill grestore} def\n";
  ofp << "/gr {grestore} def\n";
  ofp << "/gs {gsave} def\n";
  ofp << "/gy {0.4 setgray} def\n";
  ofp << "/lt {lineto} def\n";
  ofp << std::setprecision(2);
  ofp << "/mf {/" << fontname << " findfont " << fontsize
      << " scalefont setfont} def\n";
  ofp << "/mt {moveto} def\n";
  ofp << "/np {newpath} def\n";
  ofp << "/rf {/Helvetica findfont 8.0 scalefont setfont} def\n";
  ofp << "/rlt {rlineto} def\n";
  ofp << "/rmt {rmoveto} def\n";
  ofp << "/sh {show} def\n";
  ofp << "/sk {stroke} def\n";
  ofp << "/slw {setlinewidth} def\n";
  ofp << "/tr {translate} def\n";
  ofp << "/wh {1 setgray} def\n";

  std::cout << " ok\nSetting coordinates ...";
  std::cout.flush();
  set_sizes(tree.get(), mainfont, fontsize, 1.5 * fontsize);
  std::cout << " ok\n";

  // compute orientation on page(s)
  int w1 = static_cast<int>((tree->width + 540 - 1) / 540);
  int h1 = static_cast<int>((tree->height + 720 - 1) / 720);
  int w2 = static_cast<int>((tree->width + 720 - 1) / 720);
  int h2 = static_cast<int>((tree->height + 540 - 1) / 540);
  int wpages, pwidth, pheight, hpages, orientation;
  if (w2 * h2 > w1 * h1) {
    wpages = w1;
    pwidth = 540;
    pheight = 720;
    hpages = h1;
    orientation = 1;
  }
  else {
    wpages = w2;
    pwidth = 720;
    pheight = 540;
    hpages = h2;
    orientation = 2;
  }
  int tpages = hpages * wpages;

  ofp << std::setprecision(2);
  ofp << "% width: " << tree->width << " height: " << tree->height << "\n";
  ofp << "% wpages: " << wpages << ", hpages: " << hpages
      << ", total: " << tpages << "\n";
  ofp << "/sclip {np 0 0 mt 0 " << pheight << " rlt " << pwidth
      << " 0 rlt 0 " << pheight << " neg rlt cp clip} def\n";
  if (tpages > 1)
    std::cout << "Drawing tree onto " << tpages << " pages ("
              << hpages << " tall by " << wpages << " wide)\n";
  else
    std::cout << "Drawing tree onto 1 page\n";

  // use as many pages as needed, and provide cutting gluing directions
  int i = 0;
  for (int rowcount = 0; rowcount < hpages; rowcount++)
    for (int colcount = 0; colcount < wpages; colcount++)
    {
      i++;
      std::cout << " " << i;
      std::cout.flush();

      ofp << "\n";
      if (orientation == 2)
	ofp << "90 rotate 0 612 neg tr ";
      ofp << std::setprecision(5);
      ofp << "36 36 tr " << fontsize / 10.0 << " slw\n";
      ofp << std::setprecision(3);

      if (hpages - rowcount - 1) {
	ofp << "gs rf 1 slw\n";
	ofp << pwidth / 2.0 << " " << pheight + 10
            << " mt (row " << hpages - rowcount
            << " - cut to remove line"
            << ", place to cover line of adjoining page) bc\n";
	ofp << "np -36 " << 0.50 + pheight
            << " mt " << pwidth + 72 << " 0 rlt sk gr\n";
      }
      if (rowcount) {
	ofp << "gs rf 1 slw\n";
	ofp << "gy " << pwidth / 2.0
            << " -18 mt (place adjoining page of"
            << " row " << hpages - rowcount + 1
            << " to cover line) bc\n";
	ofp << "np -36 -0.5 mt " << pwidth + 72 << " 0 rlt sk gr\n";
      }
      if (colcount) {
	ofp << "gs rf 1 slw\n";
	ofp << "-10 " << pheight / 2.0
            << " mt gs 90 rotate (col " << colcount + 1
            << " - cut to"
            << " remove line, place to cover line of adjoining page) "
            << "bc gr\n";
	ofp << "np -0.5 -36 mt 0 " << pheight + 72 << " rlt sk gr\n";
      }
      if (colcount < wpages - 1) {
	ofp << "gs rf 1 slw\n";
	ofp << "gy " << pwidth + 18 << " " << pheight / 2.0
            << " mt gs 90 rotate (place adjoining "
            << "page of column " << colcount + 2
            << " to cover line) bc gr\n";
	ofp << "np " << 0.50 + pwidth << " -36 mt 0 "
            << pheight + 72 << " rlt sk gr\n";
      }
      ofp << "gs sclip " << (pwidth * wpages - tree->width) / 2.0 -
              pwidth * colcount - tree->x
          << " " << (pheight * hpages - tree->height) / 2.0 -
              pheight * rowcount - tree->y
          << " tr mf\n";

      ofp << std::setprecision(2);
      ps_draw_tree(tree.get(), fontsize, ofp);
      ofp << "gr showpage\n";
    }
  std::cout << "\n";
  ofp.close();

  return 0;

} // main

// ___________________________________________________________________________
// pst.cc
