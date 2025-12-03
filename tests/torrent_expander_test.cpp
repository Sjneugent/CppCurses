#include <gtest/gtest.h>
#include "torrent_expander.h"

class TorrentExpanderTest : public ::testing::Test {
protected:
    void SetUp() override {
        root = TorrentExpanderImpl::Root();
    }

    TorrentExpander root;
};

// Test initial state
TEST_F(TorrentExpanderTest, InitialState) {
    EXPECT_FALSE(root->expanded);
}

// Test SetExpanded
TEST_F(TorrentExpanderTest, SetExpanded) {
    root->SetExpanded(true);
    EXPECT_TRUE(root->expanded);
    
    root->SetExpanded(false);
    EXPECT_FALSE(root->expanded);
}

// Test Toggle
TEST_F(TorrentExpanderTest, Toggle) {
    EXPECT_FALSE(root->expanded);
    
    bool result = root->Toggle();
    EXPECT_TRUE(result);
    EXPECT_TRUE(root->expanded);
    
    result = root->Toggle();
    EXPECT_FALSE(result);
    EXPECT_FALSE(root->expanded);
}

// Test child creation
TEST_F(TorrentExpanderTest, CreateChild) {
    auto child = root->Child();
    ASSERT_NE(child, nullptr);
    EXPECT_FALSE(child->expanded);
}

// Test multiple children
TEST_F(TorrentExpanderTest, MultipleChildren) {
    auto child1 = root->Child();
    auto child2 = root->Child();
    auto child3 = root->Child();
    
    ASSERT_NE(child1, nullptr);
    ASSERT_NE(child2, nullptr);
    ASSERT_NE(child3, nullptr);
    
    // Each child should be independent
    child1->SetExpanded(true);
    EXPECT_TRUE(child1->expanded);
    EXPECT_FALSE(child2->expanded);
    EXPECT_FALSE(child3->expanded);
}

// Test MinLevel when not expanded
TEST_F(TorrentExpanderTest, MinLevelNotExpanded) {
    EXPECT_EQ(root->MinLevel(), 0);
}

// Test MinLevel when expanded with no children
TEST_F(TorrentExpanderTest, MinLevelExpandedNoChildren) {
    root->SetExpanded(true);
    EXPECT_EQ(root->MinLevel(), 1);
}

// Test MinLevel with expanded children
TEST_F(TorrentExpanderTest, MinLevelWithChildren) {
    root->SetExpanded(true);
    auto child1 = root->Child();
    auto child2 = root->Child();
    
    // Root expanded, children not expanded
    EXPECT_EQ(root->MinLevel(), 1);
    
    // Expand one child
    child1->SetExpanded(true);
    EXPECT_EQ(root->MinLevel(), 1);
    
    // Expand both children
    child2->SetExpanded(true);
    EXPECT_EQ(root->MinLevel(), 2);
}

// Test MaxLevel when not expanded
TEST_F(TorrentExpanderTest, MaxLevelNotExpanded) {
    EXPECT_EQ(root->MaxLevel(), 0);
}

// Test MaxLevel when expanded with no children
TEST_F(TorrentExpanderTest, MaxLevelExpandedNoChildren) {
    root->SetExpanded(true);
    EXPECT_EQ(root->MaxLevel(), 1);
}

// Test MaxLevel with mixed child states
TEST_F(TorrentExpanderTest, MaxLevelWithMixedChildren) {
    root->SetExpanded(true);
    auto child1 = root->Child();
    auto child2 = root->Child();
    
    // Root expanded, children not expanded
    EXPECT_EQ(root->MaxLevel(), 1);
    
    // Expand one child
    child1->SetExpanded(true);
    EXPECT_EQ(root->MaxLevel(), 2);
    
    // child2 still not expanded, so max should still be 2
    EXPECT_EQ(root->MaxLevel(), 2);
}

// Test MaxLevel with nested expansion
TEST_F(TorrentExpanderTest, MaxLevelNested) {
    root->SetExpanded(true);
    auto child = root->Child();
    child->SetExpanded(true);
    auto grandchild = child->Child();
    grandchild->SetExpanded(true);
    
    EXPECT_EQ(root->MaxLevel(), 3);
}

// Test Expand method
TEST_F(TorrentExpanderTest, ExpandMethod) {
    auto child = root->Child();
    
    // Initially collapsed
    EXPECT_FALSE(root->expanded);
    EXPECT_FALSE(child->expanded);
    
    // Expand once - should expand root
    bool changed = root->Expand();
    EXPECT_TRUE(changed);
    EXPECT_TRUE(root->expanded);
    
    // Expand again - should expand children
    changed = root->Expand();
    EXPECT_TRUE(changed);
    EXPECT_TRUE(child->expanded);
    
    // Expand again - no change expected
    changed = root->Expand();
    EXPECT_FALSE(changed);
}

