#pragma once

// ___________________________________________________________________________
// Includes

#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ___________________________________________________________________________
// Class definitions

struct NodeString {
  std::string text;
  double width = 0.0;
};

struct segment {
  double x1 = 0.0, y1 = 0.0, x2 = 0.0, y2 = 0.0;

  bool contains(double x, double y) const;
};

struct pstree {
  std::unique_ptr<pstree> left;
  std::unique_ptr<pstree> right;
  std::vector<NodeString> nodestrings;
  double stringswidth = 0.0, width = 0.0, height = 0.0;
  double x = 0.0, y = 0.0, xbox = 0.0, ybox = 0.0;
  double boxwidth = 0.0, boxheight = 0.0;
  std::vector<segment> seglist;
};

class font {
 private:
  double widths[256] = {};

 public:
  bool load(const std::string& fontname, const std::string& fonts_dir);
  double width(int s) const { return widths[s]; }
};

// ___________________________________________________________________________
// Function declarations

double string_width(const std::string& s, const font& f, double sz);

bool seglists_intersect(const std::vector<segment>& s1,
                        const std::vector<segment>& s2);
bool segments_intersect(const segment& s1, const segment& s2);

std::unique_ptr<pstree> ps_restore_tree(std::istream& is);

void adjust_tree_horizontally(pstree* t, double interspace);
void adjust_tree_vertically(pstree* t);
void move_seglist_horizontally(std::vector<segment>& segs, double delta);
void move_seglist_vertically(std::vector<segment>& segs, double delta);
void move_tree_horizontally(pstree* t, double delta);
void move_tree_vertically(pstree* t, double delta);
void ps_draw_arc(double x0, double y0, double x3, double y3, std::ostream& os);
void ps_draw_box(double x1, double y1, double x2, double y2, std::ostream& os);
void ps_draw_string(const std::string& s, double x, double y,
                    std::ostream& os);
void ps_draw_node(pstree* t, double fontsize, std::ostream& os);
void ps_draw_tree(pstree* t, double fontsize, std::ostream& os);
void set_node_size(pstree* t, const font& mainfont, double fontsize,
                   double interspace);
void set_sizes(pstree* t, const font& mainfont, double fontsize,
               double interspace);

// ___________________________________________________________________________
// pst.h
