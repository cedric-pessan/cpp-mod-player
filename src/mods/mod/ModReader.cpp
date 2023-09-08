
#include "mods/converters/FromDoubleConverter.hpp"
#include "mods/converters/MultiplexerConverter.hpp"
#include "mods/converters/OpenCLConverterTypes.hpp"
#include "mods/converters/ResampleParameters.hpp"
#include "mods/converters/SoftwareResampleConverter.hpp"
#include "mods/mod/Instrument.hpp"
#include "mods/mod/ModChannelConverter.hpp"
#include "mods/mod/ModReader.hpp"
#include "mods/utils/FileUtils.hpp"

#include <map>
#include <sstream>

namespace mods
{
   namespace mod
     {
        namespace
          {
             constexpr static int oldModNumberOfInstruments = 15;
             constexpr static int newModNumberOfInstruments = 31;
             
             auto getTagsToChans() -> const std::map<std::string, size_t>&
               {
                  static const std::map<std::string, size_t> tagsToChans
                    {
                       { "M.K." , 4 }
                    };
                  return tagsToChans;
               }
          } // namespace
        
        ModReader::ModReader(const std::string& fileName)
          : _fileBuffer(mods::utils::FileUtils::mapFile(fileName)),
          _notParsedBuffer(_fileBuffer.slice<u8>(0, _fileBuffer.size())),
          _songTitle(parseSongTitle()),
          _numberOfInstruments(detectNumberOfInstruments()),
          _instruments(parseInstruments()),
          _numberOfPatterns(parseNumberOfPatterns()),
          _endJumpPosition(parseEndJumpPosition()),
          _patternsOrderList(parsePatternsTable()),
          _nbChannels(getNumberOfChannelsFromFormatTag()),
          _patterns(parsePatternsBuffer()),
          _sampleBuffers(parseSampleBuffers()),
          _patternListReader(std::make_unique<PatternListReader>(_numberOfPatterns, _patternsOrderList, _nbChannels, _patterns, _sampleBuffers)),
          _modConverter(buildModConverter(_patternListReader))
            {
            }
        
        auto ModReader::parseSongTitle() -> std::string
          {
             using SongArray = mods::utils::PackedArray<char, _songFieldLength>;
             
             checkInit(_fileBuffer.size() > _songFieldLength, "File too small to contain song title");
             auto buf = _fileBuffer.slice<SongArray>(0, 1);
             auto& array = buf[0];
             size_t length = 0;
             for(length = 0; length<_songFieldLength; ++length) 
               {
                  if(array[length] == '\0')
                    {
                       break;
                    }
               }
             
             _notParsedBuffer = _notParsedBuffer.slice<u8>(_songFieldLength, _notParsedBuffer.size() - _songFieldLength);
             return std::string(array.data(), length);
          }
        
        auto ModReader::detectNumberOfInstruments() const -> u32
          {
             constexpr static int minValidAscii = 0x20;
             constexpr static int maxValidAscii = 0x7E;
             
             checkInit(_notParsedBuffer.size() >= oldModNumberOfInstruments * sizeof(Instrument),
                       "File is too small for 16 instruments");
             
             if(_notParsedBuffer.size() < newModNumberOfInstruments * sizeof(Instrument))
               {
                  return oldModNumberOfInstruments;
               }
             
             auto tmp = _notParsedBuffer.slice<Instrument>(0, newModNumberOfInstruments);
             
             for(auto& instrument : tmp) 
               {
                  bool ascii = true;
                  for(auto& c : instrument.getSampleName())
                    {
                       if(c == '\0')
                         {
                            break;
                         }
                       if(c < minValidAscii && c > maxValidAscii)
                         {
                            ascii = false;
                            break;
                         }
                    }
                  if(!ascii)
                    {
                       return oldModNumberOfInstruments;
                    }
               }
             
             return newModNumberOfInstruments;
          }
        
        auto ModReader::parseInstruments() -> mods::utils::RBuffer<Instrument>
          {
             auto instruments = _fileBuffer.slice<Instrument>(_songFieldLength, _numberOfInstruments);
             _notParsedBuffer = _notParsedBuffer.slice<u8>(_numberOfInstruments * sizeof(Instrument),
                                                           _notParsedBuffer.size() - _numberOfInstruments * sizeof(Instrument));
             return instruments;
          }
        
        auto ModReader::parseNumberOfPatterns() -> size_t
          {
             checkInit(_notParsedBuffer.size() > 0, "File is too small to contain number of patterns");
             
             u8 numberOfPatterns = _notParsedBuffer[0];
             _notParsedBuffer = _notParsedBuffer.slice<u8>(1, _notParsedBuffer.size() - 1);
             
             checkInit(numberOfPatterns > 0, "A module should contain at least one pattern");
             
             return numberOfPatterns;
          }
        
        auto ModReader::parseEndJumpPosition() -> size_t
          {
             checkInit(_notParsedBuffer.size() > 0, "File is too small to contain end jump position");
             
             u8 endJumpPosition = _notParsedBuffer[0];
             _notParsedBuffer = _notParsedBuffer.slice<u8>(1, _notParsedBuffer.size() - 1);
             return endJumpPosition;
          }
        
        auto ModReader::parsePatternsTable() -> mods::utils::RBuffer<u8>
          {
             constexpr static size_t patternTableLength = 128;
             
             checkInit(_notParsedBuffer.size() >= patternTableLength, "File is too small to contain patterns table");
             
             auto table = _notParsedBuffer.slice<u8>(0, patternTableLength);
             _notParsedBuffer = _notParsedBuffer.slice<u8>(patternTableLength, _notParsedBuffer.size() - patternTableLength);
             return table;
          }
        