// Test Collapse method
TEST_F(TorrentExpanderTest, CollapseMethod) {
    auto child = root->Child();
    auto grandchild = child->Child();
    
    // Expand all
    root->SetExpanded(true);
    child->SetExpanded(true);
    grandchild->SetExpanded(true);
    
    EXPECT_EQ(root->MaxLevel(), 3);
    
    // Collapse once - should collapse grandchild
    bool changed = root->Collapse();
    EXPECT_TRUE(changed);
    EXPECT_FALSE(grandchild->expanded);
    EXPECT_TRUE(child->expanded);
    EXPECT_TRUE(root->expanded);
    
    // Collapse again - should collapse child
    changed = root->Collapse();
    EXPECT_TRUE(changed);
    EXPECT_FALSE(child->expanded);
    EXPECT_TRUE(root->expanded);
    
    // Collapse again - should collapse root
    changed = root->Collapse();
    EXPECT_TRUE(changed);
    EXPECT_FALSE(root->expanded);
    
    // Collapse again - no change expected
    changed = root->Collapse();
    EXPECT_FALSE(changed);
}

// Test complex tree structure
TEST_F(TorrentExpanderTest, ComplexTreeStructure) {
    // Build a tree: root -> child1, child2 -> grandchild1, grandchild2
    auto child1 = root->Child();
    auto child2 = root->Child();
    auto grandchild1 = child1->Child();
    auto grandchild2 = child2->Child();
    
    // Expand root and child1
    root->SetExpanded(true);
    child1->SetExpanded(true);
    
    // Check levels
    EXPECT_EQ(root->MinLevel(), 1);  // child2 is not expanded
    EXPECT_EQ(root->MaxLevel(), 2);  // child1 is expanded
    
    // Expand child2
    child2->SetExpanded(true);
    EXPECT_EQ(root->MinLevel(), 2);  // Now both children expanded
    EXPECT_EQ(root->MaxLevel(), 2);
    
    // Expand grandchild1
    grandchild1->SetExpanded(true);
    EXPECT_EQ(root->MaxLevel(), 3);
}

// Test child destruction and parent cleanup
TEST_F(TorrentExpanderTest, ChildDestruction) {
    {
        auto child = root->Child();
        child->SetExpanded(true);
        // child goes out of scope here
    }
    
    // Root should still be valid
    root->SetExpanded(true);
    EXPECT_TRUE(root->expanded);
    
    // MinLevel should be 1 (no children)
    EXPECT_EQ(root->MinLevel(), 1);
}

// Test independent expansion states
TEST_F(TorrentExpanderTest, IndependentExpansionStates) {
    auto child1 = root->Child();
    auto child2 = root->Child();
    auto grandchild1 = child1->Child();
    auto grandchild2 = child2->Child();
    
    // Expand only specific nodes
    root->SetExpanded(true);
    child1->SetExpanded(true);
    grandchild2->SetExpanded(true);  // grandchild2's parent (child2) is not expanded
    
    EXPECT_TRUE(root->expanded);
    EXPECT_TRUE(child1->expanded);
    EXPECT_FALSE(child2->expanded);
    EXPECT_FALSE(grandchild1->expanded);
    EXPECT_TRUE(grandchild2->expanded);  // Can be expanded even if parent isn't
}

// Test MinLevel with mixed tree
TEST_F(TorrentExpanderTest, MinLevelMixedTree) {
    auto child1 = root->Child();
    auto child2 = root->Child();
    auto grandchild1 = child1->Child();
    
    root->SetExpanded(true);
    child1->SetExpanded(true);
    grandchild1->SetExpanded(true);
    // child2 is not expanded
    
    // MinLevel should be 1 because child2 is not expanded
    EXPECT_EQ(root->MinLevel(), 1);
}

// Test MaxLevel with single deep branch
TEST_F(TorrentExpanderTest, MaxLevelDeepBranch) {
    root->SetExpanded(true);
    auto level1 = root->Child();
    level1->SetExpanded(true);
    auto level2 = level1->Child();
    level2->SetExpanded(true);
    auto level3 = level2->Child();
    level3->SetExpanded(true);
    auto level4 = level3->Child();
    level4->SetExpanded(true);
    
    EXPECT_EQ(root->MaxLevel(), 5);
}

// Test that Expand respects levels
TEST_F(TorrentExpanderTest, ExpandRespectsLevels) {
    auto child = root->Child();
    auto grandchild = child->Child();
    auto greatgrandchild = grandchild->Child();
    
    // Start with everything collapsed
    EXPECT_EQ(root->MinLevel(), 0);
    
    // First expand expands to level 1
    root->Expand();
    EXPECT_TRUE(root->expanded);
    EXPECT_FALSE(child->expanded);
    EXPECT_FALSE(grandchild->expanded);
    
    // Second expand expands to level 2
    root->Expand();
    EXPECT_TRUE(root->expanded);
    EXPECT_TRUE(child->expanded);
    EXPECT_FALSE(grandchild->expanded);
    
    // Third expand expands to level 3
    root->Expand();
    EXPECT_TRUE(root->expanded);
    EXPECT_TRUE(child->expanded);
    EXPECT_TRUE(grandchild->expanded);
    EXPECT_FALSE(greatgrandchild->expanded);
}
