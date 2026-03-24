#include <gtest/gtest.h>
#include "multi_viewer.h"

class MultiViewerTest : public ::testing::Test {
protected:
  MultiViewer mv;
};

// --- Add / Count tests ---

TEST_F(MultiViewerTest, InitiallyEmpty) {
  EXPECT_EQ(mv.Count(), 0);
  EXPECT_EQ(mv.ActiveIndex(), -1);
}

TEST_F(MultiViewerTest, AddSingleTorrent) {
  int idx = mv.Add("/path/to/file1.torrent");
  EXPECT_EQ(idx, 0);
  EXPECT_EQ(mv.Count(), 1);
  EXPECT_EQ(mv.ActiveIndex(), 0);
}

TEST_F(MultiViewerTest, AddMultipleTorrents) {
  mv.Add("/path/to/file1.torrent");
  mv.Add("/path/to/file2.torrent");
  mv.Add("/path/to/file3.torrent");
  EXPECT_EQ(mv.Count(), 3);
  // First added becomes active
  EXPECT_EQ(mv.ActiveIndex(), 0);
}

TEST_F(MultiViewerTest, PathAtReturnsCorrectPath) {
  mv.Add("/path/to/alpha.torrent");
  mv.Add("/path/to/beta.torrent");
  EXPECT_EQ(mv.PathAt(0), "/path/to/alpha.torrent");
  EXPECT_EQ(mv.PathAt(1), "/path/to/beta.torrent");
}

TEST_F(MultiViewerTest, PathsReturnsAll) {
  mv.Add("/a.torrent");
  mv.Add("/b.torrent");
  EXPECT_EQ(mv.Paths().size(), 2u);
}

// --- SetActive tests ---

TEST_F(MultiViewerTest, SetActiveValid) {
  mv.Add("/a.torrent");
  mv.Add("/b.torrent");
  EXPECT_TRUE(mv.SetActive(1));
  EXPECT_EQ(mv.ActiveIndex(), 1);
}

TEST_F(MultiViewerTest, SetActiveInvalidReturnsFalse) {
  mv.Add("/a.torrent");
  EXPECT_FALSE(mv.SetActive(-1));
  EXPECT_FALSE(mv.SetActive(5));
}

// --- Tab navigation tests ---

TEST_F(MultiViewerTest, NextTabWrapsAround) {
  mv.Add("/a.torrent");
  mv.Add("/b.torrent");
  mv.Add("/c.torrent");
  EXPECT_EQ(mv.ActiveIndex(), 0);
  EXPECT_EQ(mv.NextTab(), 1);
  EXPECT_EQ(mv.NextTab(), 2);
  EXPECT_EQ(mv.NextTab(), 0); // wraps
}

TEST_F(MultiViewerTest, PrevTabWrapsAround) {
  mv.Add("/a.torrent");
  mv.Add("/b.torrent");
  mv.Add("/c.torrent");
  EXPECT_EQ(mv.ActiveIndex(), 0);
  EXPECT_EQ(mv.PrevTab(), 2); // wraps to end
  EXPECT_EQ(mv.PrevTab(), 1);
  EXPECT_EQ(mv.PrevTab(), 0);
}

TEST_F(MultiViewerTest, NextTabEmptyReturnsNegative) {
  EXPECT_EQ(mv.NextTab(), -1);
}

TEST_F(MultiViewerTest, PrevTabEmptyReturnsNegative) {
  EXPECT_EQ(mv.PrevTab(), -1);
}

TEST_F(MultiViewerTest, NextTabSingleElement) {
  mv.Add("/a.torrent");
  EXPECT_EQ(mv.NextTab(), 0);
  EXPECT_EQ(mv.NextTab(), 0);
}

TEST_F(MultiViewerTest, PrevTabSingleElement) {
  mv.Add("/a.torrent");
  EXPECT_EQ(mv.PrevTab(), 0);
  EXPECT_EQ(mv.PrevTab(), 0);
}

// --- Remove tests ---

TEST_F(MultiViewerTest, RemoveValid) {
  mv.Add("/a.torrent");
  mv.Add("/b.torrent");
  EXPECT_TRUE(mv.Remove(0));
  EXPECT_EQ(mv.Count(), 1);
  EXPECT_EQ(mv.PathAt(0), "/b.torrent");
}

TEST_F(MultiViewerTest, RemoveInvalidReturnsFalse) {
  mv.Add("/a.torrent");
  EXPECT_FALSE(mv.Remove(-1));
  EXPECT_FALSE(mv.Remove(5));
}

TEST_F(MultiViewerTest, RemoveLastSetsActiveToNegative) {
  mv.Add("/a.torrent");
  mv.Remove(0);
  EXPECT_EQ(mv.ActiveIndex(), -1);
  EXPECT_EQ(mv.Count(), 0);
}

TEST_F(MultiViewerTest, RemoveAdjustsActiveIndex) {
  mv.Add("/a.torrent");
  mv.Add("/b.torrent");
  mv.Add("/c.torrent");
  mv.SetActive(2);
  mv.Remove(2);
  // Active should clamp to last valid index
  EXPECT_EQ(mv.ActiveIndex(), 1);
}

// --- Tab label tests ---

TEST_F(MultiViewerTest, TabLabelExtractsFilename) {
  EXPECT_EQ(MultiViewer::TabLabel("/path/to/my_file.torrent"),
            "my_file.torrent");
}

TEST_F(MultiViewerTest, TabLabelHandlesSimpleName) {
  EXPECT_EQ(MultiViewer::TabLabel("simple.torrent"), "simple.torrent");
}

TEST_F(MultiViewerTest, PathAtThrowsOnInvalidIndex) {
  mv.Add("/a.torrent");
  EXPECT_THROW(mv.PathAt(5), std::out_of_range);
}
