/*
 * @(#)game.c
 *
 * Copyright 1999-2000, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 *
 * This file was generated as part of the "phire!" program developed for 
 * the Palm Computing Platform designed by Palm: 
 *
 *   http://www.palm.com/ 
 *
 * The contents of this file is confidential and proprietrary in nature 
 * ("Confidential Information"). Redistribution or modification without 
 * prior consent of the original author is prohibited. 
 */

#include "palm.h"

// interface

static void GameAdjustLevel(PreferencesType *)         __GAME__;
static void GameIncrementScore(PreferencesType *)      __GAME__;
static void GameMovePlayer(PreferencesType *)          __GAME__;
static void GameMoveJumpers(PreferencesType *)         __GAME__;
static void GameRemoveJumper(PreferencesType *, Int16) __GAME__;

// global variable structure
typedef struct
{
  WinHandle winDigits;                      // scoring digits bitmaps
  WinHandle winMisses;                      // the lives notification bitmaps

  WinHandle winSmokes;                      // the smoke bitmaps
  Boolean   smokeChanged;                   // do we need to repaint the smoke?
  Boolean   smokeOnScreen;                  // is smoke bitmap on screen?
  UInt16    smokeOldPosition;               // the *old* position of the smoke 

  WinHandle winTrampolines;                 // the trampoline bitmaps
  Boolean   trampolineChanged;              // do we need to repaint trampoline?
  Boolean   trampolineOnScreen;             // is trampoline bitmap on screen?
  UInt16    trampolineOldPosition;          // the *old* position of trampoline 

  WinHandle winJumpers;                      // the jumper bitmaps
  Boolean   jumperChanged[MAX_JUMP];         // do we need to repaint jumper
  Boolean   jumperOnScreen[MAX_JUMP];        // is jumper bitmap on screen?
  UInt16    jumperOnScreenPosition[MAX_JUMP];// the *old* position of jumper 

  WinHandle winJumperDeaths;                // the jumper death bitmaps

  UInt8     gameType;                       // the type of game active
  Boolean   playerDied;                     // has the player died?
  UInt8     moveDelayCount;                 // the delay between moves
  UInt8     moveLast;                       // the last move performed
  UInt8     moveNext;                       // the next desired move

  struct {

    Boolean    gamePadPresent;              // is the gamepad driver present
    UInt16     gamePadLibRef;               // library reference for gamepad

  } hardware;

} GameGlobals;

/**
 * Initialize the Game.
 *
 * @return true if game is initialized, false otherwise
 */  
Boolean   
GameInitialize()
{
  GameGlobals *gbls;
  Err         err;
  Boolean     result;

  // create the globals object, and register it
  gbls = (GameGlobals *)MemPtrNew(sizeof(GameGlobals));
  MemSet(gbls, sizeof(GameGlobals), 0);
  FtrSet(appCreator, ftrGameGlobals, (UInt32)gbls);

  // load the gamepad drvier if available
  {
    Err err;

    // attempt to load the library
    err = SysLibFind(GPD_LIB_NAME,&gbls->hardware.gamePadLibRef);
    if (err == sysErrLibNotFound)
      err = SysLibLoad('libr',GPD_LIB_CREATOR,&gbls->hardware.gamePadLibRef);

    // lets determine if it is available
    gbls->hardware.gamePadPresent = (err == errNone);

    // open the library if available
    if (gbls->hardware.gamePadPresent)
      GPDOpen(gbls->hardware.gamePadLibRef);
  }

  // initialize our "bitmap" windows
  err = errNone;
  {
    Int16 i;
    Err   e;

    gbls->winDigits = 
      WinCreateOffscreenWindow(70, 12, screenFormat, &e); err |= e;

    gbls->winMisses = 
      WinCreateOffscreenWindow(90, 20, screenFormat, &e); err |= e;

    gbls->winSmokes = 
      WinCreateOffscreenWindow(136, 21, screenFormat, &e); err |= e;
    gbls->smokeChanged     = true;
    gbls->smokeOnScreen    = false;
    gbls->smokeOldPosition = 0;

    gbls->winTrampolines = 
      WinCreateOffscreenWindow(105, 20, screenFormat, &e); err |= e;
    gbls->trampolineChanged     = true;
    gbls->trampolineOnScreen    = false;
    gbls->trampolineOldPosition = 0;

    gbls->winJumpers = 
      WinCreateOffscreenWindow(98, 56, screenFormat, &e); err |= e;
    for (i=0; i<MAX_JUMP; i++) {
      gbls->jumperChanged[i]          = true;
      gbls->jumperOnScreen[i]         = false;
      gbls->jumperOnScreenPosition[i] = 0;
    }

    gbls->winJumperDeaths = 
      WinCreateOffscreenWindow(63, 10, screenFormat, &e); err |= e;
  }

  // no problems creating back buffers? load images.
  if (err == errNone) {

    WinHandle currWindow;
    MemHandle bitmapHandle;

    currWindow = WinGetDrawWindow();

    // digits
    WinSetDrawWindow(gbls->winDigits);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapDigits);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // misses
    WinSetDrawWindow(gbls->winMisses);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapMisses);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // smoke
    WinSetDrawWindow(gbls->winSmokes);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapSmokes);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // trampoline
    WinSetDrawWindow(gbls->winTrampolines);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapTrampolines);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // jumpers
    WinSetDrawWindow(gbls->winJumpers);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapJumpers);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // jumper deaths
    WinSetDrawWindow(gbls->winJumperDeaths);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapJumperDeath);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    WinSetDrawWindow(currWindow);
  }

  result = (err == errNone);

  return result;
}

