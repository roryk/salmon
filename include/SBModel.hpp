#ifndef __SB_MODEL_HPP__
#define __SB_MODEL_HPP__

#include "jellyfish/mer_dna.hpp"
#include "UtilityFunctions.hpp"
#include <Eigen/Dense>
#include <cmath>

using Mer = jellyfish::mer_dna_ns::mer_base_static<uint64_t, 4>;

class SBModel {
public:
  SBModel();   
  inline int32_t contextBefore(bool rc) { return rc ? _contextRight : _contextLeft; }
  inline int32_t contextAfter(bool rc) { return rc ? _contextLeft : _contextRight; }

  inline bool addSequence(const char* seqIn, bool revCmp, double weight = 1.0) {
    _mer.from_chars(seqIn);
    if (revCmp) { _mer.reverse_complement(); }
    for (int32_t i = 0; i < _contextLength; ++i) {
      uint32_t idx = _mer.get_bits(_shifts[i], _widths[i]);
      _probs(idx, i) += weight;
    }
    return true;
  }

  Eigen::MatrixXd& counts();
  Eigen::MatrixXd& marginals();
  
  inline double evaluateLog(const char* seqIn) {
    double p = 0;
    Mer mer;
    mer.from_chars(seqIn);

    for (int32_t i = 0; i < _contextLength; ++i) {
      uint32_t idx = mer.get_bits(_shifts[i], _widths[i]);
      p += _probs(idx, i);
    }
    return p;
  }

  bool normalize();

  bool checkTransitionProbabilities();
  
  void combineCounts(const SBModel& other);
 
  int32_t getContextLength(); 

  template <typename CountVecT>
  bool train(CountVecT& kmerCounts, const uint32_t K);
  
  inline double evaluate(uint32_t kmer, uint32_t K) {
    std::vector<uint32_t> _order{0, 0, 2,2,2,2};
    double p{1.0};
    for (int32_t pos = 0; pos < K - _order.back(); ++pos) {
      uint32_t offset = 2 * (K - (pos + 1) - _order[pos]);
      auto idx = _getIndex(kmer, offset, _order[pos]);
      p *= _probs(idx, pos);
    }
    return p;
  }

private:
  inline uint32_t _getIndex(uint32_t kmer, uint32_t offset, uint32_t _order) {
    kmer >>= offset;
    switch (_order) {
    case 0:
      return kmer & 0x3;
    case 1:
      return kmer & 0xF;
    case 2:
      return kmer & 0x3F;
    default:
      return 0;
    }
    return 0;
  }
  bool _trained;

  int32_t _contextLength;
  int32_t _contextLeft;
  int32_t _contextRight;

  Eigen::MatrixXd _probs;
  Eigen::MatrixXd _marginals;

  Mer _mer;
  std::vector<int32_t> _order;
  std::vector<int32_t> _shifts;
  std::vector<int32_t> _widths;
};

#endif //__SB_MODEL_HPP__