        auto ModReader::getNumberOfChannelsFromFormatTag() -> size_t
          {
             constexpr static size_t tagSize = 4;
             
             if(_instruments.size() == oldModNumberOfInstruments)
               {
                  std::cout << "TODO: ModReader::getNumberOfChannelsFromFormatTag() const, 15 instruments mod" << std::endl;
               }
             
             checkInit(_notParsedBuffer.size() >= tagSize, "File is too small to contain format tag");
             
             std::string tag(_notParsedBuffer.begin(), _notParsedBuffer.begin() + tagSize);
             _notParsedBuffer = _notParsedBuffer.slice<u8>(tagSize, _notParsedBuffer.size() - tagSize);
             
             auto& tagsToChans = getTagsToChans();
             auto it = tagsToChans.find(tag);
             if(it != tagsToChans.end())
               {
                  return it->second;
               }
             
             std::cout << "Unknown format tag:" << tag << std::endl;
             return 0;
          }
        
        auto ModReader::parsePatternsBuffer() -> mods::utils::RBuffer<Note>
          {
             size_t maxPatternIndex = 0;
             for(auto p : _patternsOrderList)
               {
                  if(p > maxPatternIndex)
                    {
                       maxPatternIndex = p;
                    }
               }
             
             auto numberOfNotes = (maxPatternIndex + 1) * _nbChannels * PatternReader::getNumberOfLines();
             auto patternsBufferLength = numberOfNotes * sizeof(Note);
             
             checkInit(_notParsedBuffer.size() >= patternsBufferLength, "File is too small to contain the patterns table");
             
             auto patterns = _notParsedBuffer.slice<Note>(0, numberOfNotes);
             _notParsedBuffer = _notParsedBuffer.slice<u8>(patternsBufferLength, _notParsedBuffer.size() - patternsBufferLength);
             return patterns;
          }
        
        auto ModReader::parseSampleBuffers() -> std::vector<mods::utils::RBuffer<u8>>
          {
             std::vector<mods::utils::RBuffer<u8>> bufs;
             
             for(auto& instrument : _instruments)
               {
                  size_t length = instrument.getSampleLength();
                  
                  checkInit(_notParsedBuffer.size() >= length, "File is too small to contain all instruments");
                  
                  bufs.push_back(_notParsedBuffer.slice<u8>(0, length));
                  _notParsedBuffer = _notParsedBuffer.slice<u8>(length, _notParsedBuffer.size() - length);
               }
             
             return bufs;
          }
        
        auto ModReader::isFinished() const -> bool
          {
             return _modConverter->isFinished();
          }
        
        void ModReader::read(mods::utils::RWBuffer<s16>* buf)
          {
             _modConverter->read(buf);
          }
        
        auto ModReader::getInfo() const -> std::string
          {
             std::stringstream ss;
             ss << getSongTitle() << std::endl;
             for(auto& instrument : _instruments)
               {
                  ss << instrument.getSampleName() << std::endl;
               }
             return ss.str();
          }
        
        auto ModReader::getProgressInfo() const -> std::string
          {
             return _patternListReader->getProgressInfo();
          }
        
        auto ModReader::getSongTitle() const -> std::string
          {
             return _songTitle;
          }
        
        auto ModReader::buildModConverter(std::shared_ptr<PatternListReader> patternListReader) -> mods::converters::Converter<s16>::ptr
          {
             auto left = std::make_unique<ModChannelConverter>(patternListReader, ChannelId::LEFT);
             auto right = std::make_unique<ModChannelConverter>(patternListReader, ChannelId::RIGHT);
             
             return buildResamplerStage(std::move(left), std::move(right));
          }
        
        auto ModReader::buildResamplerStage(mods::converters::Converter<mods::utils::AmigaRLESample>::ptr left,
                                            mods::converters::Converter<mods::utils::AmigaRLESample>::ptr right) -> mods::converters::Converter<s16>::ptr
          {
             auto resampledLeft = buildResampler(std::move(left));
             auto resampledRight = buildResampler(std::move(right));
             
             return buildFromDoubleStage(std::move(resampledLeft),
                                         std::move(resampledRight));
          }
        
        auto ModReader::buildResampler(mods::converters::Converter<mods::utils::AmigaRLESample>::ptr channel) -> mods::converters::Converter<double>::ptr
          {
             using mods::converters::AmigaResampleParameters;
             using mods::converters::OpenCLConverterTypes;
             using mods::converters::SoftwareResampleConverter;
             using mods::utils::AmigaRLESample;
             
             using ParamType = AmigaResampleParameters;
             ParamType params;
             if(mods::utils::OpenCLManager::isEnabled())
               {
                  using ResampleConverterImpl = OpenCLConverterTypes<ParamType, AmigaRLESample>::ResampleConverterImpl;
                  return std::make_unique<ResampleConverterImpl>(std::move(channel), params);
               }
             else
               {
                  return std::make_unique<SoftwareResampleConverter<ParamType, AmigaRLESample>>(std::move(channel), params);
               }
          }
        
        auto ModReader::buildFromDoubleStage(mods::converters::Converter<double>::ptr left,
                                             mods::converters::Converter<double>::ptr right) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::FromDoubleConverter;
             
             auto intLeft = std::make_unique<FromDoubleConverter<s16>>(std::move(left));
             auto intRight = std::make_unique<FromDoubleConverter<s16>>(std::move(right));
             
             return buildMuxStage(std::move(intLeft), std::move(intRight));
          }
        
        auto ModReader::buildMuxStage(mods::converters::Converter<s16>::ptr left,
                                      mods::converters::Converter<s16>::ptr right) -> mods::converters::Converter<s16>::ptr
          {
             using mods::converters::MultiplexerConverter;
             
             return std::make_unique<MultiplexerConverter<s16>>(std::move(left), std::move(right));
          }
     } // namespace mod
} // namespace mods
