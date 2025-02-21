#include <catch.hpp>

#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>

#include <graphtyper/constants.hpp>
#include <graphtyper/graph/graph.hpp>
#include <graphtyper/graph/constructor.hpp>
#include <graphtyper/graph/graph_serialization.hpp>
#include <graphtyper/utilities/type_conversions.hpp>
#include <graphtyper/utilities/options.hpp>


namespace
{

bool inline
is_file(std::string const & filename)
{
  struct stat sb;

  if (stat(filename.c_str(), &sb) == 0 && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode)))
  {
    return true;
  }

  return false;
}


} // anon namespace


void
create_graph(std::string const & fasta,
             std::string const & vcf,
             std::string const & region,
             bool const use_absolute_positions = true
             )
{
  gyper::GenomicRegion const genomic_region(region);
  std::stringstream vcf_path;
  vcf_path << gyper_SOURCE_DIRECTORY << vcf;
  std::stringstream reference_path;
  reference_path << gyper_SOURCE_DIRECTORY << fasta;

  gyper::construct_graph(reference_path.str().c_str(), vcf_path.str().c_str(), region, use_absolute_positions);
  std::stringstream graph_path;
  graph_path << gyper_SOURCE_DIRECTORY << "/test/data/graphs";

  std::string graph_directory(graph_path.str());

  // Check if directory exists, and of not, create it
  struct stat st;

  if (stat(graph_directory.c_str(), &st) == -1)
  {
    mkdir(graph_directory.c_str(), 0755);
  }

  graph_path << fasta.substr(20, fasta.size() - 3 - 20) << '_' << genomic_region.chr << ".grf";
  REQUIRE(gyper::graph.size() > 0);
  REQUIRE(gyper::graph.check());

  {
    std::ofstream ofs(graph_path.str().c_str(), std::ios::binary);
    REQUIRE(ofs.is_open());
    boost::archive::binary_oarchive oa(ofs);
    oa << gyper::graph;
  }

  // test open
  {
    gyper::Graph new_graph;
    std::ifstream ifs(graph_path.str().c_str(), std::ios::binary);
    REQUIRE(ifs.is_open());
    boost::archive::binary_iarchive ia(ifs);
    ia >> new_graph;

    REQUIRE(new_graph.size() == gyper::graph.size());
    REQUIRE(new_graph.get_genomic_region().rID == gyper::graph.get_genomic_region().rID);
    REQUIRE(new_graph.get_genomic_region().chr == gyper::graph.get_genomic_region().chr);
    REQUIRE(new_graph.get_genomic_region().begin == gyper::graph.get_genomic_region().begin);
    REQUIRE(new_graph.get_genomic_region().end == gyper::graph.get_genomic_region().end);
  }
}


TEST_CASE("Construct test graph (chr1)")
{
  create_graph("/test/data/reference/index_test.fa", "/test/data/reference/index_test.vcf.gz", "chr1", true);

  REQUIRE(gyper::graph.ref_nodes.size() == 2);
  REQUIRE(gyper::graph.var_nodes.size() == 2);

  std::vector<gyper::RefNode> const & ref_nodes = gyper::graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = gyper::graph.var_nodes;

  SECTION("The nodes should be correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 2);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);

    REQUIRE(var_nodes[0].out_degree() == 1);
    REQUIRE(var_nodes[0].get_out_ref_index() == 1);

    REQUIRE(var_nodes[1].out_degree() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);

    REQUIRE(ref_nodes[1].out_degree() == 0);
  }

  SECTION("The nodes should have the correct order")
  {
    REQUIRE(ref_nodes[0].get_label().order == 1);
    REQUIRE(var_nodes[0].get_label().order == 37);
    REQUIRE(var_nodes[1].get_label().order == 37);
    REQUIRE(ref_nodes[1].get_label().order == 38);
  }

  SECTION("The nodes should have a label with the correct DNA bases")
  {
    REQUIRE(ref_nodes[0].get_label().dna == gyper::to_vec("AGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTT"));
    REQUIRE(var_nodes[0].get_label().dna == gyper::to_vec("C"));
    REQUIRE(var_nodes[1].get_label().dna == gyper::to_vec("G"));
    REQUIRE(ref_nodes[1].get_label().dna == gyper::to_vec("CCCAGGTTTCCCCAGGTTTCCCCTTTGGA"));
  }
}


