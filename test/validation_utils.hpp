#ifndef VALIDATION_UTILS_HPP
#define VALIDATION_UTILS_HPP

#include "../lib/multitree_iterator.hpp"
#include "../lib/validation.hpp"

namespace terraces {
namespace tests {

inline void check_unique_trees(multitree_node* root, index num_trees) {
	multitree_iterator it(root);

	std::vector<std::vector<simple_bitvector>> bipartitions;
	do {
		bipartitions.push_back(tree_bipartitions(it.tree()));
	} while (it.next());
	// check that all trees are unique
	std::sort(bipartitions.begin(), bipartitions.end());
	CHECK(std::adjacent_find(bipartitions.begin(), bipartitions.end()) == bipartitions.end());
	// check that there is the right number of trees
	CHECK(bipartitions.size() == num_trees);
}

} // namespace tests
} // namespace terraces

#endif // VALIDATION_UTILS_HPP
