#include "pst.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

// ___________________________________________________________________________
// Helper: locate runfiles (Bazel test data)

static std::string TestDataPath(const std::string& filename) {
  // Bazel sets TEST_SRCDIR and TEST_WORKSPACE for runfiles
  const char* srcdir = std::getenv("TEST_SRCDIR");
  const char* workspace = std::getenv("TEST_WORKSPACE");
  if (srcdir && workspace)
    return std::string(srcdir) + "/" + workspace + "/" + filename;
  // Fallback for running outside Bazel
  return filename;
}

static std::string FontsDir() {
  return TestDataPath("fonts");
}

// ___________________________________________________________________________
// segment::contains tests

TEST(SegmentContains, PointInsideSegment) {
  segment s{0, 0, 10, 10};
  EXPECT_TRUE(s.contains(5, 5));
}

TEST(SegmentContains, PointAtEndpoints) {
  segment s{0, 0, 10, 10};
  EXPECT_TRUE(s.contains(0, 0));
  EXPECT_TRUE(s.contains(10, 10));
}

TEST(SegmentContains, PointOutsideBothAxes) {
  segment s{0, 0, 10, 10};
  EXPECT_FALSE(s.contains(15, 15));
  EXPECT_FALSE(s.contains(-1, -1));
}

TEST(SegmentContains, PointOutsideXOnly) {
  segment s{0, 0, 10, 10};
  EXPECT_FALSE(s.contains(15, 5));
}

TEST(SegmentContains, PointOutsideYOnly) {
  segment s{0, 0, 10, 10};
  EXPECT_FALSE(s.contains(5, 15));
}

TEST(SegmentContains, ReversedSegment) {
  segment s{10, 10, 0, 0};
  EXPECT_TRUE(s.contains(5, 5));
  EXPECT_FALSE(s.contains(15, 5));
}

TEST(SegmentContains, HorizontalSegment) {
  segment s{0, 5, 10, 5};
  EXPECT_TRUE(s.contains(5, 5));
  EXPECT_FALSE(s.contains(5, 6));
}

TEST(SegmentContains, VerticalSegment) {
  segment s{5, 0, 5, 10};
  EXPECT_TRUE(s.contains(5, 5));
  EXPECT_FALSE(s.contains(6, 5));
}

// ___________________________________________________________________________
// segments_intersect tests

TEST(SegmentsIntersect, CrossingSegments) {
  segment s1{0, 0, 10, 10};
  segment s2{0, 10, 10, 0};
  EXPECT_TRUE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, NonCrossingSegments) {
  segment s1{0, 0, 10, 0};
  segment s2{0, 5, 10, 5};
  EXPECT_FALSE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, ParallelOverlapping) {
  segment s1{0, 0, 10, 0};
  segment s2{5, 0, 15, 0};
  EXPECT_TRUE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, ParallelNonOverlapping) {
  segment s1{0, 0, 4, 0};
  segment s2{5, 0, 10, 0};
  EXPECT_FALSE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, VerticalCrossingHorizontal) {
  segment s1{5, 0, 5, 10};
  segment s2{0, 5, 10, 5};
  EXPECT_TRUE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, VerticalMissingHorizontal) {
  segment s1{5, 0, 5, 3};
  segment s2{0, 5, 10, 5};
  EXPECT_FALSE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, BothVerticalSameX) {
  segment s1{5, 0, 5, 10};
  segment s2{5, 5, 5, 15};
  EXPECT_TRUE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, BothVerticalDifferentX) {
  segment s1{5, 0, 5, 10};
  segment s2{6, 0, 6, 10};
  EXPECT_FALSE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, TouchingAtEndpoint) {
  segment s1{0, 0, 5, 5};
  segment s2{5, 5, 10, 0};
  EXPECT_TRUE(segments_intersect(s1, s2));
}

TEST(SegmentsIntersect, TShapeIntersection) {
  // Vertical segment meets horizontal segment at its midpoint
  segment s1{5, 0, 5, 5};
  segment s2{0, 5, 10, 5};
  EXPECT_TRUE(segments_intersect(s1, s2));
}

// ___________________________________________________________________________
// seglists_intersect tests

TEST(SeglistsIntersect, EmptyLists) {
  std::vector<segment> s1, s2;
  EXPECT_FALSE(seglists_intersect(s1, s2));
}

TEST(SeglistsIntersect, OneEmptyList) {
  std::vector<segment> s1 = {{0, 0, 10, 10}};
  std::vector<segment> s2;
  EXPECT_FALSE(seglists_intersect(s1, s2));
}

