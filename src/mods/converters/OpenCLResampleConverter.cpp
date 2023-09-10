
#include "mods/converters/OpenCLResampleConverter.hpp"
#include "mods/converters/ResampleParameters.hpp"
#include "mods/utils/OpenCLManager.hpp"
#include "mods/utils/TransformedCollection.hpp"

#include <sstream>

#ifdef WITH_OPENCL
namespace mods
{
   namespace converters
     {
        namespace impl
          {
             template<typename PARAMETERS, typename T>
               OpenCLResampleConverterBase<PARAMETERS, T>::OpenCLResampleConverterBase(typename Converter<T>::ptr src, PARAMETERS resampleParameters, bool rleInput)
                 : ResampleConverter<PARAMETERS, T>(std::move(src), std::move(resampleParameters)),
               _context(mods::utils::OpenCLManager::getContext()),
               _queue(_context),
               _firFilterProgram(compileFirFilterProgram(rleInput)),
               _firFilterKernel(_firFilterProgram, "firFilter"),
               _tapsBuffer(buildTapsBuffer())
                 {
                 }
             
             namespace
               {
                  auto getFirKernelSource() -> const std::string&
                    {
                       static const std::string firKernelSource = R"OpenCL(

                         kernel void firFilter(global double* output,
                                               global const int* zeros,
                                               global const double* samples,
                                               global const int* sampleRepeats,
                                               global const double* taps) {
                           unsigned int idxItem = get_global_id(0);
                           unsigned int toSkip = idxItem * decimationFactor;
                           double sample = 0.0;
                           unsigned int repeat = 0;

                           // position and do first tap
                           unsigned int currentIdx = 0;
                           unsigned int remainingZeros = zeros[currentIdx];
                           if(supportsRLE) {
                             repeat = sampleRepeats[currentIdx];
                           }
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
                               if(supportsRLE) {
                                 if(repeat > 1) {
                                   --repeat;
                                   remainingZeros = zeros[currentIdx];
                                   if(repeat > 1 && (remainingZeros + 1) * repeat < toSkip) {
                                     toSkip -= (remainingZeros + 1) * repeat;
                                     ++currentIdx;
                                     remainingZeros = zeros[currentIdx];
                                     repeat = sampleRepeats[currentIdx];
                                   }
                                 } else {
                                   ++currentIdx;
                                   remainingZeros = zeros[currentIdx];
                                   repeat = sampleRepeats[currentIdx];
                                 }
                               } else {
                                 ++currentIdx;
                                 remainingZeros = zeros[currentIdx];
                               }
                             }
                           }
                          
                           if(remainingZeros < numTaps) {
                             sample += samples[currentIdx] * interpolationFactor * taps[remainingZeros+1];
                             if(supportsRLE) {
                               if(repeat == 1) {
                                 ++currentIdx;
                                 repeat = sampleRepeats[currentIdx];
                               } else {
                                 --repeat;
                               }
                             } else {
                               ++currentIdx;
                             }
                           }
                     
                           // loop after first tap
                           for(unsigned int i=remainingZeros+2; i<numTaps; ++i) {
                             i += zeros[currentIdx];
                             if(i < numTaps) {
                               if(supportsRLE) {
                                 if(repeat == 1) {
                                   sample += samples[currentIdx++] * interpolationFactor * taps[i];
                                   repeat = sampleRepeats[currentIdx];
                                 } else {
                                   sample += samples[currentIdx] * interpolationFactor * taps[i];
                                   --repeat;
                                 }
                               } else {
                                 sample += samples[currentIdx++] * interpolationFactor * taps[i];
                               }
                             }
                           }
                           
                           output[idxItem] = sample;
                         }

                       )OpenCL";
                       
