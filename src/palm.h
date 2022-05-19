/*
 * @(#)palm.h
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

#ifndef _PALM_H
#define _PALM_H

// system includes
#include <PalmOS.h>
#include <System/DLServer.h>

// resource "include" :P
#include "resource.h"

// special includes (additional hardware etc)
#include "hardware/GPDLib.h"

// application constants and structures
#define appCreator 'FYRE'
#define __REGISTER__ __attribute__ ((section ("register"))) // code0002.bin
#define __DEVICE__   __attribute__ ((section ("device")))   // code0003.bin
#define __GAME__     __attribute__ ((section ("game")))     // code0004.bin
#define __HELP__     __attribute__ ((section ("help")))     // code0005.bin
#define __SAFE0001__ __attribute__ ((section ("safe0001"))) // code0006.bin
#define __SAFE0002__ __attribute__ ((section ("safe0002"))) // code0007.bin

#define ftrGlobals             1000
#define ftrGlobalsCfgActiveVol 1001
#define ftrDeviceGlobals       2000
#define ftrGameGlobals         3000
#define ftrHelpGlobals         4000
#define ftrRegisterGlobals     1004 // <-- must be, as original code0007.bin
//#define ftrRegisterGlobals   5000

#define GAME_FPS  8  // 8 frames per second
#define MAX_JUMP  22 // 21 jumpers, one starter
#define VERSION   1

typedef struct
{
  struct 
  {
    UInt8     signatureVersion;         // a preference version number
    Char      signature[16];            // a "signature" for decryption
    Char      *hotSyncUsername;         // the HotSync user name of the user
  } system;

  struct 
  {
    UInt16    ctlKeyLeft;               // key definition for move left
    UInt16    ctlKeyRight;              // key definition for move right

    Boolean   sndMute;                  // sound is muted?
    UInt16    sndVolume;                // the volume setting for sound

    UInt8     lgray;                    // the light gray configuration setting
    UInt8     dgray;                    // the dark gray configuration setting
  } config;

  struct 
  {
    Boolean   gamePlaying;              // is there a game in play?
    Boolean   gamePaused;               // is the game currently paused?
    Boolean   gameWait;                 // are we waiting for user? 
    UInt16    gameAnimationCount;       // a ticking counter
    UInt16    gameScore;                // the score of the player
    UInt16    highScore[2];             // a high score list (score only)
    UInt16    gameLives;                // the number of lives the player has

    struct 
    {
      Int8    gameType;                 // what type of game are we playing?
      UInt16  gameLevel;                // what level are we at?
      Boolean bonusAvailable;           // is there a bonus available?
      Boolean bonusScoring;             // are we currently in BONUS mode?

      UInt16  smokeWait;                // the delay between smoke positions
      UInt16  smokePosition;            // the position of the smoke

      UInt16  trampolinePosition;       // the position of trampoline 
      UInt16  trampolineNewPosition;    // the desired position of trampoline 

      UInt16  jumperCount;              // the number of jumpers on screen
      UInt16  jumperWait[MAX_JUMP];     // the delay between jumper moves
      UInt16  jumperPosition[MAX_JUMP]; // the position of the jumper

      UInt16  jumperDeathPosition;      // the index of death?
    } phire;

  } game;
  
} PreferencesType;

// this is out 'double check' for decryption - make sure it worked :P
#define CHECK_SIGNATURE(x) (StrCompare(x->system.signature, "|HaCkMe|") == 0)

// local includes
#include "device.h"
#include "help.h"
#include "game.h"
#include "register.h"
#include "gccfix.h"

// functions
extern UInt32  PilotMain(UInt16, MemPtr, UInt16);
extern void    InitApplication(void);
extern Boolean ApplicationHandleEvent(EventType *);
extern void    ApplicationDisplayDialog(UInt16);
extern void    EventLoop(void);
extern void    EndApplication(void);

#endif 