TEST(SeglistsIntersect, IntersectingLists) {
  std::vector<segment> s1 = {{0, 0, 10, 10}};
  std::vector<segment> s2 = {{0, 10, 10, 0}};
  EXPECT_TRUE(seglists_intersect(s1, s2));
}

TEST(SeglistsIntersect, NonIntersectingLists) {
  std::vector<segment> s1 = {{0, 0, 10, 0}};
  std::vector<segment> s2 = {{0, 5, 10, 5}};
  EXPECT_FALSE(seglists_intersect(s1, s2));
}

// ___________________________________________________________________________
// ps_restore_tree tests

TEST(PsRestoreTree, SingleLeaf) {
  std::istringstream input("LHello World\n");
  auto tree = ps_restore_tree(input);
  ASSERT_NE(tree, nullptr);
  EXPECT_EQ(tree->nodestrings.size(), 1);
  EXPECT_EQ(tree->nodestrings[0].text, "Hello World");
  EXPECT_EQ(tree->left, nullptr);
  EXPECT_EQ(tree->right, nullptr);
}

TEST(PsRestoreTree, LeafWithMultipleLines) {
  std::istringstream input("LFirst Line\n+Second Line\n+Third Line\n");
  auto tree = ps_restore_tree(input);
  ASSERT_NE(tree, nullptr);
  EXPECT_EQ(tree->nodestrings.size(), 3);
  EXPECT_EQ(tree->nodestrings[0].text, "First Line");
  EXPECT_EQ(tree->nodestrings[1].text, "Second Line");
  EXPECT_EQ(tree->nodestrings[2].text, "Third Line");
  EXPECT_EQ(tree->left, nullptr);
  EXPECT_EQ(tree->right, nullptr);
}

TEST(PsRestoreTree, SimpleBranch) {
  std::istringstream input("BParent\nLLeft\nLRight\n");
  auto tree = ps_restore_tree(input);
  ASSERT_NE(tree, nullptr);
  EXPECT_EQ(tree->nodestrings.size(), 1);
  EXPECT_EQ(tree->nodestrings[0].text, "Parent");
  ASSERT_NE(tree->left, nullptr);
  ASSERT_NE(tree->right, nullptr);
  EXPECT_EQ(tree->left->nodestrings[0].text, "Left");
  EXPECT_EQ(tree->right->nodestrings[0].text, "Right");
}

TEST(PsRestoreTree, NestedBranch) {
  std::istringstream input("BRoot\nBMid\nLLL\nLLR\nLR\n");
  auto tree = ps_restore_tree(input);
  ASSERT_NE(tree, nullptr);
  EXPECT_EQ(tree->nodestrings[0].text, "Root");

  ASSERT_NE(tree->left, nullptr);
  EXPECT_EQ(tree->left->nodestrings[0].text, "Mid");

  ASSERT_NE(tree->left->left, nullptr);
  EXPECT_EQ(tree->left->left->nodestrings[0].text, "LL");

  ASSERT_NE(tree->left->right, nullptr);
  EXPECT_EQ(tree->left->right->nodestrings[0].text, "LR");

  ASSERT_NE(tree->right, nullptr);
  EXPECT_EQ(tree->right->nodestrings[0].text, "R");
}

TEST(PsRestoreTree, Sample2File) {
  std::string path = TestDataPath("testdata/sample2.txt");
  std::ifstream ifs(path);
  ASSERT_TRUE(ifs.good()) << "Cannot open " << path;
  auto tree = ps_restore_tree(ifs);
  ASSERT_NE(tree, nullptr);
  EXPECT_EQ(tree->nodestrings[0].text, "Top Node A");
  ASSERT_NE(tree->left, nullptr);
  EXPECT_EQ(tree->left->nodestrings[0].text, "Left Child A11");
  ASSERT_NE(tree->right, nullptr);
  EXPECT_EQ(tree->right->nodestrings[0].text, "Dropped it on the floor");
}

// ___________________________________________________________________________
// Font loading tests

TEST(FontLoad, LoadsHelveticaNarrow) {
  font f;
  EXPECT_TRUE(f.load("Helvetica-Narrow", FontsDir()));
}

TEST(FontLoad, NonexistentFont) {
  font f;
  EXPECT_FALSE(f.load("NoSuchFont", FontsDir()));
}

