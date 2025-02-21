#pragma once

#include <bitset> // std::bitset
#include <cstdint> // uint64_t
#include <memory> // std::unique_ptr
#include <vector> // std::vector

#include <graphtyper/constants.hpp> // MAX_NUMBER_OF_HAPLOTYPES
#include <graphtyper/graph/genotype.hpp> // gyper::Genotype
#include <graphtyper/typer/var_stats.hpp> // gyper::MapQ

namespace gyper
{


struct HapStats
{
  std::vector<uint8_t> hap_coverage;
  std::vector<uint8_t> hap_unique_coverage;
  //std::vector<std::vector<std::vector<char> > > predecessor;
  //std::vector<std::vector<std::vector<char> > > successor;
  std::vector<std::vector<std::pair<uint32_t, uint32_t> > > pair_info;
};


struct AlleleCoverage
{
  uint16_t total = 0;
};


struct HapSample
{
  /** Likelihood score of each haplotype. */
  std::vector<uint16_t> log_score{};

  /** Coverage of each allele. */
  std::vector<std::vector<uint16_t> > gt_coverage{};

  uint16_t max_log_score = 0u;

  /** Further statistics are only calculated when --stats option is used. Therefore only save a pointer to the other details. */
  std::unique_ptr<HapStats> stats{nullptr};

  /**
   * INFO
   */
  uint8_t inline get_ambiguous_depth() const {return ambiguous_depth;}
  uint8_t inline get_ambiguous_depth_alt() const {return ambiguous_depth_alt;}
  uint8_t inline get_alt_proper_pair_depth() const {return alt_proper_pair_depth;}

  /**
   * MODIFIERS
   */
  void increment_ambiguous_depth();
  void increment_ambiguous_depth_alt();
  void increment_allele_depth(std::size_t variant_index,
                              std::size_t allele_index
                              );
  void increment_alt_proper_pair_depth();

private:
  /** PRIVATE MEMBERS */
  uint8_t ambiguous_depth = 0u;
  uint8_t ambiguous_depth_alt = 0u;
  uint8_t alt_proper_pair_depth = 0u;
};


class Haplotype
{
public:
  std::vector<Genotype> gts; /** \brief A list of genotypes this haplotype has. */
  std::vector<HapSample> hap_samples;

  /** \brief All calls of each sample. This is used for segment calling only. */
  std::vector<std::pair<uint16_t, uint16_t> > calls;

  std::vector<VarStats> var_stats;

  Haplotype() noexcept;

  /*******************
   * CLASS MODIFIERS *
   *******************/
  void add_genotype(Genotype && gt);
  void check_for_duplicate_haplotypes();
  void clear_and_resize_samples(std::size_t new_size);
  void clear();

  void add_coverage(uint32_t local_genotype_id, uint16_t c);
  void add_explanation(uint32_t local_genotype_id, std::bitset<MAX_NUMBER_OF_HAPLOTYPES> const & e);
  void update_max_log_score();

  /*********************
   * CLASS INFORMATION *
   *********************/
  std::vector<uint16_t> get_haplotype_calls() const;
  uint32_t get_genotype_num() const;
  bool has_too_many_genotypes() const;
  std::vector<uint32_t> get_genotype_ids() const;

  uint32_t best_score_of_a_path(std::size_t pn_index,
                                std::bitset<MAX_NUMBER_OF_HAPLOTYPES> const & e1,
                                std::bitset<MAX_NUMBER_OF_HAPLOTYPES> const & e2
                                ) const;

  /** Update likelihood and stats */
  void explain_to_score(std::size_t pn_index,
                        bool non_unique_paths,
                        uint8_t mapq,
                        bool fully_aligned,
                        std::size_t mismatches
                        );

  void clipped_reads_to_stats(bool fully_aligned);
  void graph_complexity_to_stats();
  void mapq_to_stats(uint8_t mapq);
  void realignment_to_stats(bool is_unaligned_read,
                            bool is_originally_clipped,
                            uint32_t original_pos,
                            uint32_t new_pos
    );

  void strand_to_stats(bool forward_strand, bool is_first_in_pair);
  void coverage_to_gts(std::size_t pn_index, bool is_proper_pair);
  std::bitset<MAX_NUMBER_OF_HAPLOTYPES> explain_to_path_explain();


  uint16_t static constexpr NO_COVERAGE = 0xFFFFu;
  uint16_t static constexpr MULTI_ALT_COVERAGE = 0xFFFEu;
  uint16_t static constexpr MULTI_REF_COVERAGE = 0xFFFDu;

  /** Coverage per gt. Contains the called allele, unless one it is one of:
   *   - NO_COVERAGE: No allele is supported
   *   - MULTI_ALT_COVERAGE: Multiple alleles are supported, none of which is the reference allele.
   *   - MULTI_REF_COVERAGE: Multiple alleles are supported, one of which is the reference allele.
   */

private:
  std::vector<uint16_t> coverage; // per gt
  std::vector<std::bitset<MAX_NUMBER_OF_HAPLOTYPES> > explains; // per gt

  std::bitset<MAX_NUMBER_OF_HAPLOTYPES> find_which_haplotypes_explain_the_read(uint32_t cnum) const;
  std::vector<uint16_t> find_with_how_many_errors_haplotypes_explain_the_read(uint32_t cnum) const;
};

} // namespace gyper
