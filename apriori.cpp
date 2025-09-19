#include "include/apriori.h"
#include <iostream>
#include <set>
#include <map>
#include <algorithm>
#include <vector>
#include <numeric>


// Helper function to generate combinations
std::vector<std::vector<std::string>> generate_combinations(
    const std::vector<std::string>& items, int k) {
    std::vector<std::vector<std::string>> combinations;
    std::vector<int> p(k);
    std::vector<int> v(items.size());
    std::iota(v.begin(), v.end(), 0);

    std::function<void(int, int)> f = 
        [&](int offset, int k_idx) {
        if (k_idx == k) {
            std::vector<std::string> combination;
            for (int i = 0; i < k; ++i) {
                combination.push_back(items[p[i]]);
            }
            combinations.push_back(combination);
            return;
        }
        for (int i = offset; i <= (int)items.size() - (k - k_idx); ++i) {
            p[k_idx] = v[i];
            f(i + 1, k_idx + 1);
        }
    };
    f(0, 0);
    return combinations;
}

// Helper function to check if a candidate contains all subsets of size k-1
bool has_frequent_subset(
    const std::vector<std::string>& candidate,
    const std::set<std::vector<std::string>>& frequent_k_minus_1_itemsets) {
    if (candidate.size() <= 1) return true; // Base case for single items

    for (size_t i = 0; i < candidate.size(); ++i) {
        std::vector<std::string> subset;
        for (size_t j = 0; j < candidate.size(); ++j) {
            if (i == j) continue;
            subset.push_back(candidate[j]);
        }
        std::sort(subset.begin(), subset.end());
        if (frequent_k_minus_1_itemsets.find(subset) == frequent_k_minus_1_itemsets.end()) {
            return false;
        }
    }
    return true;
}

