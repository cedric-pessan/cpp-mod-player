
#include "mods/utils/OpenCLManager.hpp"
#include "mods/utils/TransformedCollection.hpp"
#include "mods/wav/OpenCLResampleConverter.hpp"
#include "mods/wav/ResampleParameters.hpp"

#include <sstream>

#ifdef WITH_OPENCL
namespace mods
{
   namespace wav
     {
        template<typename PARAMETERS>
          OpenCLResampleConverter<PARAMETERS>::OpenCLResampleConverter(WavConverter::ptr src, PARAMETERS resampleParameters)
            : _src(std::move(src)),
          _resampleParameters(std::move(resampleParameters)),
          _inputVec(((_resampleParameters.getNumTaps() / _resampleParameters.getInterpolationFactor())+1) * sizeof(double)),
          _inputBuffer(initBuffer()),
          _inputBufferAsDouble(_inputBuffer.slice<double>(0, _inputVec.size() / sizeof(double))),
          _currentSample(_resampleParameters.getNumTaps()),
          _history(_resampleParameters.getNumTaps()),
          _context(mods::utils::OpenCLManager::getContext()),
          _queue(_context),
          _firFilterProgram(compileFirFilterProgram()),
          _firFilterKernel(_firFilterProgram, "firFilter"),
          _tapsBuffer(buildTapsBuffer())
            {
               using impl::SampleWithZeros;
               _history.push_back(SampleWithZeros(0.0, _resampleParameters.getNumTaps()-1));
            }
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::initBuffer() -> mods::utils::RWBuffer<u8>
          {
             u8* ptr = _inputVec.data();
             auto deleter = std::make_unique<mods::utils::RWBufferBackend::EmptyDeleter>();
             auto buffer = std::make_shared<mods::utils::RWBufferBackend>(ptr, _inputVec.size(), std::move(deleter));
             return mods::utils::RWBuffer<u8>(buffer);
          }
        
        namespace
          {
             const std::string firKernelSource = R"OpenCL(

               kernel void firFilter(global double* output,
                                     global const int* zeros,
                                     global const double* samples,
                                     global const double* taps) {
                 unsigned int idxItem = get_global_id(0);
                 unsigned int toSkip = idxItem * decimationFactor;
                 double sample = 0.0;

                 // position and do first tap
                 unsigned int currentIdx = 0;
                 unsigned int remainingZeros = zeros[currentIdx];
                 while(toSkip > 0) {
                   if(remainingZeros > 0) {
                     if(toSkip > remainingZeros) {
                       toSkip -= remainingZeros;
                       remainingZeros = 0;
                     } else {
                       remainingZeros -= toSkip;
                       toSkip = 0;
                     }
                   } else {
                     --toSkip;
                     ++currentIdx;
                     remainingZeros = zeros[currentIdx];
                   }
                 }
                 
                 if(remainingZeros < numTaps) {
                   sample += samples[currentIdx++] * interpolationFactor * taps[remainingZeros+1];
                 }
                 
                 // loop after first tap
                 for(unsigned int i=remainingZeros+2; i<numTaps; ++i) {
                   i += zeros[currentIdx];
                   if(i < numTaps) {
                     sample += samples[currentIdx++] * interpolationFactor * taps[i];
                   }
                 }
                 
                 output[idxItem] = sample;
               }

             )OpenCL";
          } // namespace
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::compileFirFilterProgram() -> cl::Program
          {
             try
               {
                  std::stringstream ss;
                  ss << "constant unsigned int interpolationFactor = " << _resampleParameters.getInterpolationFactor() << ";" << std::endl;
                  ss << "constant unsigned int decimationFactor = " << _resampleParameters.getDecimationFactor() << ";" << std::endl;
                  ss << "constant unsigned int numTaps = " << _resampleParameters.getNumTaps() << ";" << std::endl;
                  ss << firKernelSource;
                  
                  cl::Program program(_context, ss.str(), true);
                  return program;
               }
             catch(cl::BuildError& buildError)
               {
                  auto buildLog = buildError.getBuildLog();
                  for(auto& error : buildLog)
                    {
                       std::cout << "Error while compiling fir filter for device " << error.first.getInfo<CL_DEVICE_NAME>() << std::endl;
                       std::cout << error.second << std::endl;
                    }
               }
             return cl::Program();
          }
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::buildTapsBuffer() -> cl::Buffer
          {
             std::vector<double> taps(_resampleParameters.getTaps().begin(),
                                      _resampleParameters.getTaps().end());
             return cl::Buffer(_context, taps.begin(), taps.end(), true, true);
          }
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::isFinished() const -> bool
          {
             return _src->isFinished();
          }
        