TEST(FontLoad, WidthsAreReasonable) {
  font f;
  ASSERT_TRUE(f.load("Helvetica-Narrow", FontsDir()));
  // Space should have a positive width
  EXPECT_GT(f.width(' '), 0.0);
  // 'W' is typically wider than 'i'
  EXPECT_GT(f.width('W'), f.width('i'));
  // Null character typically has zero width
  EXPECT_EQ(f.width(0), 0.0);
}

// ___________________________________________________________________________
// string_width tests

TEST(StringWidth, EmptyString) {
  font f;
  ASSERT_TRUE(f.load("Helvetica-Narrow", FontsDir()));
  EXPECT_DOUBLE_EQ(string_width("", f, 6.0), 0.0);
}

TEST(StringWidth, SingleChar) {
  font f;
  ASSERT_TRUE(f.load("Helvetica-Narrow", FontsDir()));
  double w = string_width("A", f, 6.0);
  EXPECT_GT(w, 0.0);
}

TEST(StringWidth, ScalesWithFontSize) {
  font f;
  ASSERT_TRUE(f.load("Helvetica-Narrow", FontsDir()));
  double w6 = string_width("Test", f, 6.0);
  double w12 = string_width("Test", f, 12.0);
  EXPECT_NEAR(w12, 2.0 * w6, 1e-10);
}

TEST(StringWidth, LongerStringIsWider) {
  font f;
  ASSERT_TRUE(f.load("Helvetica-Narrow", FontsDir()));
  double w_short = string_width("Hi", f, 6.0);
  double w_long = string_width("Hello World", f, 6.0);
  EXPECT_GT(w_long, w_short);
}

// ___________________________________________________________________________
// PostScript drawing primitive tests

TEST(PsDrawArc, ProducesExpectedOutput) {
  std::ostringstream os;
  os << std::fixed;
  ps_draw_arc(10.0, 20.0, 30.0, 40.0, os);
  std::string output = os.str();
  EXPECT_NE(output.find("np"), std::string::npos);
  EXPECT_NE(output.find("mt"), std::string::npos);
  EXPECT_NE(output.find("ct sk"), std::string::npos);
  // Check midpoint y: (20+40)/2 = 30
  EXPECT_NE(output.find("10.00 30.00"), std::string::npos);
}

TEST(PsDrawBox, ProducesExpectedOutput) {
  std::ostringstream os;
  os << std::fixed;
  ps_draw_box(0.0, 0.0, 10.0, 20.0, os);
  std::string output = os.str();
  EXPECT_NE(output.find("np"), std::string::npos);
  EXPECT_NE(output.find("cp er sk"), std::string::npos);
}

TEST(PsDrawString, SimpleString) {
  std::ostringstream os;
  os << std::fixed;
  ps_draw_string("Hello", 5.0, 10.0, os);
  std::string output = os.str();
  EXPECT_NE(output.find("(Hello)"), std::string::npos);
  EXPECT_NE(output.find("sh"), std::string::npos);
}

TEST(PsDrawString, EscapesParentheses) {
  std::ostringstream os;
  os << std::fixed;
  ps_draw_string("a(b)c", 0.0, 0.0, os);
  std::string output = os.str();
  EXPECT_NE(output.find("(a\\(b\\)c)"), std::string::npos);
}

TEST(PsDrawString, EscapesBackslash) {
  std::ostringstream os;
  os << std::fixed;
  ps_draw_string("a\\b", 0.0, 0.0, os);
  std::string output = os.str();
  EXPECT_NE(output.find("(a\\\\b)"), std::string::npos);
}

// ___________________________________________________________________________
// move_tree tests

TEST(MoveTree, HorizontalMove) {
  auto t = std::make_unique<pstree>();
  t->x = 10.0;
  t->xbox = 15.0;
  move_tree_horizontally(t.get(), 5.0);
  EXPECT_DOUBLE_EQ(t->x, 15.0);
  EXPECT_DOUBLE_EQ(t->xbox, 20.0);
}

TEST(MoveTree, VerticalMove) {
  auto t = std::make_unique<pstree>();
  t->y = 10.0;
  t->ybox = 15.0;
  move_tree_vertically(t.get(), 5.0);
  EXPECT_DOUBLE_EQ(t->y, 15.0);
  EXPECT_DOUBLE_EQ(t->ybox, 20.0);
}

