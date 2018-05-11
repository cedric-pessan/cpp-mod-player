#ifndef _SOUNDPLAYER_HPP_
#define _SOUNDPLAYER_HPP_

class SoundPlayer
{
 public:
   SoundPlayer();
   ~SoundPlayer();
   
   void play();
   
 private:
   SoundPlayer(const SoundPlayer&);
   SoundPlayer& operator=(const SoundPlayer&);
};

#endif // _SOUNDPLAYER_HPP_
