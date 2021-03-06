#include <terraces/advanced.hpp>

#include <terraces/clamped_uint.hpp>
#include <terraces/errors.hpp>
#include <terraces/rooting.hpp>
#include <terraces/subtree_extraction.hpp>

#include "multitree_iterator.hpp"
#include "supertree_enumerator.hpp"
#include "supertree_variants.hpp"
#include "supertree_variants_multitree.hpp"

namespace terraces {

supertree_data create_supertree_data(const tree& tree, const bitmatrix& data) {
	auto root = find_comprehensive_taxon(data);
	utils::ensure<bad_input_error>(data.rows() == num_leaves_from_nodes(tree.size()),
	                               bad_input_error_type::tree_mismatching_size);
	utils::ensure<no_usable_root_error>(root != none, "No comprehensive taxon found");
	auto rerooted_tree = tree;
	reroot_at_taxon_inplace(rerooted_tree, root);
	auto trees = subtrees(rerooted_tree, data);
	auto constraints = compute_constraints(trees);
	deduplicate_constraints(constraints);

	auto num_leaves = data.rows();
	utils::ensure<bad_input_error>(num_leaves >= 4, bad_input_error_type::nwk_tree_trivial);
	return {constraints, num_leaves, root};
}

index_t find_comprehensive_taxon(const bitmatrix& data) {
	for (index_t i = 0; i < data.rows(); ++i) {
		bool comp = true;
		for (index_t j = 0; j < data.cols(); ++j) {
			comp &= data.get(i, j);
		}
		if (comp) {
			return i;
		}
	}
	return none;
}

bitmatrix maximum_comprehensive_columnset(const bitmatrix& data) {
	std::vector<index_t> row_counts(data.rows(), 0u);
	for (index_t i = 0; i < data.rows(); ++i) {
		for (index_t j = 0; j < data.cols(); ++j) {
			row_counts[i] += data.get(i, j) ? 1u : 0u;
		}
	}
	auto it = std::max_element(row_counts.begin(), row_counts.end());
	index_t comp_row = static_cast<index_t>(std::distance(row_counts.begin(), it));
	std::vector<index_t> columns;
	for (index_t j = 0; j < data.cols(); ++j) {
		if (data.get(comp_row, j)) {
			columns.push_back(j);
		}
	}
	return data.get_cols(columns);
}

index_t fast_count_terrace(const supertree_data& data) {
	tree_enumerator<variants::check_callback> enumerator{{}};
	try {
		return enumerator.run(data.num_leaves, data.constraints, data.root);
	} catch (terraces::tree_count_overflow_error&) {
		return std::numeric_limits<index_t>::max();
	}
}

bool check_terrace(const supertree_data& data) { return fast_count_terrace(data) > 1; }

index_t count_terrace(const supertree_data& data, execution_limits limits, bool& terminated_early) {
	tree_enumerator<variants::timeout_decorator<variants::clamped_count_callback>> enumerator{
	        {limits.time_limit_seconds}};
	try {
		auto result = enumerator.run(data.num_leaves, data.constraints, data.root).value();
		terminated_early = enumerator.callback().has_timed_out();
		return result;
	} catch (terraces::tree_count_overflow_error&) {
		return std::numeric_limits<index_t>::max();
	}
}

big_integer count_terrace_bigint(const supertree_data& data, execution_limits limits,
                                 bool& terminated_early) {
	tree_enumerator<variants::timeout_decorator<variants::count_callback<big_integer>>>
	        enumerator{{limits.time_limit_seconds}};
	auto result = enumerator.run(data.num_leaves, data.constraints, data.root);
	terminated_early = enumerator.callback().has_timed_out();
	return result;
}

using limited_multitree_callback =
        variants::timeout_decorator<variants::memory_limited_multitree_callback>;

big_integer print_terrace_compressed(const supertree_data& data, const name_map& names,
                                     std::ostream& output, execution_limits limits,
                                     bool& terminated_early) {
	tree_enumerator<limited_multitree_callback> enumerator{
	        limited_multitree_callback{limits.time_limit_seconds, limits.mem_limit_bytes}};
	auto result = enumerator.run(data.num_leaves, data.constraints, data.root);
	terminated_early = enumerator.callback().has_timed_out() ||
	                   enumerator.callback().has_hit_memory_limit();
	output << as_newick(result, names);

	return result->num_trees;
}

big_integer print_terrace(const supertree_data& data, const name_map& names, std::ostream& output,
                          execution_limits limits, bool& terminated_early) {
	tree_enumerator<limited_multitree_callback> enumerator{
	        limited_multitree_callback{limits.time_limit_seconds, limits.mem_limit_bytes}};
	auto result = enumerator.run(data.num_leaves, data.constraints, data.root);
	terminated_early = enumerator.callback().has_timed_out() ||
	                   enumerator.callback().has_hit_memory_limit();
	if (!terminated_early) {
		multitree_iterator mit{result};
		do {
			output << as_newick(mit.tree(), names) << '\n';
		} while (mit.next());
	}
	return result->num_trees;
}

void enumerate_terrace(const supertree_data& data, std::function<void(const tree&)> callback,
                       execution_limits limits, bool& terminated_early) {
	tree_enumerator<limited_multitree_callback> enumerator{
	        limited_multitree_callback{limits.time_limit_seconds, limits.mem_limit_bytes}};
	auto result = enumerator.run(data.num_leaves, data.constraints, data.root);
	terminated_early = enumerator.callback().has_timed_out() ||
	                   enumerator.callback().has_hit_memory_limit();
	if (!terminated_early) {
		multitree_iterator mit{result};
		do {
			callback(mit.tree());
		} while (mit.next());
	}
}

index_t count_terrace(const supertree_data& data) {
	execution_limits limits{};
	bool tmp;
	return count_terrace(data, limits, tmp);
}

big_integer count_terrace_bigint(const supertree_data& data) {
	execution_limits limits{};
	bool tmp;
	return count_terrace_bigint(data, limits, tmp);
}

big_integer print_terrace_compressed(const supertree_data& data, const name_map& names,
                                     std::ostream& output) {
	execution_limits limits{};
	bool tmp;
	return print_terrace_compressed(data, names, output, limits, tmp);
}

big_integer print_terrace(const supertree_data& data, const name_map& names, std::ostream& output) {
	execution_limits limits{};
	bool tmp;
	return print_terrace(data, names, output, limits, tmp);
}

void enumerate_terrace(const supertree_data& data, std::function<void(const tree&)> callback) {
	execution_limits limits{};
	bool tmp;
	return enumerate_terrace(data, std::move(callback), limits, tmp);
}

} // namespace terraces
