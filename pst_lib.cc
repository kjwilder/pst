
// ___________________________________________________________________________
// Includes and defines

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "pst.h"

// ___________________________________________________________________________

void move_seglist_horizontally(std::vector<segment>& segs, double delta)
{
  for (auto& s : segs) {
    s.x1 += delta;
    s.x2 += delta;
  }

} // move_seglist_horizontally

// ___________________________________________________________________________

void move_seglist_vertically(std::vector<segment>& segs, double delta)
{
  for (auto& s : segs) {
    s.y1 += delta;
    s.y2 += delta;
  }

} // move_seglist_vertically

// ___________________________________________________________________________

void move_tree_horizontally(pstree* t, double delta)
{
  if (t)
  {
    t->x += delta;
    t->xbox += delta;
    if (t->left && t->right) {
      move_tree_horizontally(t->left.get(), delta);
      move_tree_horizontally(t->right.get(), delta);
    }
  }

} // move_tree_horizontally

// ___________________________________________________________________________

void move_tree_vertically(pstree* t, double delta)
{
  if (t)
  {
    t->y += delta;
    t->ybox += delta;
    if (t->left && t->right) {
      move_tree_vertically(t->left.get(), delta);
      move_tree_vertically(t->right.get(), delta);
    }
  }

} // move_tree_vertically

// ___________________________________________________________________________

void adjust_tree_horizontally(pstree* t, double interspace)
{
  // do a bisection search
  pstree* l = t->left.get();
  pstree* r = t->right.get();
  if (l && r)
  {
    double xmin = l->xbox - r->xbox;
    double xmax = l->x + l->width - r->x + interspace;
    double xlast = 0;
    while (xmax - xmin > 1.0) {
      double xmid = (xmin + xmax) / 2.0;
      double delta = xmid - xlast;
      move_tree_horizontally(r, delta);
      move_seglist_horizontally(r->seglist, delta);
      xlast = xmid;
      if (r->xbox <= l->xbox || seglists_intersect(l->seglist, r->seglist))
	xmin = xmid;
      else
	xmax = xmid;
    }
    move_tree_horizontally(r, interspace);
    move_seglist_horizontally(r->seglist, interspace);
  }

} // adjust_tree_horizontally

// ___________________________________________________________________________

void adjust_tree_vertically(pstree* t)
{
  pstree* l = t->left.get();
  pstree* r = t->right.get();
  if (l && r)
  {
    double y_left = l->y + l->height;
    double y_right = r->y + r->height;
    if (y_left < y_right) {
      double delta = y_right - y_left;
      move_tree_vertically(l, delta);
      move_seglist_vertically(l->seglist, delta);
    }
    else if (y_right < y_left) {
      double delta = y_left - y_right;
      move_tree_vertically(r, delta);
      move_seglist_vertically(r->seglist, delta);
    }
  }

} // adjust_tree_vertically

// ___________________________________________________________________________

void ps_draw_arc(double x0, double y0, double x3, double y3, std::ostream& os)
{
  double y1 = (y0 + y3) / 2.0;
  os << std::setprecision(2);
  os << "np " << x0 << " " << y0 << " mt "
     << x0 << " " << y1 << " "
     << x3 << " " << y1 << " "
     << x3 << " " << y3 << " ct sk\n";

} // ps_draw_arc

// ___________________________________________________________________________

void ps_draw_box(double x1, double y1, double x2, double y2, std::ostream& os)
{
  os << std::setprecision(2);
  os << "np " << x1 << " " << y1 << " mt "
     << x2 << " " << y1 << " lt "
     << x2 << " " << y2 << " lt "
     << x1 << " " << y2 << " lt cp er sk\n";

} // ps_draw_box

// ___________________________________________________________________________

void ps_draw_string(const std::string& s, double x, double y, std::ostream& os)
{
  os << std::setprecision(2);
  os << x << " " << y << " mt (";
  for (char c : s) {
    if (c == '(' || c == ')' || c == '\\')
      os << '\\';
    os << c;
  }
  os << ") sh\n";

} // ps_draw_string

// ___________________________________________________________________________

void ps_draw_node(pstree* t, double fontsize, std::ostream& os)
{
  double x1 = t->xbox - t->stringswidth / 2.0 - 0.2 * fontsize;
  double x2 = t->xbox + t->stringswidth / 2.0 + 0.2 * fontsize;
  double y2 = t->ybox + 0.8 * fontsize;
  double y1 = y2 - t->boxheight;
  ps_draw_box(x1, y1, x2, y2, os);

  x1 = t->xbox;
  y1 = t->ybox - 0.4 * fontsize;
  for (const auto& ns : t->nodestrings) {
    ps_draw_string(ns.text, x1 - ns.width / 2.0, y1, os);
    y1 -= 1.2 * fontsize;
  }

} // ps_draw_node

