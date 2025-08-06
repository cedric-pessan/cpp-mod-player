#ifndef MODS_MOD_SLIDETONOTEANDVOLUMESLIDE_HPP
#define MODS_MOD_SLIDETONOTEANDVOLUMESLIDE_HPP

#include "mods/mod/Effect.hpp"
#include "mods/mod/VolumeSlide.hpp"

namespace mods
{
   namespace mod
     {
        class SlideToNote;
        
        class SlideToNoteAndVolumeSlide : public Effect
          {
           public:
             using Volume = VolumeSlide::Volume;
             using Delta = VolumeSlide::Delta;
             
             SlideToNoteAndVolumeSlide(SlideToNote* slideToNote, VolumeSlide* volumeSlide);
             
             SlideToNoteAndVolumeSlide() = delete;
             SlideToNoteAndVolumeSlide(const SlideToNoteAndVolumeSlide&) = delete;
             SlideToNoteAndVolumeSlide(SlideToNoteAndVolumeSlide&&) = delete;
             auto operator=(const SlideToNoteAndVolumeSlide&) -> SlideToNoteAndVolumeSlide& = delete;
             auto operator=(SlideToNoteAndVolumeSlide&&) -> SlideToNoteAndVolumeSlide& = delete;
             ~SlideToNoteAndVolumeSlide() override = default;
             
             void init(Volume currentVolume, Delta delta);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume ) const -> u16 override;
             auto retriggerSample() const -> bool override;
             void tick() override;
             
           private:
             SlideToNote* _slideToNote;
             VolumeSlide* _volumeSlide;
          };
     } // namespace mod
} // namespace mods

#endif // MODS_MOD_SLIDETONOTEANDVOLUMESLIDE_HPP