TEST_CASE("Construct test graph (chr1) but with absolute positions")
{
  using namespace gyper;

  create_graph("/test/data/reference/index_test.fa", "/test/data/reference/index_test.vcf.gz", "chr1", true);

  REQUIRE(graph.ref_nodes.size() == 2);
  REQUIRE(graph.var_nodes.size() == 2);

  std::vector<gyper::RefNode> const & ref_nodes = graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = graph.var_nodes;

  SECTION("The nodes should be correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 2);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);

    REQUIRE(var_nodes[0].out_degree() == 1);
    REQUIRE(var_nodes[0].get_out_ref_index() == 1);

    REQUIRE(var_nodes[1].out_degree() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);

    REQUIRE(ref_nodes[1].out_degree() == 0);
  }

  SECTION("The nodes should have the correct order")
  {
    REQUIRE(ref_nodes[0].get_label().order == 0 + 1);
    REQUIRE(var_nodes[0].get_label().order == 36 + 1);
    REQUIRE(var_nodes[1].get_label().order == 36 + 1);
    REQUIRE(ref_nodes[1].get_label().order == 37 + 1);
  }

  SECTION("The nodes should have a label with the correct DNA bases")
  {
    REQUIRE(ref_nodes[0].get_label().dna == gyper::to_vec("AGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTT"));
    REQUIRE(var_nodes[0].get_label().dna == gyper::to_vec("C"));
    REQUIRE(var_nodes[1].get_label().dna == gyper::to_vec("G"));
    REQUIRE(ref_nodes[1].get_label().dna == gyper::to_vec("CCCAGGTTTCCCCAGGTTTCCCCTTTGGA"));
  }
}


TEST_CASE("Construct test graph (chr2)")
{
  using namespace gyper;

  create_graph("/test/data/reference/index_test.fa", "/test/data/reference/index_test.vcf.gz", "chr2", true);

  REQUIRE(graph.ref_nodes.size() == 3);
  REQUIRE(graph.var_nodes.size() == 4);

  std::vector<gyper::RefNode> const & ref_nodes = graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = graph.var_nodes;

  SECTION("The nodes should be correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 2);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);

    REQUIRE(var_nodes[0].get_out_ref_index() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);

    REQUIRE(ref_nodes[1].get_var_index(0) == 2);
    REQUIRE(ref_nodes[1].get_var_index(1) == 3);

    REQUIRE(var_nodes[2].get_out_ref_index() == 2);
    REQUIRE(var_nodes[3].get_out_ref_index() == 2);

    REQUIRE(ref_nodes[2].out_degree() == 0);
  }

  SECTION("The nodes should have the correct order")
  {
    REQUIRE(ref_nodes[0].get_label().order == 0 + 67);
    REQUIRE(var_nodes[0].get_label().order == 1 + 67);
    REQUIRE(var_nodes[1].get_label().order == 1 + 67);
    REQUIRE(ref_nodes[1].get_label().order == 2 + 67);
    REQUIRE(var_nodes[2].get_label().order == 2 + 67);
    REQUIRE(var_nodes[3].get_label().order == 2 + 67);
    REQUIRE(ref_nodes[2].get_label().order == 3 + 67);
  }

  SECTION("The nodes should have a label with the correct DNA bases")
  {
    // CCCCAGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTTGGACCC
    REQUIRE(ref_nodes[0].get_label().dna == gyper::to_vec("C"));
    REQUIRE(var_nodes[0].get_label().dna == gyper::to_vec("C"));
    REQUIRE(var_nodes[1].get_label().dna == gyper::to_vec("A"));
    REQUIRE(ref_nodes[1].get_label().dna == gyper::to_vec(""));
    REQUIRE(var_nodes[2].get_label().dna == gyper::to_vec("C"));
    REQUIRE(var_nodes[3].get_label().dna == gyper::to_vec("A"));
    REQUIRE(ref_nodes[2].get_label().dna == gyper::to_vec("CAGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTTGGACCC"));
  }
}


