
#include "mods/mod/Effect.hpp"

namespace mods
{
   namespace mod
     {
        class SlideUp : public Effect
          {
           public:
             SlideUp() = default;
             SlideUp(const SlideUp&) = delete;
             SlideUp(SlideUp&&) = delete;
             auto operator=(const SlideUp&) -> SlideUp& = delete;
             auto operator=(SlideUp&&) -> SlideUp& = delete;
             ~SlideUp() override = default;
             
             void init(int delta);
             
             auto getModifiedPeriod(u16 period) -> u16 override;
             auto getModifiedVolume(u16 volume) const -> u16 override;
             void tick() override;
             
           private:
             bool _newTick = false;
             int _delta = 0;
          };
     } // namespace mod
} // namespace mods
