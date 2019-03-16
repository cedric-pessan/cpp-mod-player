#ifndef MODS_UTILS_BAND_HPP
#define MODS_UTILS_BAND_HPP

namespace mods
{
   namespace utils
     {
        class Band
          {
           public:
             Band(double startHz, double stopHz, double gain, double ripple, double sampleFrequency);
             
             Band() = delete;
             Band(const Band&) = default;
             Band(Band&&) = delete;
             Band& operator=(const Band&) = delete;
             Band& operator=(Band&&) = delete;
             ~Band() = default;
             
             double getStop() const;
             double getStart() const;
             double getGain() const;
             double getWeight() const;
             double getUpper() const;
             double getLower() const;
             
           private:
             double _start;
             double _stop;
             double _gain;
             double _weight;
             double _upper;
             double _lower;
          };
     } // namespace utils
} // namespace mods

#endif // MODS_UTILS_BAND_HPP
