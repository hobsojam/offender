#pragma once
#include "raylib.h"

struct AudioFX {
    Sound fire, explode, abduct, rescue, bomb, hyper, die, extraLife;
    Sound bgm;

    void init();
    void shutdown();

    // Call once per frame while gameplay is active
    void updateBGM() { if (bgm.stream.buffer && !IsSoundPlaying(bgm)) PlaySound(bgm); }
    void startBGM()  { if (bgm.stream.buffer) { StopSound(bgm); PlaySound(bgm); } }
    void stopBGM()   { StopSound(bgm); }

    void playFire()      const { if (fire.stream.buffer)      PlaySound(fire); }
    void playExplode()   const { if (explode.stream.buffer)   PlaySound(explode); }
    void playAbduct()    const { if (abduct.stream.buffer)    PlaySound(abduct); }
    void playRescue()    const { if (rescue.stream.buffer)    PlaySound(rescue); }
    void playBomb()      const { if (bomb.stream.buffer)      PlaySound(bomb); }
    void playHyper()     const { if (hyper.stream.buffer)     PlaySound(hyper); }
    void playDie()       const { if (die.stream.buffer)       PlaySound(die); }
    void playExtraLife() const { if (extraLife.stream.buffer) PlaySound(extraLife); }
};
