#ifndef MODS_CONVERTERS_IMPL_RESAMPLECONVERTERIMPL_HPP
#define MODS_CONVERTERS_IMPL_RESAMPLECONVERTERIMPL_HPP

#include <vector>

namespace mods
{
   namespace converters
     {
        namespace impl
          {
             class SampleWithZeros
               {
                public:
                  SampleWithZeros(double sample, bool filtered, int zeros);
                  
                  SampleWithZeros() = default;
                  SampleWithZeros(const SampleWithZeros&) = delete;
                  SampleWithZeros(SampleWithZeros&&) = delete;
                  auto operator=(const SampleWithZeros&) -> SampleWithZeros& = default;
                  auto operator=(SampleWithZeros&&) -> SampleWithZeros& = delete;
                  ~SampleWithZeros() = default;
                  
                  auto isMergableWith(const SampleWithZeros& neighborSample) const -> bool;
                  
                  void setNumberOfZeros(int numberOfZeros);
                  auto getNumberOfZeros() const -> int;
                  auto getNumberOfZerosReference() -> int&;
                  void setSample(double sample);
                  auto getSample() const -> double;
                  auto getSampleReference() -> double&;
                  auto isFiltered() const -> bool;
                  void setRepeatCount(int repeatCount);
                  auto getRepeatCount() const -> int;
                  auto getRepeatCountReference() -> int&;
                  
                private:
                  int _numberOfZeros = 0;
                  double _sample = 0.0;
                  bool _filtered = false;
                  int _repeatCount = 1;
               };
             
             class History
               {
                private:
                  class Iterator;
                  
                public:
                  using value_type = SampleWithZeros;
                  using reference = value_type&;
                  using const_reference = const value_type&;
                  using pointer = value_type*;
                  using const_pointer = const value_type*;
                  using iterator = Iterator;
                  using const_iterator= Iterator;
                  using reverse_iterator= std::reverse_iterator<iterator>;
                  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
                  using difference_type = ptrdiff_t;
                  using size_type = size_t;
                  
                  explicit History(int numTaps);
                  
                  History() = delete;
                  History(const History&) = delete;
                  History(History&&) = delete;
                  auto operator=(const History&) -> History& = delete;
                  auto operator=(History&&) -> History& = delete;
                  ~History() = default;
                  
                  void push_back(const SampleWithZeros& sampleWithZeros);
                  auto front() -> SampleWithZeros&;
                  auto back() -> SampleWithZeros&;
                  void pop_front();
                  void pop_back();
                  auto isEmpty() const -> bool;
                  auto getSample(size_t num) -> SampleWithZeros&;
                  auto size() const -> size_t;
                  
                  void tryToMergeLast2Elements();
                  auto popFrontAndUnmergeNextElement(int maxToRemove) -> int;
                  
                  auto begin() -> iterator;
                  auto end() -> iterator;
                  
                private:
                  static constexpr int _sizeOfZeroChunksReturnedWhenEmpty = 1000000;
                  
                  std::vector<SampleWithZeros> _v;
                  size_t _begin = 0;
                  size_t _end = 0;
                  
                  SampleWithZeros _zeros;
                  
                  class Iterator
                    {
                     public:
                       Iterator(History* history, size_t idx);
                       
                       Iterator() = delete;
                       Iterator(const Iterator&) = default;
                       Iterator(Iterator&&) = default;
                       auto operator=(const Iterator&) -> Iterator& = delete;
                       auto operator=(Iterator&&) -> Iterator& = delete;
                       ~Iterator() = default;
                       
                       auto operator-(const Iterator& itRight) const -> difference_type;
                       
                       auto operator*() const -> reference;
                       
                       auto operator++() -> Iterator&;
                       
                     private:
                       History* _history;
                       size_t _idx;
                    };
               };
          } // namespace impl
     } // namespace converters
} // namespace mods

#endif // MODS_CONVERTERS_IMPL_RESAMPLECONVERTERIMPL_HPP