// ___________________________________________________________________________

void ps_draw_tree(pstree* t, double fontsize, std::ostream& os)
{
  if (t && !t->nodestrings.empty())
  {
    if (t->left && t->right) {
      ps_draw_arc(t->xbox, t->ybox, t->left->xbox, t->left->ybox, os);
      ps_draw_tree(t->left.get(), fontsize, os);
      ps_draw_arc(t->xbox, t->ybox, t->right->xbox, t->right->ybox, os);
      ps_draw_tree(t->right.get(), fontsize, os);
    }
    ps_draw_node(t, fontsize, os);
  }

} // ps_draw_tree

// ___________________________________________________________________________

std::unique_ptr<pstree> ps_restore_tree(std::istream& is)
{
  int nodetype;
  char c;
  std::string line;
  std::unique_ptr<pstree> thisnode;
  switch (nodetype = is.get())
  {
   case 'B':
   case 'L':
    thisnode = std::make_unique<pstree>();
    std::getline(is, line);
    thisnode->nodestrings.push_back({line});
    while ((c = static_cast<char>(is.get())) == '+')
    {
      std::getline(is, line);
      thisnode->nodestrings.push_back({line});
    }
    is.putback(c);
    if (nodetype == 'B') {
      thisnode->left = ps_restore_tree(is);
      thisnode->right = ps_restore_tree(is);
    }
    break;

   default:
    std::cout << "This is not a proper tree data file\n";
    return nullptr;
  }

  return thisnode;

} // ps_restore_tree

// ___________________________________________________________________________

bool seglists_intersect(const std::vector<segment>& s1,
                        const std::vector<segment>& s2)
{
  for (const auto& t1 : s1)
    for (const auto& t2 : s2)
      if (segments_intersect(t1, t2))
	return true;
  return false;

} // seglists_intersect

// ___________________________________________________________________________

bool segment::contains(double x, double y) const
{
  bool result;

  if (x1 < x2)
    result = (x1 <= x && x <= x2);
  else
    result = (x2 <= x && x <= x1);

  if (result) {
    if (y1 < y2)
      result = (y1 <= y && y <= y2);
    else
      result = (y2 <= y && y <= y1);
  }

  return result;

} // segment::contains

// ___________________________________________________________________________

bool segments_intersect(const segment& s1, const segment& s2)
{
  if (s1.x1 == s1.x2)
  {
    if (s2.x1 == s2.x2)
    {
      if (s1.x1 == s2.x1)
      {
	if (s1.y1 <= s1.y2)
	  return ((s1.y1 <= s2.y1 && s2.y1 <= s1.y2) ||
                  (s1.y1 <= s2.y2 && s2.y2 <= s1.y2));
	else
	  return ((s1.y2 <= s2.y1 && s2.y1 <= s1.y1) ||
                  (s1.y2 <= s2.y2 && s2.y2 <= s1.y1));
      }
      else
        return false;
    }
    else {
      double m = (s2.y1 - s2.y2) / (s2.x1 - s2.x2);
      double y = m * s1.x1 + s2.y1 - m * s2.x1;
      return (s1.contains(s1.x1, y) && s2.contains(s1.x1, y));
    }
  }
  else
  {
    if (s2.x1 == s2.x2) {
      double m = (s1.y2 - s1.y1) / (s1.x2 - s1.x1);
      double y = m * s2.x1 + s1.y1 - m * s1.x1;
      return (s1.contains(s2.x1, y) && s2.contains(s2.x1, y));
    }
    else
    {
      double m1 = (s1.y2 - s1.y1) / (s1.x2 - s1.x1);
      double b1 = s1.y1 - m1 * s1.x1;
      double m2 = (s2.y2 - s2.y1) / (s2.x2 - s2.x1);
      double b2 = s2.y1 - m2 * s2.x1;
      if (m1 == m2)
      {
	if (b1 == b2)
	{
	  if (s1.x1 < s1.x2)
	    return ((s1.x1 <= s2.x1 && s2.x1 <= s1.x2) ||
                    (s1.x1 <= s2.x2 && s2.x2 <= s1.x2));
	  else
	    return ((s1.x2 <= s2.x1 && s2.x1 <= s1.x1) ||
                    (s1.x2 <= s2.x2 && s2.x2 <= s1.x1));
	}
	else
	  return false;
      }
      else {
	double x = (b2 - b1) / (m1 - m2);
	double y = m1 * x + b1;
	return (s1.contains(x, y) && s2.contains(x, y));
      }
    }
  }

} // segments_intersect