TEST_CASE("Construct test graph (chr3)")
{
  using namespace gyper;

  // AAAACAAAATAAAACAAAATAAAAGAAAACAAAATAAAACAAAATAAAAGAAAACATTATAAAACA
  // chr3 31 rs4 A G,GA

  create_graph("/test/data/reference/index_test.fa", "/test/data/reference/index_test.vcf.gz", "chr3", true);

  std::vector<gyper::RefNode> const & ref_nodes = graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = graph.var_nodes;

  SECTION("Nodes are correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 3);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);
    REQUIRE(ref_nodes[0].get_var_index(2) == 2);

    REQUIRE(var_nodes[0].get_out_ref_index() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);
    REQUIRE(var_nodes[2].get_out_ref_index() == 1);
  }

  SECTION("Nodes have the correct order")
  {
    REQUIRE(ref_nodes[0].get_label().order == 0  + 133);
    REQUIRE(var_nodes[0].get_label().order == 30 + 133);
    REQUIRE(var_nodes[1].get_label().order == 30 + 133);
    REQUIRE(var_nodes[2].get_label().order == 30 + 133);
    REQUIRE(ref_nodes[1].get_label().order == 31 + 133);
  }

  SECTION("Nodes have the correct bases")
  {
    REQUIRE(graph.ref_nodes.size() == 2);
    REQUIRE(graph.ref_nodes[0].get_label().dna == gyper::to_vec("AAAACAAAATAAAACAAAATAAAAGAAAAC"));
    REQUIRE(graph.ref_nodes[1].get_label().dna == gyper::to_vec("AAATAAAACAAAATAAAAGAAAACATTATAAAACA"));
    REQUIRE(graph.var_nodes.size() == 3);
    REQUIRE(graph.var_nodes[0].get_label().dna == gyper::to_vec("A"));
    REQUIRE(graph.var_nodes[1].get_label().dna == gyper::to_vec("G"));
    REQUIRE(graph.var_nodes[2].get_label().dna == gyper::to_vec("GA"));

    REQUIRE(graph.actual_poses.size() == 1);
    REQUIRE(graph.actual_poses[0] == 31 + 133);
    REQUIRE(graph.ref_reach_poses.size() == 1);
    REQUIRE(graph.ref_reach_poses[0] == 30 + 133);
    REQUIRE(std::distance(graph.ref_reach_to_special_pos.begin(), graph.ref_reach_to_special_pos.end()) == 1);
    REQUIRE(graph.ref_reach_to_special_pos.count(30 + 133) == 1);
  }
}


TEST_CASE("Construct test graph (chr8) in a region that fully overlaps only a second indel")
{
  using namespace gyper;
  Options::instance()->add_all_variants = false;

  // TGCAAATCTCATATATATATATATATATATATATATATATATATATTTTTTTTTTTTTTTTTTTTTTTTTTTTTTCCAA
  //chr8 31 ATATATATATATATATTTTTTTTTTTT,A
  //chr8 39 ATATATATTTTTTTTTTT,A


  create_graph("/test/data/reference/index_test.fa", "/test/data/reference/index_test.vcf.gz", "chr8:1-56", true);

  std::vector<gyper::RefNode> const & ref_nodes = graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = graph.var_nodes;

  SECTION("Nodes are correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 2);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);

    REQUIRE(var_nodes[0].get_out_ref_index() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);
  }

  SECTION("Nodes have the correct order")
  {
    REQUIRE(ref_nodes[0].get_label().order == 0  + 1105);
    REQUIRE(var_nodes[0].get_label().order == 38 + 1105);
    REQUIRE(var_nodes[1].get_label().order == 38 + 1105);
    REQUIRE(ref_nodes[1].get_label().order == 56 + 1105);
  }

  SECTION("Nodes have the correct bases")
  {
    REQUIRE(graph.ref_nodes.size() == 2);
    REQUIRE(graph.ref_nodes[0].get_label().dna == gyper::to_vec("TGCAAATCTCATATATATATATATATATATATATATAT"));
    REQUIRE(graph.ref_nodes[1].get_label().dna == gyper::to_vec(""));
    REQUIRE(graph.var_nodes.size() == 2);
    REQUIRE(graph.var_nodes[0].get_label().dna == gyper::to_vec("ATATATATTTTTTTTTTT"));
    REQUIRE(graph.var_nodes[1].get_label().dna == gyper::to_vec("A"));

    REQUIRE(graph.actual_poses.size() == 0);
  }
}


