
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
            : ResampleConverter<PARAMETERS>(std::move(src), std::move(resampleParameters)),
          _context(mods::utils::OpenCLManager::getContext()),
          _queue(_context),
          _firFilterProgram(compileFirFilterProgram()),
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
                  
                  return firKernelSource;
               }
          } // namespace
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::compileFirFilterProgram() -> cl::Program
          {
             try
               {
                  auto& resampleParameters = ResampleConverter<PARAMETERS>::getResampleParameters();
                  
                  std::stringstream ss;
                  ss << "constant unsigned int interpolationFactor = " << resampleParameters.getInterpolationFactor() << ";" << std::endl;
                  ss << "constant unsigned int decimationFactor = " << resampleParameters.getDecimationFactor() << ";" << std::endl;
                  ss << "constant unsigned int numTaps = " << resampleParameters.getNumTaps() << ";" << std::endl;
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
        
        template<typename PARAMETERS>
          auto OpenCLResampleConverter<PARAMETERS>::buildTapsBuffer() -> cl::Buffer
          {
             auto& resampleParameters = ResampleConverter<PARAMETERS>::getResampleParameters();
             
             std::vector<double> taps(resampleParameters.getTaps().begin(),
                                      resampleParameters.getTaps().end());
             return cl::Buffer(_context, taps.begin(), taps.end(), true, true);
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
                    ResampleConverter<PARAMETERS>::addToHistory();
                 }
               
               using SampleHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<double&(impl::SampleWithZeros&)>>;
               using ZeroHistoryCollection = mods::utils::TransformedCollection<impl::History, std::function<int&(impl::SampleWithZeros&)>>;
               auto& history = ResampleConverter<PARAMETERS>::getHistory();
               
               SampleHistoryCollection sampleHistory(history, [](impl::SampleWithZeros& sampleWithZeros) -> double& {
                  return sampleWithZeros.sample();
               });
               ZeroHistoryCollection zeroHistory(history, [](impl::SampleWithZeros& sampleWithZeros) -> int& {
                  return sampleWithZeros.numberOfZeros();
               });
               
               cl::Buffer sampleBuffer(_context, sampleHistory.begin(), sampleHistory.end(), true);
               cl::Buffer zerosBuffer(_context, zeroHistory.begin(), zeroHistory.end(), true);
               cl::Buffer outputBuffer(_context, CL_MEM_WRITE_ONLY, nbElems * sizeof(double)); // NOLINT(hicpp-signed-bitwise)
               
               cl::NDRange global(nbElems);
               _firFilterKernel(cl::EnqueueArgs(_queue, global),
                                outputBuffer, zerosBuffer, sampleBuffer, _tapsBuffer);
               
               cl::copy(_queue, outputBuffer, outView.begin(), outView.end());
               
               for(size_t i=0; i<nbElems; ++i)
                 {
                    ResampleConverter<PARAMETERS>::removeFromHistory();
                 }
            }
        
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_22000, StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_8000,  StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_48000, StandardFrequency::_44100>>;
	template class OpenCLResampleConverter<StaticResampleParameters<StandardFrequency::_10000, StandardFrequency::_44100>>;
        template class OpenCLResampleConverter<DynamicResampleParameters>;
     } // namespace wav
} // namespace mods
#endif // WITH_OPENCL