/**
 * Reset the Game.
 * 
 * @param prefs the global preference data.
 * @param gameType the type of game to configure for.
 */  
void   
GameReset(PreferencesType *prefs, Int8 gameType)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // turn off all the "bitmaps"
  FrmDrawForm(FrmGetActiveForm());

  // turn on all the "bitmaps"
  {
    RectangleType rect    = { {   0,   0 }, {   0,   0 } };
    RectangleType scrRect = { {   0,   0 }, {   0,   0 } };
    UInt16        i;

    // 
    // draw the score
    //

    for (i=0; i<4; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 7;
      scrRect.extent.y  = 12;
      rect.topLeft.x    = 8 * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the digit!
      WinCopyRectangle(gbls->winDigits, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
    }

    // 
    // draw the misses
    //

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    scrRect.extent.x  = 30;
    scrRect.extent.y  = 20;
    rect.topLeft.x    = 2 * scrRect.extent.x;
    rect.topLeft.y    = 0;
    rect.extent.x     = scrRect.extent.x;
    rect.extent.y     = scrRect.extent.y;

    // draw the miss bitmap!
    WinCopyRectangle(gbls->winMisses, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);

    //
    // draw the smoke
    //

    for (i=0; i<4; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteSmoke, 0, 
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 34;
      scrRect.extent.y  = 21;
      rect.topLeft.x    = i * scrRect.extent.x; 
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the smoke bitmap!
      WinCopyRectangle(gbls->winSmokes, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    //
    // draw the jumpers
    //

    for (i=0; i<2; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteJumper, i,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 14;
      scrRect.extent.y  = 14;
      rect.topLeft.x    = i * scrRect.extent.x;
      rect.topLeft.y    = 0 * scrRect.extent.y;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the jumper starter bitmap!
      WinCopyRectangle(gbls->winJumpers, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    for (i=7; i<28; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteJumper, i,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 14;
      scrRect.extent.y  = 14;
      rect.topLeft.x    = (i % 7) * scrRect.extent.x;
      rect.topLeft.y    = (i / 7) * scrRect.extent.y;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the jumper bitmap!
      WinCopyRectangle(gbls->winJumpers, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }
    
    //
    // draw the jumper death
    //

    for (i=0; i<3; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteJumperDeath, i,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 21;
      scrRect.extent.y  = 10;
      rect.topLeft.x    = i * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the jumper death bitmap!
      WinCopyRectangle(gbls->winJumperDeaths, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }

    //
    // draw the trampoline
    //

    for (i=0; i<3; i++) {

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteTrampoline, i, 
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 35;
      scrRect.extent.y  = 20;
      rect.topLeft.x    = i * scrRect.extent.x; 
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the trampoline bitmap
      WinCopyRectangle(gbls->winTrampolines, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
    }
  }

  // wait a good two seconds :))
  SysTaskDelay(2 * SysTicksPerSecond());

  // turn off all the "bitmaps"
  FrmDrawForm(FrmGetActiveForm());

  // reset the preferences
  GameResetPreferences(prefs, gameType);
}

/**
 * Reset the Game preferences.
 * 
 * @param prefs the global preference data.
 * @param gameType the type of game to configure for.
 */  
void   
GameResetPreferences(PreferencesType *prefs, Int8 gameType)
{
  GameGlobals *gbls;
  Int16       i;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // now we are playing
  prefs->game.gamePlaying                   = true;
  prefs->game.gamePaused                    = false;
  prefs->game.gameWait                      = true;
  prefs->game.gameAnimationCount            = 0;

  // reset score and lives
  prefs->game.gameScore                     = 0;
  prefs->game.gameLives                     = 3;

  // reset phire specific things
  prefs->game.phire.gameType                = gameType;
  prefs->game.phire.gameLevel               = 1;
  prefs->game.phire.bonusAvailable          = true;
  prefs->game.phire.bonusScoring            = false;

  prefs->game.phire.smokeWait               = 0;
  prefs->game.phire.smokePosition           = 0;

  prefs->game.phire.trampolinePosition      = 0;
  prefs->game.phire.trampolineNewPosition   = 0;

  prefs->game.phire.jumperCount             = 0;
  MemSet(&prefs->game.phire.jumperPosition[0], sizeof(UInt16) * MAX_JUMP, 0);
  MemSet(&prefs->game.phire.jumperWait[0],     sizeof(UInt16) * MAX_JUMP, 0);
  prefs->game.phire.jumperDeathPosition = 0;

  // reset the "backup" and "onscreen" flags
  gbls->smokeChanged                        = true;
  gbls->trampolineChanged                   = true;
  for (i=0; i<MAX_JUMP; i++) {
    gbls->jumperChanged[i]                  = true;
    gbls->jumperOnScreen[i]                 = false;
  }

  gbls->gameType                            = gameType;
  gbls->playerDied                          = false;
  gbls->moveDelayCount                      = 0;
  gbls->moveLast                            = moveNone;
  gbls->moveNext                            = moveNone;
}

/**
 * Process key input from the user.
 * 
 * @param prefs the global preference data.
 * @param keyStatus the current key state.
 */  
void   
GameProcessKeyInput(PreferencesType *prefs, UInt32 keyStatus)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  keyStatus &= (prefs->config.ctlKeyLeft  |
                prefs->config.ctlKeyRight);

  // additional checks here
  if (gbls->hardware.gamePadPresent) {
    
    UInt8 gamePadKeyStatus;
    Err   err;

    // read the state of the gamepad
    err = GPDReadInstant(gbls->hardware.gamePadLibRef, &gamePadKeyStatus);
    if (err == errNone) {

      // process
      if (((gamePadKeyStatus & GAMEPAD_LEFT)      != 0) ||
          ((gamePadKeyStatus & GAMEPAD_LEFTFIRE)  != 0))
        keyStatus |= prefs->config.ctlKeyLeft;
      if (((gamePadKeyStatus & GAMEPAD_RIGHT)     != 0) ||
          ((gamePadKeyStatus & GAMEPAD_RIGHTFIRE) != 0))
        keyStatus |= prefs->config.ctlKeyRight;

      // special purpose :)
      if  ((gamePadKeyStatus & GAMEPAD_SELECT)    != 0) {

        // wait until they let it go :)
        do {
          GPDReadInstant(gbls->hardware.gamePadLibRef, &gamePadKeyStatus);
        } while ((gamePadKeyStatus & GAMEPAD_SELECT) != 0);

        keyStatus = 0;
        prefs->game.gamePaused = !prefs->game.gamePaused;
      }
      if  ((gamePadKeyStatus & GAMEPAD_START)     != 0) {

        // wait until they let it go :)
        do {
          GPDReadInstant(gbls->hardware.gamePadLibRef, &gamePadKeyStatus);
        } while ((gamePadKeyStatus & GAMEPAD_START) != 0);

        keyStatus = 0;
        GameReset(prefs, prefs->game.phire.gameType);
      }
    }
  }

  // did they press at least one of the game keys?
  if (keyStatus != 0) {

    // if they were waiting, we should reset the game animation count
    if (prefs->game.gameWait) { 
      prefs->game.gameAnimationCount = 0;
    }

    // great! they wanna play
    prefs->game.gamePaused = false;
    prefs->game.gameWait   = false;
  }

  // move left
  if (
      ((keyStatus &  prefs->config.ctlKeyLeft) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveLeft)
      )
     ) {

    // adjust the position if possible
    if (prefs->game.phire.trampolinePosition > 0) {
      prefs->game.phire.trampolineNewPosition = 
        prefs->game.phire.trampolinePosition - 1;
    }
  }

  // move right
  else
  if (
      ((keyStatus & prefs->config.ctlKeyRight) != 0) &&
      (
       (gbls->moveDelayCount == 0) || 
       (gbls->moveLast       != moveRight)
      )
     ) {

    // adjust the position if possible
    if (prefs->game.phire.trampolinePosition < 2) {
      prefs->game.phire.trampolineNewPosition = 
        prefs->game.phire.trampolinePosition + 1;
    }
  }
}
  
/**
 * Process stylus input from the user.
 * 
 * @param prefs the global preference data.
 * @param x the x co-ordinate of the stylus event.
 * @param y the y co-ordinate of the stylus event.
 */  
void   
GameProcessStylusInput(PreferencesType *prefs, Coord x, Coord y)
{
  RectangleType rect;
  Int16         i;

  // lets take a look at all the possible "positions"
  for (i=0; i<3; i++) {

    // get the bounding box of the position
    GameGetSpritePosition(spriteTrampoline, i,
                          &rect.topLeft.x, &rect.topLeft.y);
    rect.extent.x = 35;
    rect.extent.y = 20;

    // did they tap inside this rectangle?
    if (RctPtInRectangle(x, y, &rect)) {

      // ok, this is where we are going to go :)
      prefs->game.phire.trampolineNewPosition = i;

      // if they were waiting, we should reset the game animation count
      if (prefs->game.gameWait) {
        prefs->game.gameAnimationCount = 0;
        prefs->game.gameWait           = false;
      }

      // great! they wanna play
      prefs->game.gamePaused = false;
      break;                                        // stop looking
    }
  }
}

/**
 * Process the object movement in the game.
 * 
 * @param prefs the global preference data.
 */  
void   
GameMovement(PreferencesType *prefs)
{
  const CustomPatternType erase = { 0,0,0,0,0,0,0,0 };
  const RectangleType     rect  = {{   0,  16 }, { 160, 16 }};

  GameGlobals    *gbls;
  SndCommandType deathSnd = {sndCmdFreqDurationAmp,0, 512,50,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  //
  // the game is NOT paused.
  //

  if (!prefs->game.gamePaused) {

    // animate the smoke
    if (prefs->game.phire.smokeWait == 0) {
    
      prefs->game.phire.smokePosition =
        (prefs->game.phire.smokePosition + 1) % 4;
      prefs->game.phire.smokeWait     = 4;
  
      gbls->smokeChanged = true;
    }
    else {
      prefs->game.phire.smokeWait--;
    }

    // we must make sure the user is ready for playing 
    if (!prefs->game.gameWait) {

      // we cannot be dead yet :)
      gbls->playerDied = false;

      // are we in bonus mode?
      if ((prefs->game.phire.bonusScoring) &&
          (prefs->game.gameAnimationCount % GAME_FPS) < (GAME_FPS >> 1)) {

        Char   str[32];
        FontID currFont = FntGetFont();

        StrCopy(str, "    * BONUS PLAY *    ");
        FntSetFont(boldFont);
        WinDrawChars(str, StrLen(str), 
                     80 - (FntCharsWidth(str, StrLen(str)) >> 1), 19);
        FntSetFont(currFont);
      }
      else {

        // erase the status area
        WinSetPattern(&erase);
        WinFillRectangle(&rect, 0);
      }

      // player gets first move
      GameMovePlayer(prefs);
      GameMoveJumpers(prefs);

      // is it time to upgrade the game?
      if (prefs->game.gameAnimationCount >= 
           ((gbls->gameType == GAME_A) ? 0x17f : 0x100)) {

        prefs->game.gameAnimationCount = 0;
        prefs->game.phire.gameLevel++;

        // upgrading of difficulty?
        if (
            (gbls->gameType              == GAME_A) &&
            (prefs->game.phire.gameLevel > 12)
           ) {

          gbls->gameType               = GAME_B;
          prefs->game.phire.gameLevel -= 2;  // give em a break :)
        }
      } 

      // has the player died in this frame?
      if (gbls->playerDied) {

        UInt16        i, index;
	RectangleType rect    = { {   0,   0 }, {   0,   0 } };
	RectangleType scrRect = { {   0,   0 }, {   0,   0 } };

        // play death sound and flash the player
	for (i=0; i<4; i++) {

	  index = prefs->game.phire.jumperDeathPosition;

          // what is the rectangle we need to copy?
	  GameGetSpritePosition(spriteJumperDeath, index,
	                        &scrRect.topLeft.x, &scrRect.topLeft.y);
          scrRect.extent.x  = 21;
	  scrRect.extent.y  = 10;
	  rect.topLeft.x    = index * scrRect.extent.x;
	  rect.topLeft.y    = 0;
	  rect.extent.x     = scrRect.extent.x;
	  rect.extent.y     = scrRect.extent.y;

	  // invert the jumper death bitmap
	  WinCopyRectangle(gbls->winJumperDeaths, WinGetDrawWindow(),
	                   &rect, scrRect.topLeft.x, scrRect.topLeft.y, winInvert);

          // play the beep sound
	  DevicePlaySound(&deathSnd);
	  SysTaskDelay(50);
        }

        // lose a life :(
        prefs->game.gameLives--;

        // no more lives left: GAME OVER!
        if (prefs->game.gameLives == 0) {

          EventType event;

          // GAME OVER - return to main screen
          MemSet(&event, sizeof(EventType), 0);
          event.eType            = menuEvent;
          event.data.menu.itemID = gameMenuItemExit;
          EvtAddEventToQueue(&event);

          prefs->game.gamePlaying = false;
        }

        // continue game
        else {
          GameAdjustLevel(prefs);
          prefs->game.phire.bonusScoring = false;
          prefs->game.gameWait           = true;
        }
      }
    }

    // we have to display "GET READY!"
    else {

      // flash on:
      if ((prefs->game.gameAnimationCount % GAME_FPS) < (GAME_FPS >> 1)) {

        Char   str[32];
        FontID currFont = FntGetFont();

        StrCopy(str, "    * GET READY *    ");
        FntSetFont(boldFont);
        WinDrawChars(str, StrLen(str), 
                     80 - (FntCharsWidth(str, StrLen(str)) >> 1), 19);
        FntSetFont(currFont);
      }

      // flash off:
      else {

        // erase the status area
        WinSetPattern(&erase);
        WinFillRectangle(&rect, 0);
      }
    }

    // update the animation counter
    prefs->game.gameAnimationCount++;
  }

  //
  // the game is paused.
  //

  else {

    Char   str[32];
    FontID currFont = FntGetFont();

    StrCopy(str, "    *  PAUSED  *    ");
    FntSetFont(boldFont);
    WinDrawChars(str, StrLen(str), 
                 80 - (FntCharsWidth(str, StrLen(str)) >> 1), 19);
    FntSetFont(currFont);
  }
}

/**
 * Draw the game on the screen.
 * 
 * @param prefs the global preference data.
 */
void   
GameDraw(PreferencesType *prefs)
{
  GameGlobals   *gbls;
  Int16         i, index;
  RectangleType rect    = { {   0,   0 }, {   0,   0 } };
  RectangleType scrRect = { {   0,   0 }, {   0,   0 } };

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // 
  // DRAW INFORMATION/BITMAPS ON SCREEN
  //

  // draw the score
  {
    Int16 base;
 
    base = 1000;  // max score (4 digits)
    for (i=0; i<4; i++) {

      index = (prefs->game.gameScore / base) % 10;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i, 
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 7;
      scrRect.extent.y  = 12;
      rect.topLeft.x    = index * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the digit!
      WinCopyRectangle(gbls->winDigits, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
      base /= 10;
    }
  }

  // draw the misses that have occurred :( 
  if (prefs->game.gameLives < 3) {
  
    index = 2 - prefs->game.gameLives;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0, 
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    scrRect.extent.x  = 30;
    scrRect.extent.y  = 20;
    rect.topLeft.x    = index * scrRect.extent.x;
    rect.topLeft.y    = 0;
    rect.extent.x     = scrRect.extent.x;
    rect.extent.y     = scrRect.extent.y;

    // draw the miss bitmap!
    WinCopyRectangle(gbls->winMisses, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
  }

  // no missed, make sure none are shown
  else {
  
    index = 2;  // the miss with *all* three misses

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteMiss, 0, 
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    scrRect.extent.x  = 30;
    scrRect.extent.y  = 20;
    rect.topLeft.x    = index * scrRect.extent.x;
    rect.topLeft.y    = 0;
    rect.extent.x     = scrRect.extent.x;
    rect.extent.y     = scrRect.extent.y;

    // invert the three miss bitmap!
    WinCopyRectangle(gbls->winMisses, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
  }

  // draw the smoke on the screen (only if it has changed)
  if (gbls->smokeChanged) {

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteSmoke, 0, 
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    scrRect.extent.x  = 34;
    scrRect.extent.y  = 21;
    rect.topLeft.x    = 0;  
    rect.topLeft.y    = 0;  
    rect.extent.x     = scrRect.extent.x;
    rect.extent.y     = scrRect.extent.y;

    // 
    // erase the previous smoke
    // 

    if (gbls->smokeOnScreen) {

      index = gbls->smokeOldPosition;

      // what is the rectangle we need to copy?
      rect.topLeft.x = index * scrRect.extent.x; 
      rect.topLeft.y = 0;
      rect.extent.x  = scrRect.extent.x;
      rect.extent.y  = scrRect.extent.y;

      // invert the smoke bitmap!
      WinCopyRectangle(gbls->winSmokes, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
      gbls->smokeOnScreen = false;
    }

    // 
    // draw the smoke at the new position
    // 

    index = prefs->game.phire.smokePosition;

    // what is the rectangle we need to copy?
    rect.topLeft.x = index * scrRect.extent.x; 
    rect.topLeft.y = 0;
    rect.extent.x  = scrRect.extent.x;
    rect.extent.y  = scrRect.extent.y;

    // save this location, record smoke is onscreen
    gbls->smokeOnScreen    = true;
    gbls->smokeOldPosition = index;

    // draw the smoke bitmap!
    WinCopyRectangle(gbls->winSmokes, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);

    // dont draw until we need to
    gbls->smokeChanged = false;
  }

  // draw the jumpers
  for (i=0; i<prefs->game.phire.jumperCount; i++) {

    // draw the jumper on the screen (only if it has changed)
    if (gbls->jumperChanged[i]) {

      //
      // erase the previous jumper
      //
 
      if (gbls->jumperOnScreen[i]) {

        index = gbls->jumperOnScreenPosition[i];

        // what is the rectangle we need to copy?
        GameGetSpritePosition(spriteJumper, index,
                              &scrRect.topLeft.x, &scrRect.topLeft.y);
        scrRect.extent.x  = 14;
        scrRect.extent.y  = 14;
        rect.topLeft.x    = (index % 7) * scrRect.extent.x;
        rect.topLeft.y    = (index / 7) * scrRect.extent.y;
        rect.extent.x     = scrRect.extent.x;
        rect.extent.y     = scrRect.extent.y;

        // invert the old jumper bitmap!
        WinCopyRectangle(gbls->winJumpers, WinGetDrawWindow(),
                         &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
      }

      //
      // draw the jumper at the new position
      //

      index = prefs->game.phire.jumperPosition[i];

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteJumper, index,
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 14;
      scrRect.extent.y  = 14;
      rect.topLeft.x    = (index % 7) * scrRect.extent.x;
      rect.topLeft.y    = (index / 7) * scrRect.extent.y;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // save this location, record jumper is onscreen
      gbls->jumperOnScreen[i]         = true;
      gbls->jumperOnScreenPosition[i] = prefs->game.phire.jumperPosition[i];

      // draw the jumper bitmap!
      WinCopyRectangle(gbls->winJumpers, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);

      // dont draw until we need to
      gbls->jumperChanged[i] = false;
    }
  }

  // draw trampoline (only if it has changed)
  if (gbls->trampolineChanged) {

    // 
    // erase the previous trampoline
    // 

    if (gbls->trampolineOnScreen) {

      index = gbls->trampolineOldPosition;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteTrampoline, index, 
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 35;
      scrRect.extent.y  = 20;
      rect.topLeft.x    = index * scrRect.extent.x; 
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // invert the old trampoline bitmap
      WinCopyRectangle(gbls->winTrampolines, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
      gbls->trampolineOnScreen    = false;
    }

    // 
    // draw trampoline at the new position
    // 

    index = prefs->game.phire.trampolinePosition;

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteTrampoline, index, 
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    scrRect.extent.x  = 35;
    scrRect.extent.y  = 20;
    rect.topLeft.x    = index * scrRect.extent.x; 
    rect.topLeft.y    = 0;
    rect.extent.x     = scrRect.extent.x;
    rect.extent.y     = scrRect.extent.y;

    // save this location, record trampoline is onscreen
    gbls->trampolineOnScreen    = true;
    gbls->trampolineOldPosition = index;

    // draw the trampoline bitmap!
    WinCopyRectangle(gbls->winTrampolines, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);

    // dont draw until we need to
    gbls->trampolineChanged = false;
  }
}

/**
 * Get the position of a particular sprite on the screen.
 *
 * @param spriteType the type of sprite.
 * @param index the index required in the sprite position list.
 * @param x the x co-ordinate of the position
 * @param y the y co-ordinate of the position
 */
void
GameGetSpritePosition(UInt8 spriteType, 
                      UInt8 index, 
                      Coord *x, 
                      Coord *y)
{
  switch (spriteType) 
  {
    case spriteDigit: 
         {
           *x = 69 + (index * 9);
           *y = 38;
         }
         break;

    case spriteMiss: 
         {
           *x = 126;
           *y = 37;
         }
         break;

    case spriteSmoke: 
         {
           *x = 13;
           *y = 36;
         }
         break;

    case spriteTrampoline: 
         {
           Coord positions[][2] = {
                                   {  13, 104 },
                                   {  54, 103 },
                                   {  93, 105 }
                                 };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    case spriteJumper: 
         {
           Coord positions[][2] = {
                                   {   9,  57 },  // jump starters
                                   {   9,  74 }, 
                                   {   0,   0 }, 
                                   {   0,   0 }, 
                                   {   0,   0 }, 
                                   {   0,   0 }, 
                                   {   0,   0 }, 
                                   {  18,  64 },  // falling
                                   {  18,  79 },
                                   {  19,  92 },
                                   {  25, 103 },
                                   {  30,  91 },
                                   {  36,  79 },
                                   {  39,  65 },
                                   {  48,  56 },
                                   {  57,  66 },
                                   {  58,  80 },
                                   {  60,  93 },
                                   {  65, 104 },
                                   {  72,  90 },
                                   {  79,  79 },
                                   {  87,  70 },
                                   {  93,  80 },
                                   {  98,  92 },
                                   { 104, 104 },
                                   { 111,  90 },
                                   { 116,  82 },
                                   { 129,  88 }
                                  };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    case spriteJumperDeath: 
         {
           Coord positions[][2] = {
                                   {  24, 127 },
                                   {  62, 126 },
                                   { 102, 126 }
                                 };

           *x = positions[index][0];
           *y = positions[index][1];
         }
         break;

    default:
         break;
  }
}

/**
 * Terminate the game.
 */
void   
GameTerminate()
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // unlock the gamepad driver (if available)
  if (gbls->hardware.gamePadPresent) {

    Err    err;
    UInt32 gamePadUserCount;

    err = GPDClose(gbls->hardware.gamePadLibRef, &gamePadUserCount);
    if (gamePadUserCount == 0)
      SysLibRemove(gbls->hardware.gamePadLibRef);
  }

  // clean up windows/memory
  if (gbls->winDigits != NULL)
    WinDeleteWindow(gbls->winDigits,       false);
  if (gbls->winMisses != NULL)
    WinDeleteWindow(gbls->winMisses,       false);
  if (gbls->winSmokes != NULL)
    WinDeleteWindow(gbls->winSmokes,       false);
  if (gbls->winTrampolines != NULL)
    WinDeleteWindow(gbls->winTrampolines,  false);
  if (gbls->winJumpers != NULL)
    WinDeleteWindow(gbls->winJumpers,      false);
  if (gbls->winJumperDeaths != NULL)
    WinDeleteWindow(gbls->winJumperDeaths, false);
  MemPtrFree(gbls);

  // unregister global data
  FtrUnregister(appCreator, ftrGameGlobals);
}

/**
 * Adjust the level (remove birds that are too close and reset positions)
 *
 * @param prefs the global preference data.
 */
static void 
GameAdjustLevel(PreferencesType *prefs)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // player should stay were the were
  prefs->game.phire.trampolineNewPosition = prefs->game.phire.trampolinePosition;
  gbls->trampolineChanged                 = true;

  // player is not dead
  gbls->playerDied                        = false;
}

/**
 * Increment the players score. 
 *
 * @param prefs the global preference data.
 */
static void 
GameIncrementScore(PreferencesType *prefs)
{
  GameGlobals    *gbls;
  Int16          i, index;
  RectangleType  rect     = { {   0,   0 }, {   0,   0 } };
  RectangleType  scrRect  = { {   0,   0 }, {   0,   0 } };
  SndCommandType scoreSnd = {sndCmdFreqDurationAmp,0,1024, 5,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // adjust accordingly
  prefs->game.gameScore += prefs->game.phire.bonusScoring ? 2 : 1;

  // redraw score bitmap
  {
    Int16 base;
 
    base = 1000;  // max score (4 digits)
    for (i=0; i<4; i++) {

      index = (prefs->game.gameScore / base) % 10;

      // what is the rectangle we need to copy?
      GameGetSpritePosition(spriteDigit, i, 
                            &scrRect.topLeft.x, &scrRect.topLeft.y);
      scrRect.extent.x  = 7;
      scrRect.extent.y  = 12;
      rect.topLeft.x    = index * scrRect.extent.x;
      rect.topLeft.y    = 0;
      rect.extent.x     = scrRect.extent.x;
      rect.extent.y     = scrRect.extent.y;

      // draw the digit!
      WinCopyRectangle(gbls->winDigits, WinGetDrawWindow(),
                       &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
      base /= 10;
    }
  }

  // play the sound
  DevicePlaySound(&scoreSnd);
  SysTaskDelay(5);

  // is it time for a bonus?
  if (
      (prefs->game.gameScore >= 300) &&
      (prefs->game.phire.bonusAvailable)
     ) {

    SndCommandType snd = {sndCmdFreqDurationAmp,0,0,5,sndMaxAmp};

    // give a little fan-fare sound
    for (i=0; i<15; i++) {
      snd.param1 += 256 + (1 << i);  // frequency
      DevicePlaySound(&snd);

      SysTaskDelay(2); // small deley 
    }

    // apply the bonus!
    if (prefs->game.gameLives == 3) 
      prefs->game.phire.bonusScoring = true;
    else
      prefs->game.gameLives = 3;

    prefs->game.phire.bonusAvailable = false;
  }

#ifdef PROTECTION_ON 
  // is it time to say 'bye-bye' to our freeware guys?  
  if (
      (prefs->game.gameScore >= 50) && 
      (!CHECK_SIGNATURE(prefs))
     ) {

    EventType event;

    // "please register" dialog :)
    ApplicationDisplayDialog(rbugForm);

    // GAME OVER - return to main screen
    MemSet(&event, sizeof(EventType), 0);
    event.eType            = menuEvent;
    event.data.menu.itemID = gameMenuItemExit;
    EvtAddEventToQueue(&event);

    // stop the game in its tracks
    prefs->game.gamePlaying = false;
  }
#endif
}

/**
 * Move the player.
 *
 * @param prefs the global preference data.
 */
static void
GameMovePlayer(PreferencesType *prefs) 
{
  GameGlobals    *gbls;
  SndCommandType plymvSnd = {sndCmdFreqDurationAmp,0, 768, 5,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  //
  // where does trampoline want to go today?
  //

  // current position differs from new position?
  if (prefs->game.phire.trampolinePosition != 
      prefs->game.phire.trampolineNewPosition) {

    // need to move left
    if (prefs->game.phire.trampolinePosition > 
        prefs->game.phire.trampolineNewPosition) {

      gbls->moveNext = moveLeft;
    }

    // need to move right
    else
    if (prefs->game.phire.trampolinePosition < 
        prefs->game.phire.trampolineNewPosition) {

      gbls->moveNext = moveRight;
    }
  }

  // lets make sure they are allowed to do the move
  if (
      (gbls->moveDelayCount == 0) || 
      (gbls->moveLast != gbls->moveNext) 
     ) {
    gbls->moveDelayCount = 
     ((gbls->gameType == GAME_A) ? 4 : 3);
  }
  else {
    gbls->moveDelayCount--;
    gbls->moveNext = moveNone;
  }

  // which direction do they wish to move?
  switch (gbls->moveNext)
  {
    case moveLeft:
         {
           prefs->game.phire.trampolinePosition--;
           gbls->trampolineChanged = true;
         }
         break;

    case moveRight:
         {
           prefs->game.phire.trampolinePosition++;
           gbls->trampolineChanged = true;
         }
         break;

    default:
         break;
  }

  gbls->moveLast = gbls->moveNext;
  gbls->moveNext = moveNone;

  // do we need to play a movement sound? 
  if (gbls->trampolineChanged)  
    DevicePlaySound(&plymvSnd);
}

/**
 * Move the jumpers.
 *
 * @param prefs the global preference data.
 */
static void
GameMoveJumpers(PreferencesType *prefs)
{
  GameGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // only do this if the player is still alive
  if (!gbls->playerDied) {

    SndCommandType grdmvSnd = {sndCmdFreqDurationAmp,0, 384, 5,sndMaxAmp};
    UInt16         i, j;

    // process the jumpers
    i = 0;
    while (i<prefs->game.phire.jumperCount) {

      UInt16  bounceIndex[] = { 10, 18, 24 };
      Boolean removal = false;

      if (prefs->game.phire.jumperWait[i] == 0) {

        Boolean ok;

        // lets make sure it is not moving into a jumper in front of us?
        ok = true;
        for (j=0; j<prefs->game.phire.jumperCount; j++) {
          ok &= (
                 (prefs->game.phire.jumperPosition[i]+1 !=
                  prefs->game.phire.jumperPosition[j])
                );
        }

        // the coast is clear, move!
        if (ok) {

          // CASE 1: has the jumper hit the floor?
	  for (j=0; j<3; j++) {

            if (prefs->game.phire.jumperPosition[i] == bounceIndex[j]) {

              // trampoline was not there, death
              if (prefs->game.phire.trampolinePosition != j) {

                gbls->playerDied |= true;
                prefs->game.phire.jumperDeathPosition = j;

                // remove the jumper
                GameRemoveJumper(prefs, i); removal = true;
              }

              // trampoline was there, score
              else 
                GameIncrementScore(prefs);

              break; // get out of loop
            }
          }

          // CASE 2: has the jumper made it to the ambulance?
          if ((prefs->game.phire.jumperPosition[i] == 27)) {

            // remove the jumper
            GameRemoveJumper(prefs, i); removal = true;
          }

          // did we not have to remove it?
	  if (!removal) {

            if (prefs->game.phire.jumperPosition[i] < 7) 
              prefs->game.phire.jumperPosition[i] += 6;
            prefs->game.phire.jumperPosition[i]++;

            prefs->game.phire.jumperWait[i] =
              (gbls->gameType == GAME_A) ? 6 : 4;
            gbls->jumperChanged[i] = true;

            DevicePlaySound(&grdmvSnd);
          }
        }
      }
      else {

        prefs->game.phire.jumperWait[i]--;

        // has the jumper been "bounced" off?
        for (j=0; j<3; j++) {

          if (
	      (prefs->game.phire.jumperPosition[i] == bounceIndex[j]) &&
              (prefs->game.phire.trampolinePosition == j)
	     ) {

            // increase score
	    GameIncrementScore(prefs);

            // move the jumper along
            prefs->game.phire.jumperPosition[i]++;
            prefs->game.phire.jumperWait[i] =
              (gbls->gameType == GAME_A) ? 6 : 4;
            gbls->jumperChanged[i] = true;
	  }
        }
      }

      if (!removal) i++;
    }

    // new jumper appearing on screen?
    {
      Boolean ok;
      UInt8   birthFactor        = (gbls->gameType == GAME_A) ? 8 : 4;
      UInt8   maxOnScreenJumpers = prefs->game.phire.gameLevel;

      // we must be able to add a jumper (based on level)
      ok = (
            (prefs->game.phire.jumperCount < maxOnScreenJumpers) &&
	    ((SysRandom(0) % birthFactor) == 0)
           );

      // lets make sure there are not any jumpers already :)
      for (i=0; i<prefs->game.phire.jumperCount; i++) {
        ok &= (prefs->game.phire.jumperPosition[i] > 8);
      }

      // lets add a new jumper
      if (ok) {
        UInt8 new, pos;
	
	if (gbls->gameType == GAME_A) new = 0;
	else                          new = SysRandom(0) % 2;

        pos = prefs->game.phire.jumperCount++;
	prefs->game.phire.jumperPosition[pos] = new;
	prefs->game.phire.jumperWait[pos]     =
	  (gbls->gameType == GAME_A) ? 6 : 4;
        gbls->jumperChanged[pos]              = true;
	gbls->jumperOnScreen[pos]             = false;
	gbls->jumperOnScreenPosition[pos]     = 0;

	DevicePlaySound(&grdmvSnd);
      }
    }
  }
}

/**
 * Remove a jumper from the game.
 *
 * @param prefs the global preference data.
 * @param jumperIndex the index of the jumper to remove.
 */
static void 
GameRemoveJumper(PreferencesType *prefs, 
                 Int16           jumperIndex)
{
  GameGlobals   *gbls;
  Int16         index;
  RectangleType rect    = { {   0,   0 }, {   0,   0 } };
  RectangleType scrRect = { {   0,   0 }, {   0,   0 } };

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&gbls);

  // 
  // remove the bitmap from the screen
  //
 
  if (gbls->jumperOnScreen[jumperIndex]) {

    index = gbls->jumperOnScreenPosition[jumperIndex];

    // what is the rectangle we need to copy?
    GameGetSpritePosition(spriteJumper, index,
                          &scrRect.topLeft.x, &scrRect.topLeft.y);
    scrRect.extent.x  = 14;
    scrRect.extent.y  = 14;
    rect.topLeft.x    = (index % 7) * scrRect.extent.x;
    rect.topLeft.y    = (index / 7) * scrRect.extent.y;
    rect.extent.x     = scrRect.extent.x;
    rect.extent.y     = scrRect.extent.y;

    // invert the old jumper bitmap!
    WinCopyRectangle(gbls->winJumpers, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
  }

  //
  // update the information arrays
  //

  // we will push the 'jumper' out of the array
  //
  // before: 1234567---  after: 1345672---
  //          ^     |                 |
  //                end point         end point

  prefs->game.phire.jumperCount--;

  // removal NOT from end?
  if (prefs->game.phire.jumperCount > jumperIndex) {

    Int16 i, count;

    count = prefs->game.phire.jumperCount - jumperIndex;

    // shift all elements down
    for (i=jumperIndex; i<(jumperIndex+count); i++) {
      prefs->game.phire.jumperPosition[i] = prefs->game.phire.jumperPosition[i+1];
      prefs->game.phire.jumperWait[i]     = prefs->game.phire.jumperWait[i+1];
      gbls->jumperChanged[i]              = gbls->jumperChanged[i+1];
      gbls->jumperOnScreen[i]             = gbls->jumperOnScreen[i+1];
      gbls->jumperOnScreenPosition[i]     = gbls->jumperOnScreenPosition[i+1];
    }
  }
}
