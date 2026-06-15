#include "sprites.h"

static Color paletteColor(char ch) {
    switch (ch) {
        // Player ship
        case 'E': return {255, 140,  40, 255};  // engine orange
        case 'b': return {100, 150, 200, 255};  // body dark steel
        case 'B': return {140, 180, 220, 255};  // body mid steel
        case 'C': return { 60, 210, 255, 255};  // cockpit cyan
        case 'N': return {210, 235, 255, 255};  // nose bright
        case 'n': return {170, 200, 240, 255};  // nose dim
        case 'f': return { 60,  80, 120, 255};  // fin dark
        // Lander
        case 'd': return {200, 100, 100, 255};  // dome medium
        case 'D': return {255, 160, 160, 255};  // dome bright
        case 'r': return {200,  60,  60, 255};  // body red
        case 'R': return {255, 100, 100, 255};  // body bright red
        case 'l': return {255, 240, 180, 255};  // underlight yellow
        // Mutant
        case 'm': return {130,  20, 180, 255};  // purple dark
        case 'M': return {180,  60, 240, 255};  // purple mid
        case 'S': return {210, 140, 255, 255};  // body lavender
        case 'c': return {255, 255, 180, 255};  // core glow
        // Baiter
        case 'g': return {180, 150,  20, 255};  // gold dark
        case 'G': return {255, 220,  40, 255};  // gold bright
        // Humanoid
        case 'h': return {240, 185, 120, 255};  // skin
        case 'J': return { 80, 200,  80, 255};  // suit bright green
        case 'j': return { 50, 140,  50, 255};  // suit dark green
        default:  return {  0,   0,   0,   0};  // transparent
    }
}

static Texture2D buildSprite(const char* const* rows, int W, int H) {
    Image img = GenImageColor(W, H, BLANK);
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            Color c = paletteColor(rows[y][x]);
            if (c.a > 0)
                ImageDrawPixel(&img, x, y, c);
        }
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

// ── Player ship: 20w × 12h, facing RIGHT  (flip src width for left) ──────────
//
//  Engine (E) at left, rear fin (f) near engine, cockpit (C) center,
//  tapered nose (N/n) at right.  Fins protrude 2px above/below hull.
//
static const char* const PLAYER_PIX[12] = {
    ".....f..............",  // row  0  dorsal fin
    ".....f..............",  // row  1
    "E....f..........NN..",  // row  2  engine begins; nose tip
    "EE..bbbbbbbbbbbNNN..",  // row  3  full engine; body; wide nose
    "EEBBBBBBBBBBBbbNNN..",  // row  4  bright body; rim shade
    "EEBBBBBBCCBBBbbNNN..",  // row  5  cockpit window (2 px)
    "EEBBBBBBCCBBBbbNNN..",  // row  6  cockpit
    "EEBBBBBBBBBBBbbNNN..",  // row  7
    "EE..bbbbbbbbbbbNNN..",  // row  8
    "E....f..........NN..",  // row  9  ventral fin
    ".....f..............",  // row 10
    ".....f..............",  // row 11
};

// ── Lander: 16w × 14h  ────────────────────────────────────────────────────────
//
//  Pink dome on top, wide red saucer body, warm underlight, two angled legs.
//
static const char* const LANDER_PIX[14] = {
    "......dd........",  // row  0  dome tip
    "....ddDDdd......",  // row  1  dome
    "....ddDDDdd.....",  // row  2  dome base
    "...rrrrrrrrrr...",  // row  3  upper body
    "..rrrRRRRRRrrr..",  // row  4
    ".rrrrRRRRRRrrrr.",  // row  5  widest
    ".rrrrRllllRrrrr.",  // row  6  underlight strip
    "..rrrrrrrrrrrr..",  // row  7
    "....rrrrrrrr....",  // row  8
    "....r.......r...",  // row  9  left leg
    "...r.........r..",  // row 10  legs spread
    "...r.........r..",  // row 11
    "................",  // row 12
    "................",  // row 13
};

// ── Mutant: 14w × 14h  ────────────────────────────────────────────────────────
//
//  Perfect diamond, three-shade purple with yellow core glow at centre.
//
static const char* const MUTANT_PIX[14] = {
    "......m.......",  // row  0
    ".....mMm......",  // row  1
    "....mMSMm.....",  // row  2
    "...mMSSSMm....",  // row  3
    "..mMSSSSSMm...",  // row  4
    ".mMSSSSSSSMm..",  // row  5
    "mMSSSSCSSSSMm.",  // row  6  core glow
    "mMSSSSCSSSSMm.",  // row  7  core glow
    ".mMSSSSSSSMm..",  // row  8
    "..mMSSSSSMm...",  // row  9
    "...mMSSSMm....",  // row 10
    "....mMSMm.....",  // row 11
    ".....mMm......",  // row 12
    "......m.......",  // row 13
};

// ── Baiter: 16w × 8h  ─────────────────────────────────────────────────────────
//
//  Low-profile gold lens; scanner dot at centre.  Rotating scan line is
//  drawn on top of the sprite in Enemy::draw() using the existing wobble field.
//
static const char* const BAITER_PIX[8] = {
    "................",  // row 0
    "....gGGGGGGGg...",  // row 1
    "..ggGGGGGGGGgg..",  // row 2
    "gGGGGGGGGGGGGGGg",  // row 3
    "gGGGGGGrGGGGGGGg",  // row 4  scanner centre dot
    "..ggGGGGGGGGgg..",  // row 5
    "....gGGGGGGGg...",  // row 6
    "................",  // row 7
};

// ── Humanoid: 8w × 14h  ───────────────────────────────────────────────────────
//
//  Tiny pixel astronaut: rounded head, wide shoulders, narrow waist, two legs.
//
static const char* const HUMANOID_PIX[14] = {
    "...h....",  // row  0  head
    "..hhh...",  // row  1
    "...h....",  // row  2  neck
    ".JJJJJ..",  // row  3  shoulders
    ".JJJJJ..",  // row  4  torso
    "..JJJ...",  // row  5
    "..JJJ...",  // row  6
    "..JJJ...",  // row  7  waist
    ".j...j..",  // row  8  upper legs
    ".j...j..",  // row  9
    ".j...j..",  // row 10
    ".j...j..",  // row 11
    ".j...j..",  // row 12
    "........",  // row 13
};

void Sprites::load() {
    player   = buildSprite(PLAYER_PIX,   20, 12);
    lander   = buildSprite(LANDER_PIX,   16, 14);
    mutant   = buildSprite(MUTANT_PIX,   14, 14);
    baiter   = buildSprite(BAITER_PIX,   16,  8);
    humanoid = buildSprite(HUMANOID_PIX,  8, 14);
}

void Sprites::unload() {
    UnloadTexture(player);
    UnloadTexture(lander);
    UnloadTexture(mutant);
    UnloadTexture(baiter);
    UnloadTexture(humanoid);
    player = lander = mutant = baiter = humanoid = {};
}