                       return firKernelSource;
                    }
               } // namespace
             
             template<typename PARAMETERS, typename T>
               auto OpenCLResampleConverterBase<PARAMETERS, T>::compileFirFilterProgram(bool rleInput) -> cl::Program
               {
                  try
                    {
                       auto& resampleParameters = this->getResampleParameters();
                       
                       std::stringstream ss;
                       ss << "constant unsigned int interpolationFactor = " << resampleParameters.getInterpolationFactor() << ";" << std::endl;
                       ss << "constant unsigned int decimationFactor = " << resampleParameters.getDecimationFactor() << ";" << std::endl;
                       ss << "constant unsigned int numTaps = " << resampleParameters.getNumTaps() << ";" << std::endl;
                       ss << "constant bool supportsRLE = " << (rleInput ? "true;" : "false;") << std::endl;
                       ss << getFirKernelSource();
                       
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
             
             template<typename PARAMETERS, typename T>
               auto OpenCLResampleConverterBase<PARAMETERS, T>::buildTapsBuffer() -> cl::Buffer
               {
                  auto& resampleParameters = this->getResampleParameters();
                  
                  _taps = std::vector<double>(resampleParameters.getTaps().begin(),
                                              resampleParameters.getTaps().end());
                  
                  return cl::Buffer(_context, _taps.begin(), _taps.end(), true, true);
               }
        
             template<typename PARAMETERS, typename T>
               void OpenCLResampleConverterBase<PARAMETERS, T>::resample(mods::utils::RWBuffer<double>* buf, const cl::Buffer& tapsBuffer)
                 {
                    auto nbElems = buf->size();
                    
                    fillBuffers(_context, _queue, &_sampleBuffer, &_zerosBuffer, &_repeatBuffer);
                    
                    if(!_outputBuffer.get() || _outputBuffer.getInfo<CL_MEM_SIZE>() < nbElems * sizeof(double))
                      {
                         _outputBuffer = cl::Buffer(_context, CL_MEM_WRITE_ONLY, nbElems * sizeof(double)); // NOLINT(hicpp-signed-bitwise)
                      }
                    
                    cl::NDRange global(nbElems);
                    _firFilterKernel(cl::EnqueueArgs(_queue, global),
                                     _outputBuffer, _zerosBuffer, _sampleBuffer, _repeatBuffer, tapsBuffer);
                    
                    cl::copy(_queue, _outputBuffer, buf->begin(), buf->end());
                 }
             
             template<typename PARAMETERS, typename T>
               auto OpenCLResampleConverterBase<PARAMETERS, T>::getTapsBuffer() const -> const cl::Buffer&
               {
                  return _tapsBuffer;
               }
          } // namespace impl
        
        template<typename PARAMETERS, typename T>
          OpenCLResampleConverter<PARAMETERS, T>::OpenCLResampleConverter(typename Converter<T>::ptr src, PARAMETERS resampleParameters)
            : impl::OpenCLResampleConverterBase<PARAMETERS, T>(std::move(src), std::move(resampleParameters), false)
              {
              }
        
        OpenCLResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::OpenCLResampleConverter(Converter<RLESample>::ptr src, AmigaResampleParameters  resampleParameters)
          : OpenCLResampleConverterBase<AmigaResampleParameters, RLESample>(std::move(src), std::move(resampleParameters), true),
          _ledFilterTapsBuffer(buildFilteredTapsBuffer())
            {
            }
        
        template<typename PARAMETERS, typename T>
          void OpenCLResampleConverter<PARAMETERS, T>::read(mods::utils::RWBuffer<double>* buf)
            {
               auto nbElems = buf->size();
               
               for(size_t i=0; i<nbElems; ++i)
                 {
                    this->addToHistory();
                 }
               
               this->resample(buf, this->getTapsBuffer());
               
               for(size_t i=0; i<nbElems; ++i)
                 {
                    this->removeFromHistory();
                 }
            }
        
        void OpenCLResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::read(mods::utils::RWBuffer<double>* buf)
          {
             auto nbElems = buf->size();
             auto& history = getHistory();
             
             size_t blockSize = nbElems;
             
             for(size_t i=0; i<nbElems; ++i)
               {
                  addToHistory();
                  if(_filtered != history.back().isFiltered())
                    {
                       blockSize = i+1;
                       break;
                    }
               }
             
             auto block = buf->slice<double>(0, blockSize);
             
             if(_filtered)
               {
                  resample(&block, _ledFilterTapsBuffer);
               }
             else
               {
                  resample(&block, getTapsBuffer());
               }
             
             _filtered = history.back().isFiltered();
             
             for(size_t i=0; i<blockSize; ++i)
               {
                  removeFromHistory();
               }
             
             if(blockSize != nbElems)
               {
                  auto nextBlock = buf->slice<double>(blockSize, nbElems - blockSize);
                  
                  read(&nextBlock);
               }
          }
        
        auto OpenCLResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::buildFilteredTapsBuffer() -> cl::Buffer
          {
             auto& resampleParameters = getResampleParameters();
             auto context = mods::utils::OpenCLManager::getContext();
             
             _filteredTaps = std::vector<double>(resampleParameters.getFilteredTaps().begin(),
                                                 resampleParameters.getFilteredTaps().end());
             
             return cl::Buffer(context, _filteredTaps.begin(), _filteredTaps.end(), true, true);
          }
        
        template<typename PARAMETERS, typename T>
          void OpenCLResampleConverter<PARAMETERS, T>::fillBuffers(const cl::Context& context, const cl::CommandQueue& queue, cl::Buffer* sampleBuffer, cl::Buffer* zerosBuffer, cl::Buffer* repeatBuffer)
            {
               using SampleHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<double&(impl::SampleWithZeros&)>>;
               using ZeroHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<int&(impl::SampleWithZeros&)>>;
               auto& history = this->getHistory();
               
               SampleHistoryCollection sampleHistory(history, [](impl::SampleWithZeros& sampleWithZeros) -> double& {
                  return sampleWithZeros.getSampleReference();
               });
               ZeroHistoryCollection zeroHistory(history, [](impl::SampleWithZeros& sampleWithZeros) -> int& {
                  return sampleWithZeros.getNumberOfZerosReference();
               });
               
               if(sampleBuffer->get() && sampleBuffer->getInfo<CL_MEM_SIZE>() >= history.size() * sizeof(double))
                 {
                    cl::copy(queue, sampleHistory.begin(), sampleHistory.end(), *sampleBuffer);
                 }
               else
                 {
                    *sampleBuffer = cl::Buffer(context, sampleHistory.begin(), sampleHistory.end(), true /* read only */);
                 }
               if(zerosBuffer->get() && zerosBuffer->getInfo<CL_MEM_SIZE>() >= history.size() * sizeof(int))
                 {
                    cl::copy(queue, zeroHistory.begin(), zeroHistory.end(), *zerosBuffer);
                 }
               else
                 {
                    *zerosBuffer = cl::Buffer(context, zeroHistory.begin(), zeroHistory.end(), true /* read only */);
                 }
            }
        
        void OpenCLResampleConverter<AmigaResampleParameters, mods::utils::AmigaRLESample>::fillBuffers(const cl::Context& context, const cl::CommandQueue& queue, cl::Buffer* sampleBuffer, cl::Buffer* zerosBuffer, cl::Buffer* repeatBuffer)
          {
             using SampleHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<double&(impl::SampleWithZeros&)>>;
             using ZeroHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<int&(impl::SampleWithZeros&)>>;
             using RepeatHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<int&(impl::SampleWithZeros&)>>;
             auto& history = this->getHistory();
             
             SampleHistoryCollection sampleHistory(history, [](impl::SampleWithZeros& sampleWithZeros) -> double& {
                return sampleWithZeros.getSampleReference();
             });
             ZeroHistoryCollection zeroHistory(history, [](impl::SampleWithZeros& sampleWithZeros) -> int& {
                return sampleWithZeros.getNumberOfZerosReference();
             });
             RepeatHistoryCollection repeatHistory(history, [](impl::SampleWithZeros& sampleWithZeros) -> int& {
                return sampleWithZeros.getRepeatCountReference();
             });
             
             if(sampleBuffer->get() && sampleBuffer->getInfo<CL_MEM_SIZE>() >= history.size() * sizeof(double))
               {
                  cl::copy(queue, sampleHistory.begin(), sampleHistory.end(), *sampleBuffer);
               }
             else
               {
                  *sampleBuffer = cl::Buffer(context, sampleHistory.begin(), sampleHistory.end(), true /* read only */);
               }
             if(zerosBuffer->get() && zerosBuffer->getInfo<CL_MEM_SIZE>() >= history.size() * sizeof(int))
               {
                  cl::copy(queue, zeroHistory.begin(), zeroHistory.end(), *zerosBuffer);
               }
             else
               {
                  *zerosBuffer = cl::Buffer(context, zeroHistory.begin(), zeroHistory.end(), true /* read only */);
               }
             if(repeatBuffer->get() && repeatBuffer->getInfo<CL_MEM_SIZE>() >= history.size() * sizeof(int))
               {
                  cl::copy(queue, repeatHistory.begin(), repeatHistory.end(), *repeatBuffer);
               }
             else
               {
                  *repeatBuffer = cl::Buffer(context, repeatHistory.begin(), repeatHistory.end(), true /* read only */);
               }
          }
        
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>, double>;
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>, double>;
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>, double>;
	template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>, double>;
        template class OpenCLResampleConverter<DynamicResampleParameters, double>;
     } // namespace converters
} // namespace mods
#endif // WITH_OPENCL