/*
TEST_CASE("Construct test graph with SV deletion (chr5)")
{
  using namespace gyper;
  using gyper::to_vec;

  // 70A 70C 70G 70T
  // chr5 70 A70C SVTYPE=DEL,SVSIZE=70

  create_graph("/test/data/reference/index_test.fa", "/test/data/reference/index_test.vcf.gz", "chr5", false);

  std::vector<gyper::RefNode> const & ref_nodes = graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = graph.var_nodes;

  REQUIRE(ref_nodes.size() == 2);
  REQUIRE(var_nodes.size() == 2);

  SECTION("Nodes are correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 2);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);

    REQUIRE(var_nodes[0].get_out_ref_index() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);
  }

  SECTION("The nodes have the correct bases")
  {
    REQUIRE(ref_nodes[0].get_label().dna == to_vec("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
    REQUIRE(var_nodes[0].get_label().dna == to_vec("A"));
    REQUIRE(var_nodes[1].get_label().dna == to_vec("AGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT<SV:0000000<"));
    REQUIRE(ref_nodes[1].get_label().dna == to_vec("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"));
  }

  SECTION("The nodes should have the correct order")
  {
    REQUIRE(ref_nodes[0].get_label().order == 0);
    REQUIRE(var_nodes[0].get_label().order == 69);
    REQUIRE(var_nodes[1].get_label().order == 69);
    REQUIRE(ref_nodes[1].get_label().order == 70);
  }
}


TEST_CASE("Construct test graph with SV duplication (chr6)")
{
  using namespace gyper;

  create_graph("/test/data/reference/index_test.fa", "/test/data/reference/index_test.vcf.gz", "chr6", false);

  std::vector<gyper::RefNode> const & ref_nodes = graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = graph.var_nodes;

  REQUIRE(ref_nodes.size() == 3);
  REQUIRE(var_nodes.size() == 4);

  SECTION("Nodes are correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 2);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);

    REQUIRE(var_nodes[0].get_out_ref_index() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);

    REQUIRE(ref_nodes[1].out_degree() == 2);
    REQUIRE(ref_nodes[2].out_degree() == 0);
  }

  SECTION("The nodes have the correct bases")
  {
    REQUIRE(ref_nodes[0].get_label().dna == to_vec("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACCCC"));
    REQUIRE(var_nodes[0].get_label().dna == to_vec("C"));
    REQUIRE(var_nodes[1].get_label().dna == to_vec("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGTTTTTTTTTTTT<SV:0000000>AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACCCCCCCCCC"));
    std::vector<char> const & ref_dna = ref_nodes[1].get_label().dna;
    REQUIRE(ref_dna.size() >= 66);
    REQUIRE(std::vector<char>(ref_dna.begin(), ref_dna.begin() + 66) == to_vec("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCG"));
  }

  SECTION("The nodes should have the correct order")
  {
    REQUIRE(ref_nodes[0].get_label().order == 0);
    REQUIRE(var_nodes[0].get_label().order == 74);
    REQUIRE(var_nodes[1].get_label().order == 74);
    REQUIRE(ref_nodes[1].get_label().order == 75);
  }
}


TEST_CASE("Construct test graph with SV duplication (chr7)")
{
  using namespace gyper;

  create_graph("/test/data/reference/index_test.fa", "/test/data/reference/index_test.vcf.gz", "chr7", false);

  std::vector<gyper::RefNode> const & ref_nodes = graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = graph.var_nodes;

  REQUIRE(ref_nodes.size() == 2);
  REQUIRE(var_nodes.size() == 3);

  SECTION("Nodes are correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 3);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);
    REQUIRE(ref_nodes[0].get_var_index(2) == 2);

    REQUIRE(var_nodes[0].get_out_ref_index() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);
  }

  SECTION("The nodes have the correct bases")
  {
    REQUIRE(ref_nodes[0].get_label().dna == to_vec("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));
    REQUIRE(var_nodes[0].get_label().dna == to_vec("A"));
    //REQUIRE(var_nodes[1].get_label().dna == to_vec("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGTTTTTTTTTTTT<SV:0000000>AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACCCCCCCCCC"));
    std::vector<char> const & ref_dna = ref_nodes[1].get_label().dna;
    REQUIRE(ref_dna.size() >= 71);
    REQUIRE(std::vector<char>(ref_dna.begin(), ref_dna.begin() + 71) == to_vec("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCG"));
  }

  //SECTION("The nodes should have the correct order")
  //{
  //  REQUIRE(ref_nodes[0].get_label().order == 0);
  //  REQUIRE(var_nodes[0].get_label().order == 74);
  //  REQUIRE(var_nodes[1].get_label().order == 74);
  //  REQUIRE(ref_nodes[1].get_label().order == 75);
  //}
}
*/


