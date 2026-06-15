#pragma once
#include "raylib.h"

struct AudioFX {
    Sound fire, explode, abduct, rescue, bomb, hyper, die, extraLife;

    void init();
    void shutdown();

    void playFire()      const { if (fire.stream.buffer)      PlaySound(fire); }
    void playExplode()   const { if (explode.stream.buffer)   PlaySound(explode); }
    void playAbduct()    const { if (abduct.stream.buffer)    PlaySound(abduct); }
    void playRescue()    const { if (rescue.stream.buffer)    PlaySound(rescue); }
    void playBomb()      const { if (bomb.stream.buffer)      PlaySound(bomb); }
    void playHyper()     const { if (hyper.stream.buffer)     PlaySound(hyper); }
    void playDie()       const { if (die.stream.buffer)       PlaySound(die); }
    void playExtraLife() const { if (extraLife.stream.buffer) PlaySound(extraLife); }
};
