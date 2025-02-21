#include <cassert>

#include <graphtyper/constants.hpp>
#include <graphtyper/graph/absolute_position.hpp>
#include <graphtyper/graph/graph.hpp>

#include <boost/log/trivial.hpp>

namespace gyper
{

AbsolutePosition::AbsolutePosition()
{
  this->calculate_offsets();
  /*
  offsets.resize(26);
  offsets[0] = 0;
  chromosome_to_offset[chromosome_names[0]] = 0;

  for (long i = 1; i < 26l; ++i)
  {
    offsets[i] = offsets[i - 1] + chromosome_lengths[i - 1];
    chromosome_to_offset[chromosome_names[i]] = offsets[i];
  }
  */
}


void
AbsolutePosition::calculate_offsets()
{
  if (gyper::graph.contigs.size() == 0 || gyper::graph.contigs.size() == offsets.size())
    return;

  offsets.clear();
  chromosome_to_offset.clear();

  offsets.resize(gyper::graph.contigs.size());
  offsets[0] = 0;
  chromosome_to_offset[gyper::graph.contigs[0].name] = 0;

  for (long i = 1; i < static_cast<long>(offsets.size()); ++i)
  {
    offsets[i] = offsets[i - 1] + gyper::graph.contigs[i - 1].length;
    chromosome_to_offset[gyper::graph.contigs[i].name] = offsets[i];
  }
}


bool
AbsolutePosition::is_contig_available(std::string const & contig) const
{
  return chromosome_to_offset.count(contig) > 0;
}


uint32_t
AbsolutePosition::get_absolute_position(std::string const & chromosome, uint32_t const contig_position) const
{
  uint32_t abs_pos;

  try
  {
    abs_pos = chromosome_to_offset.at(chromosome) + contig_position;
  }
  catch (std::out_of_range const &)
  {
    std::cerr << "[gyper::graph::absolute_position] ERROR: No chromosome \""
              << chromosome << "\" available. Available chromosomes are:\n";

    for (auto it = chromosome_to_offset.begin(); it != chromosome_to_offset.end(); ++it)
      std::cerr << it->first << "\n";

    std::cerr << std::endl;
    assert(false);
    std::exit(113);
  }

  return abs_pos;
}


std::pair<std::string, uint32_t>
AbsolutePosition::get_contig_position(uint32_t const absolute_position) const
{
  auto offset_it = std::lower_bound(offsets.begin(), offsets.end(), absolute_position);
  long const i = std::distance(offsets.begin(), offset_it);
  assert(i > 0);
  assert(i <= static_cast<long>(gyper::graph.contigs.size()));
  return std::make_pair<std::string, uint32_t>(std::string(gyper::graph.contigs[i - 1].name),
                                               absolute_position - offsets[i - 1]);
}


AbsolutePosition absolute_pos;

} // namespace gyper
