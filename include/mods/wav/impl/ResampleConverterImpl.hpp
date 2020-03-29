#ifndef MODS_WAV_IMPL_RESAMPLECONVERTERIMPL_HPP
#define MODS_WAV_IMPL_RESAMPLECONVERTERIMPL_HPP

#include <vector>

namespace mods
{
   namespace wav
     {
        namespace impl
          {
             struct SampleWithZeros
               {
                  SampleWithZeros(double sample, int zeros);
                  
                  SampleWithZeros() = default;
                  SampleWithZeros(const SampleWithZeros&) = delete;
                  SampleWithZeros(SampleWithZeros&&) = delete;
                  SampleWithZeros& operator=(const SampleWithZeros&) = default;
                  SampleWithZeros& operator=(SampleWithZeros&&) = delete;
                  ~SampleWithZeros() = default;
                  
                  int numberOfZeros;
                  double sample;
               };
             
             class History
               {
                public:
                  History(int numTaps);
                  
                  History() = delete;
                  History(const History&) = delete;
                  History(History&&) = delete;
                  History& operator=(const History&) = delete;
                  History& operator=(History&&) = delete;
                  ~History() = default;
                  
                  void push_back(const SampleWithZeros& sampleWithZeros);
                  SampleWithZeros& front();
                  SampleWithZeros& back();
                  void pop_front();
                  bool isEmpty() const;
                  const SampleWithZeros& getSample(size_t i);
                  
                private:
                  std::vector<SampleWithZeros> _v;
                  size_t _begin = 0;
                  size_t _end = 0;
                  
                  SampleWithZeros _zeros;
               };
          } // namespace impl
     } // namespace wav
} // namespace mods

#endif // MODS_WAV_IMPL_RESAMPLECONVERTERIMPL_HPP