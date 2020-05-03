#ifndef MODS_WAV_IMPL_RESAMPLECONVERTERIMPL_HPP
#define MODS_WAV_IMPL_RESAMPLECONVERTERIMPL_HPP

#include <vector>

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             class SampleWithZeros
               {
                public:
                  SampleWithZeros(double sample, int zeros);
                  
                  SampleWithZeros() = default;
                  SampleWithZeros(const SampleWithZeros&) = delete;
                  SampleWithZeros(SampleWithZeros&&) = delete;
                  auto operator=(const SampleWithZeros&) -> SampleWithZeros& = default;
                  auto operator=(SampleWithZeros&&) -> SampleWithZeros& = delete;
                  ~SampleWithZeros() = default;
                  
                  auto numberOfZeros() -> int&;
                  auto numberOfZeros() const -> int;
                  auto sample() -> double&;
                  auto sample() const -> double;
                  
                private:
                  int _numberOfZeros;
                  double _sample;
               };
             
             class History
               {
                public:
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
                  auto isEmpty() const -> bool;
                  auto getSample(size_t i) -> const SampleWithZeros&;
                  
                private:
                  static constexpr int _sizeOfZeroChunksReturnedWhenEmpty = 1000000;
                  
                  std::vector<SampleWithZeros> _v;
                  size_t _begin = 0;
                  size_t _end = 0;
                  
                  SampleWithZeros _zeros;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_RESAMPLECONVERTERIMPL_HPP