TEST(MoveTree, HorizontalMoveWithChildren) {
  auto t = std::make_unique<pstree>();
  t->x = 0.0; t->xbox = 5.0;
  t->left = std::make_unique<pstree>();
  t->left->x = -5.0; t->left->xbox = -2.0;
  t->right = std::make_unique<pstree>();
  t->right->x = 5.0; t->right->xbox = 8.0;

  move_tree_horizontally(t.get(), 10.0);
  EXPECT_DOUBLE_EQ(t->x, 10.0);
  EXPECT_DOUBLE_EQ(t->xbox, 15.0);
  EXPECT_DOUBLE_EQ(t->left->x, 5.0);
  EXPECT_DOUBLE_EQ(t->left->xbox, 8.0);
  EXPECT_DOUBLE_EQ(t->right->x, 15.0);
  EXPECT_DOUBLE_EQ(t->right->xbox, 18.0);
}

TEST(MoveTree, NullTreeIsNoOp) {
  move_tree_horizontally(nullptr, 10.0);
  move_tree_vertically(nullptr, 10.0);
  // Should not crash
}

// ___________________________________________________________________________
// move_seglist tests

TEST(MoveSeglist, Horizontal) {
  std::vector<segment> segs = {{0, 0, 10, 10}, {5, 5, 15, 15}};
  move_seglist_horizontally(segs, 3.0);
  EXPECT_DOUBLE_EQ(segs[0].x1, 3.0);
  EXPECT_DOUBLE_EQ(segs[0].x2, 13.0);
  EXPECT_DOUBLE_EQ(segs[0].y1, 0.0);  // y unchanged
  EXPECT_DOUBLE_EQ(segs[1].x1, 8.0);
  EXPECT_DOUBLE_EQ(segs[1].x2, 18.0);
}

TEST(MoveSeglist, Vertical) {
  std::vector<segment> segs = {{0, 0, 10, 10}};
  move_seglist_vertically(segs, -2.0);
  EXPECT_DOUBLE_EQ(segs[0].y1, -2.0);
  EXPECT_DOUBLE_EQ(segs[0].y2, 8.0);
  EXPECT_DOUBLE_EQ(segs[0].x1, 0.0);  // x unchanged
}

// ___________________________________________________________________________
// Golden file integration tests: full pipeline for each sample

class GoldenTest : public ::testing::Test {
 protected:
  // Run the full PST pipeline on an input file and return the PostScript output
  // (just the tree-drawing portion that depends on layout computation).
  std::string RunPipeline(const std::string& sample_name) {
    font mainfont;
    std::string fonts_dir = FontsDir();
    EXPECT_TRUE(mainfont.load("Helvetica-Narrow", fonts_dir));

    std::string fontname = "Helvetica-Narrow";
    double fontsize = 6.0;

    std::string input_path = TestDataPath("testdata/" + sample_name);
    std::ifstream ifp(input_path);
    EXPECT_TRUE(ifp.good()) << "Cannot open " << input_path;

    auto tree = ps_restore_tree(ifp);
    ifp.close();
    EXPECT_NE(tree, nullptr);

    std::ostringstream ofp;
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

    set_sizes(tree.get(), mainfont, fontsize, 1.5 * fontsize);

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
    } else {
      wpages = w2;
      pwidth = 720;
      pheight = 540;
      hpages = h2;
      orientation = 2;
    }

    ofp << std::setprecision(2);
    ofp << "% width: " << tree->width << " height: " << tree->height << "\n";
    ofp << "% wpages: " << wpages << ", hpages: " << hpages
        << ", total: " << hpages * wpages << "\n";
    ofp << "/sclip {np 0 0 mt 0 " << pheight << " rlt " << pwidth
        << " 0 rlt 0 " << pheight << " neg rlt cp clip} def\n";

    for (int rowcount = 0; rowcount < hpages; rowcount++)
      for (int colcount = 0; colcount < wpages; colcount++) {
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

    return ofp.str();
  }

  std::string ReadGoldenFile(const std::string& sample_name) {
    std::string path = TestDataPath("testdata/" + sample_name + ".ps");
    std::ifstream ifs(path);
    EXPECT_TRUE(ifs.good()) << "Cannot open golden file " << path;
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
  }
};

TEST_F(GoldenTest, Sample1) {
  std::string actual = RunPipeline("sample1.txt");
  std::string expected = ReadGoldenFile("sample1.txt");
  EXPECT_EQ(actual, expected);
}

TEST_F(GoldenTest, Sample2) {
  std::string actual = RunPipeline("sample2.txt");
  std::string expected = ReadGoldenFile("sample2.txt");
  EXPECT_EQ(actual, expected);
}

TEST_F(GoldenTest, Sample3) {
  std::string actual = RunPipeline("sample3.txt");
  std::string expected = ReadGoldenFile("sample3.txt");
  EXPECT_EQ(actual, expected);
}

// ___________________________________________________________________________
// pst_test.cc
