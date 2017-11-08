
#include <catch.hpp>
#include <iostream>

#include <terraces/rooting.hpp>
#include <terraces/trees.hpp>

namespace terraces {
namespace tests {

bool parent_child_relationship(tree& t, index parent, index child) {
	bool left_child = t[parent].lchild() == child;
	if (left_child) {
		return t[t[parent].lchild()].parent() == parent;
	}
	bool right_child = t[parent].rchild() == child;
	return right_child && t[t[parent].rchild()].parent() == parent;
}

TEST_CASE("rerooting basic", "[rerooting]") {
	auto t = tree{{none, 1, 2, none},
	              {0, none, none, 0},
	              {0, 3, 4, none},
	              {2, none, none, 1},
	              {2, none, none, 2}};
	auto t_2 = tree(t);
	auto t_3 = tree(t);

	const auto root_leaf = 3;
	reroot_inplace(t, t[root_leaf].taxon());
	CHECK(((t[0].lchild() == root_leaf) or (t[0].rchild() == root_leaf)));
	CHECK(parent_child_relationship(t, 0, root_leaf));
	CHECK(parent_child_relationship(t, 0, 2));
	CHECK(parent_child_relationship(t, 2, 4));
	CHECK(parent_child_relationship(t, 2, 1));

	const auto root_leaf_2 = 4;
	reroot_inplace(t_2, t_2[root_leaf_2].taxon());
	CHECK(((t_2[0].lchild() == root_leaf_2) or (t_2[0].rchild() == root_leaf_2)));

	const auto root_leaf_3 = 1;
	reroot_inplace(t_3, t_3[root_leaf_3].taxon());
	CHECK(((t_3[0].lchild() == root_leaf_3) or (t_3[0].rchild() == root_leaf_3)));
}

TEST_CASE("rerooting advanced at 1", "[rerooting]") {
	auto t = tree{{none, 1, 2, none}, {0, none, none, 0},    {0, 3, 4, none},
	              {2, 5, 6, none},    {2, none, none, none}, {3, none, none, none},
	              {3, 7, 8, none},    {6, none, none, none}, {6, none, none, none}};

	const auto root_leaf = 1;
	reroot_inplace(t, t[root_leaf].taxon());
	CHECK(((t[0].lchild() == root_leaf) or (t[0].rchild() == root_leaf)));
}

TEST_CASE("rerooting advanced at 4", "[rerooting]") {
	auto t = tree{{none, 1, 2, none}, {0, none, none, none}, {0, 3, 4, none},
	              {2, 5, 6, none},    {2, none, none, 0},    {3, none, none, none},
	              {3, 7, 8, none},    {6, none, none, none}, {6, none, none, none}};

	const auto root_leaf = 4;
	reroot_inplace(t, t[root_leaf].taxon());
	CHECK(((t[0].lchild() == root_leaf) or (t[0].rchild() == root_leaf)));
}

TEST_CASE("rerooting advanced at 5", "[rerooting]") {
	auto t = tree{{none, 1, 2, none}, {0, none, none, none}, {0, 3, 4, none},
	              {2, 5, 6, none},    {2, none, none, none}, {3, none, none, 0},
	              {3, 7, 8, none},    {6, none, none, none}, {6, none, none, none}};

	const auto root_leaf = 5;
	reroot_inplace(t, t[root_leaf].taxon());
	CHECK(((t[0].lchild() == root_leaf) or (t[0].rchild() == root_leaf)));
}

TEST_CASE("rerooting advanced at 7", "[rerooting]") {
	auto t = tree{{none, 1, 2, none}, {0, none, none, none}, {0, 3, 4, none},
	              {2, 5, 6, none},    {2, none, none, none}, {3, none, none, none},
	              {3, 7, 8, none},    {6, none, none, 0},    {6, none, none, none}};

	const auto root_leaf = 7;
	reroot_inplace(t, t[root_leaf].taxon());
	CHECK(((t[0].lchild() == root_leaf) or (t[0].rchild() == root_leaf)));
}

TEST_CASE("rerooting advanced at 8", "[rerooting]") {
	auto t = tree{{none, 1, 2, none}, {0, none, none, none}, {0, 3, 4, none},
	              {2, 5, 6, none},    {2, none, none, none}, {3, none, none, none},
	              {3, 7, 8, none},    {6, none, none, none}, {6, none, none, 0}};

	const auto root_leaf = 8;
	reroot_inplace(t, t[root_leaf].taxon());
	CHECK(((t[0].lchild() == root_leaf) or (t[0].rchild() == root_leaf)));
}
} // namespace tests
} // namespace terraces