        template<typename PARAMETERS>
          void OpenCLResampleConverter<PARAMETERS>::read(mods::utils::RWBuffer<u8>* buf, size_t len)
            {
               if((len % sizeof(double)) != 0)
                 {
                    std::cout << "TODO: wrong buffer length in OpenCLResemapleConverter" << std::endl;
                 }
               
               auto nbElems = len / sizeof(double);
               
               auto outView = buf->slice<double>(0, nbElems);
               
               for(size_t i=0; i<nbElems; ++i)
                 {
                    addToHistory();
                 }
               
               using SampleHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<double&(impl::SampleWithZeros&)>>;
               using ZeroHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<int&(impl::SampleWithZeros&)>>;
               
               SampleHistoryCollection sampleHistory(_history, [](impl::SampleWithZeros& sampleWithZeros) -> double& {
                  return sampleWithZeros.sample();
               });
               ZeroHistoryCollection zeroHistory(_history, [](impl::SampleWithZeros& sampleWithZeros) -> int& {
                  return sampleWithZeros.numberOfZeros();
               });
               
               cl::Buffer sampleBuffer(_context, sampleHistory.begin(), sampleHistory.end(), true);
               cl::Buffer zerosBuffer(_context, zeroHistory.begin(), zeroHistory.end(), true);
               cl::Buffer outputBuffer(_context, CL_MEM_WRITE_ONLY, nbElems * sizeof(double));
               
               cl::NDRange global(nbElems);
               _firFilterKernel(cl::EnqueueArgs(_queue, global),
                                outputBuffer, zerosBuffer, sampleBuffer, _tapsBuffer);
               
               cl::copy(_queue, outputBuffer, outView.begin(), outView.end());
               
               for(size_t i=0; i<nbElems; ++i)
                 {
                    removeFromHistory();
                 }
            }
        
        template<typename PARAMETERS>
          void OpenCLResampleConverter<PARAMETERS>::addToHistory()
            {
               using impl::SampleWithZeros;
               
               int toAdd = _resampleParameters.getDecimationFactor();
               while(toAdd > 0)
                 {
                    if(toAdd <= _zerosToNextInterpolatedSample)
                      {
                         bool merged = false;
                         if(!_history.isEmpty())
                           {
                              auto& latestElement = _history.back();
                              if(latestElement.sample() == 0.0)
                                {
                                   latestElement.numberOfZeros() += toAdd;
                                   merged = true;
                                }
                           }
                         if(!merged)
                           {
                              _history.push_back(SampleWithZeros(0.0, toAdd-1));
                           }
                         _zerosToNextInterpolatedSample -= toAdd;
                         toAdd = 0;
                      }
                    else
                      {
                         if(nextSampleExists())
                           {
                              double sample = getNextSample();
                              bool merged = false;
                              if(!_history.isEmpty())
                                {
                                   auto& latestElement = _history.back();
                                   if(latestElement.sample() == 0.0)
                                     {
                                        latestElement.numberOfZeros() += (1 + _zerosToNextInterpolatedSample);
                                        latestElement.sample() = sample;
                                        merged = true;
                                     }
                                }
                              if(!merged)
                                {
                                   _history.push_back(SampleWithZeros(sample, _zerosToNextInterpolatedSample));
                                }
                              toAdd -= (_zerosToNextInterpolatedSample + 1);
                              _zerosToNextInterpolatedSample = _resampleParameters.getInterpolationFactor() - 1;
                           }
                         else 
                           {
                              toAdd = 0;
                           }
                      }
                 }
            }
        
        template<typename PARAMETERS>
          void OpenCLResampleConverter<PARAMETERS>::removeFromHistory()
            {
               int toRemove = _resampleParameters.getDecimationFactor();
               while(toRemove > 0)
                 {
                    auto& oldestElement = _history.front();
                    if(oldestElement.numberOfZeros() > 0)
                      {
                         if(oldestElement.numberOfZeros() >= toRemove)
                           {
                              oldestElement.numberOfZeros() -= toRemove;
                              toRemove = 0;
                           }
                         else
                           {
                              toRemove -= oldestElement.numberOfZeros();
                              oldestElement.numberOfZeros() = 0;
                           }
                      }
                    else
                      {
                         _history.pop_front();
                         --toRemove;
                      }
                 }
            }
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::getNextSample() -> double
          {
             if(_currentSample >= _inputBufferAsDouble.size())
               {
                  _src->read(&_inputBuffer, _inputVec.size());
                  _currentSample = 0;
               }
             return _inputBufferAsDouble[_currentSample++];
          }
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::nextSampleExists() const -> bool
            {
               return _currentSample < _inputBufferAsDouble.size() || !_src->isFinished();
            }
        
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>>;
	template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<DynamicResampleParameters>;
     } // namespace wav
} // namespace mods
#endif // WITH_OPENCL
