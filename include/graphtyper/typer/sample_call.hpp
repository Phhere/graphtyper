#pragma once

#include <cstdint> // uint8_t, uint16_t
#include <vector> // std::vector

namespace gyper
{

struct AlleleCoverage;
class SV;

class SampleCall
{
public:
  SampleCall() noexcept;
  SampleCall(std::vector<uint8_t> const & phred,
             std::vector<uint16_t> const & coverage,
             uint8_t ambiguous_depth,
             uint8_t ambiguous_depth_alt,
             uint8_t alt_proper_pair_depth
    ) noexcept;

  // If R is the number of alleles, then phred should be of size R * (R + 1) / 2 and coverage of size R.
  std::vector<uint8_t> phred; // GT and PL
  std::vector<uint16_t> coverage; // AD and DP
  uint16_t ref_total_depth = 0u; // Total reads that support the reference allele
  uint16_t alt_total_depth = 0u; // Total reads that support alternative allele(s)
  uint8_t ambiguous_depth = 0u; //
  uint8_t alt_proper_pair_depth = 0u; // Total reads in proper pairs that support the alternative allele(s)

  mutable int8_t filter = -1; // -1 is unknown, 0 is PASS, 1 is GQ filter

  uint32_t get_depth() const;
  uint32_t get_unique_depth() const;
  std::pair<uint16_t, uint16_t> get_gt_call() const;
  uint8_t get_gq() const;
  int8_t check_filter(long gq) const;


};

/**
 * \brief Create a biallelic call from a multi-allelic call
 * \param old_call Old multi-allelic call
 * \param aa index of alternative allele
 * \param pn_index index of the sample/PN
 * \param sv structural variant
 * \return Biallelic call for alternative allele at index aa
 */
SampleCall
make_bi_allelic_call(SampleCall const & old_call, long aa);

SampleCall
make_call_based_on_coverage(long pn_index, SV const & sv);

} // namespace gyper