// ___________________________________________________________________________

void set_node_size(pstree* t, const font& mainfont,
                   double fontsize, double interspace)
{
  double width = 0.0, height = 0.0;
  for (auto& ns : t->nodestrings) {
    ns.width = string_width(ns.text, mainfont, fontsize);
    if (ns.width > width)
      width = ns.width;
    height += fontsize;
  }

  t->boxwidth = width + 0.4 * fontsize;
  t->boxheight = 1.2 * height + 0.4 * fontsize;
  t->stringswidth = width;

  adjust_tree_vertically(t);
  adjust_tree_horizontally(t, interspace);

  double half_width = t->boxwidth / 2.0;
  double half_height = t->boxheight / 2.0;
  if (t->left && t->right) {
    double xmin = t->left->x;
    double xmax = xmin + t->left->width;
    double ymin = t->left->y;
    double ymax = ymin + t->left->height;
    if (t->right->x < xmin)
      xmin = t->right->x;
    double x = t->right->x + t->right->width;
    if (x > xmax)
      xmax = x;
    if (t->right->y < ymin)
      ymin = t->right->y;
    double y = t->right->y + t->right->height;
    if (y > ymax)
      ymax = y;

    // position the box w.r.t. roots of subtrees
    double left_center = t->left->xbox;
    double right_center = t->right->xbox;
    t->xbox = (left_center + right_center) / 2.0;
    if (right_center > left_center)
      t->ybox = ymax + 1.2 * fontsize * log(right_center - left_center) +
        (height - fontsize);
    else
      t->ybox = ymax + 4.8 * fontsize + (height - fontsize);
    ymax = t->ybox + half_height;

    if (t->xbox + half_width > xmax)
      xmax = t->xbox + half_width;
    if (t->xbox - half_width < xmin)
      xmin = t->xbox - half_width;

    t->x = xmin;
    t->y = ymin;
    t->width = xmax - xmin;
    t->height = ymax - ymin;

    // Transfer child seglists into this node's seglist
    t->seglist = std::move(t->left->seglist);
    t->seglist.insert(t->seglist.end(),
                      std::make_move_iterator(t->right->seglist.begin()),
                      std::make_move_iterator(t->right->seglist.end()));
    t->right->seglist.clear();

    // create segments somewhat above the arcs that connects the nodes
    t->seglist.push_back({t->left->xbox - t->left->boxwidth / 2.0,
                          t->left->ybox + 0.8 * fontsize,
                          t->xbox - half_width,
                          t->ybox + half_height});
    t->seglist.push_back({t->right->xbox + t->right->boxwidth / 2.0,
                          t->right->ybox + 0.8 * fontsize,
                          t->xbox + half_width,
                          t->ybox + half_height});
  }
  else {
    t->x = t->y = 0.0;
    t->width = t->boxwidth;
    t->height = t->boxheight;
    t->xbox = t->width / 2.0;
    t->ybox = t->height + 0.4 * fontsize;
  }

  // every side of test box is also a segment
  t->seglist.push_back({t->xbox - half_width, t->ybox - half_height,
                         t->xbox - half_width, t->ybox + half_height});
  t->seglist.push_back({t->xbox + half_width, t->ybox - half_height,
                         t->xbox + half_width, t->ybox + half_height});
  t->seglist.push_back({t->xbox - half_width, t->ybox - half_height,
                         t->xbox + half_width, t->ybox - half_height});
  t->seglist.push_back({t->xbox - half_width, t->ybox + half_height,
                         t->xbox + half_width, t->ybox + half_height});

} // set_node_size

// ___________________________________________________________________________

void set_sizes(pstree* t, const font& mainfont, double fontsize,
               double interspace)
{
  if (t->left && t->right) {
    set_sizes(t->left.get(), mainfont, fontsize, interspace);
    set_sizes(t->right.get(), mainfont, fontsize, interspace);
  }
  set_node_size(t, mainfont, fontsize, interspace);

} // set_sizes

// ___________________________________________________________________________

bool font::load(const std::string& fontname, const std::string& fonts_dir)
{
  std::string filename = fonts_dir + "/" + fontname + ".nfm";

  std::ifstream ifs(filename);
  if (!ifs)
    return false;

  int w;
  for (int i = 0; i < 256; i++) {
    ifs >> w;
    widths[i] = w / 1000.0;
  }
  return true;

} // font::load

// ___________________________________________________________________________

double string_width(const std::string& s, const font& f, double sz)
{
  double total = 0.0;
  for (char c : s)
    total += f.width(static_cast<unsigned char>(c));
  return (sz * total);

} // string_width

// ___________________________________________________________________________
// pst_lib.cc