std::vector<AssociationRule> run_apriori(
    const std::vector<std::vector<std::string>>& transactions,
    double min_support,
    double min_confidence) {

    std::vector<AssociationRule> rules;
    if (transactions.empty()) return rules;

    // Step 1: Generate frequent 1-itemsets
    std::map<std::string, int> item_counts;
    for (const auto& t : transactions) {
        for (const auto& item : t) {
            item_counts[item]++;
        }
    }

    std::map<std::vector<std::string>, int> frequent_itemsets_counts;
    std::set<std::vector<std::string>> frequent_itemsets_k_minus_1;

    for (const auto& pair : item_counts) {
        if ((double)pair.second / transactions.size() >= min_support) {
            frequent_itemsets_counts[{pair.first}] = pair.second;
            frequent_itemsets_k_minus_1.insert({pair.first});
        }
    }

    int k = 2;
    while (!frequent_itemsets_k_minus_1.empty()) {
        // Step 2: Generate candidate k-itemsets (Fk-1 x Fk-1 join)
        std::set<std::vector<std::string>> candidate_k_itemsets;
        std::vector<std::vector<std::string>> prev_frequent_list(
            frequent_itemsets_k_minus_1.begin(), frequent_itemsets_k_minus_1.end());

        for (size_t i = 0; i < prev_frequent_list.size(); ++i) {
            for (size_t j = i + 1; j < prev_frequent_list.size(); ++j) {
                std::vector<std::string> itemset1 = prev_frequent_list[i];
                std::vector<std::string> itemset2 = prev_frequent_list[j];

                // Sort and compare all but the last item
                bool can_join = true;
                if (k > 2) {
                    for (int l = 0; l < k - 2; ++l) {
                        if (itemset1[l] != itemset2[l]) {
                            can_join = false;
                            break;
                        }
                    }
                    if (itemset1[k-2] >= itemset2[k-2]) can_join = false; // Ensure distinctness and order
                }

                if (can_join) {
                    std::set<std::string> joined_set;
                    for (const auto& item : itemset1) joined_set.insert(item);
                    for (const auto& item : itemset2) joined_set.insert(item);

                    if (joined_set.size() == k) {
                        std::vector<std::string> candidate(joined_set.begin(), joined_set.end());
                        std::sort(candidate.begin(), candidate.end());
                        // Pruning step: check if all (k-1)-subsets are frequent
                        if (has_frequent_subset(candidate, frequent_itemsets_k_minus_1)) {
                            candidate_k_itemsets.insert(candidate);
                        }
                    }
                }
            }
        }

        if (candidate_k_itemsets.empty()) break;

        // Step 3: Count support for candidate k-itemsets
        std::map<std::vector<std::string>, int> current_frequent_itemsets_counts;
        for (const auto& t : transactions) {
            for (const auto& candidate : candidate_k_itemsets) {
                bool contains = true;
                for (const auto& item : candidate) {
                    if (std::find(t.begin(), t.end(), item) == t.end()) {
                        contains = false;
                        break;
                    }
                }
                if (contains) {
                    current_frequent_itemsets_counts[candidate]++;
                }
            }
        }

        // Step 4: Filter frequent k-itemsets
        frequent_itemsets_k_minus_1.clear();
        for (const auto& pair : current_frequent_itemsets_counts) {
            if ((double)pair.second / transactions.size() >= min_support) {
                frequent_itemsets_counts[pair.first] = pair.second;
                frequent_itemsets_k_minus_1.insert(pair.first);
            }
        }
        k++;
    }

    // Step 5: Generate association rules from frequent itemsets
    for (const auto& itemset_pair : frequent_itemsets_counts) {
        const std::vector<std::string>& itemset = itemset_pair.first;
        if (itemset.size() < 2) continue;

        int itemset_support_count = itemset_pair.second;

        // Generate all possible non-empty proper subsets of itemset
        for (int i = 1; i < (1 << itemset.size()) - 1; ++i) {
            std::vector<std::string> antecedent;
            std::vector<std::string> consequent;

            for (size_t j = 0; j < itemset.size(); ++j) {
                if ((i >> j) & 1) {
                    antecedent.push_back(itemset[j]);
                } else {
                    consequent.push_back(itemset[j]);
                }
            }
            std::sort(antecedent.begin(), antecedent.end());
            std::sort(consequent.begin(), consequent.end());

            if (antecedent.empty() || consequent.empty()) continue;

            // Find support count for antecedent
            int antecedent_support_count = 0;
            auto it = frequent_itemsets_counts.find(antecedent);
            if (it != frequent_itemsets_counts.end()) {
                antecedent_support_count = it->second;
            } else {
                // This case should ideally not happen if all frequent subsets are stored
                // For safety, re-calculate if not found
                for (const auto& t : transactions) {
                    bool contains = true;
                    for (const auto& item : antecedent) {
                        if (std::find(t.begin(), t.end(), item) == t.end()) {
                            contains = false;
                            break;
                        }
                    }
                    if (contains) antecedent_support_count++;
                }
            }

            if (antecedent_support_count > 0) {
                double confidence = (double)itemset_support_count / antecedent_support_count;
                if (confidence >= min_confidence) {
                    AssociationRule rule;
                    rule.antecedent = antecedent;
                    rule.consequent = consequent;
                    rule.support = (double)itemset_support_count / transactions.size();
                    rule.confidence = confidence;
                    // Calculate lift
                    int consequent_support_count = 0;
                    auto it_consequent = frequent_itemsets_counts.find(consequent);
                    if (it_consequent != frequent_itemsets_counts.end()) {
                        consequent_support_count = it_consequent->second;
                    } else {
                        for (const auto& t : transactions) {
                            bool contains = true;
                            for (const auto& item : consequent) {
                                if (std::find(t.begin(), t.end(), item) == t.end()) {
                                    contains = false;
                                    break;
                                }
                            }
                            if (contains) consequent_support_count++;
                        }
                    }
                    double consequent_support = (double)consequent_support_count / transactions.size();
                    rule.lift = (consequent_support > 1e-9) ? confidence / consequent_support : 0.0;
                    rules.push_back(rule);
                }
            }
        }
    }

    return rules;
}