/*
TEST_CASE("Construct test graph (chromosome 1, without chr in front)")
{
  using namespace gyper;

  // AAAACAAAATAAAACAAAATAAAAGAAAACAAAATAAAACAAAATAAAAGAAAACATTATAAAACA

  create_graph("/test/data/reference/index_test_b37.fa", "/test/data/reference/index_test_b37.vcf.gz", "1", true);

  REQUIRE(gyper::graph.ref_nodes.size() == 2);
  REQUIRE(gyper::graph.var_nodes.size() == 2);

  std::vector<gyper::RefNode> const & ref_nodes = gyper::graph.ref_nodes;
  std::vector<gyper::VarNode> const & var_nodes = gyper::graph.var_nodes;

  SECTION("The nodes should be correctly connected")
  {
    REQUIRE(ref_nodes[0].out_degree() == 2);
    REQUIRE(ref_nodes[0].get_var_index(0) == 0);
    REQUIRE(ref_nodes[0].get_var_index(1) == 1);

    REQUIRE(var_nodes[0].out_degree() == 1);
    REQUIRE(var_nodes[0].get_out_ref_index() == 1);

    REQUIRE(var_nodes[1].out_degree() == 1);
    REQUIRE(var_nodes[1].get_out_ref_index() == 1);

    REQUIRE(ref_nodes[1].out_degree() == 0);
  }

  SECTION("The nodes should have the correct order")
  {
    REQUIRE(ref_nodes[0].get_label().order == 0 + 1);
    REQUIRE(var_nodes[0].get_label().order == 36 + 1);
    REQUIRE(var_nodes[1].get_label().order == 36 + 1);
    REQUIRE(ref_nodes[1].get_label().order == 37 + 1);
  }

  SECTION("The nodes should have a label with the correct DNA bases")
  {
    REQUIRE(ref_nodes[0].get_label().dna == gyper::to_vec("AGGTTTCCCCAGGTTTCCCCAGGTTTCCCCAGGTTT"));
    REQUIRE(var_nodes[0].get_label().dna == gyper::to_vec("C"));
    REQUIRE(var_nodes[1].get_label().dna == gyper::to_vec("A"));
    REQUIRE(ref_nodes[1].get_label().dna == gyper::to_vec("CCCAGGTTTCCCCAGGTTTCCCCTTTGGA"));
  }
}
*/
