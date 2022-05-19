/*
 * @(#)game.h
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

#ifndef _GAME_H
#define _GAME_H

#include "palm.h"

enum gameType 
{ 
  GAME_A = 0, 
  GAME_B 
};

enum spriteType
{
  spriteDigit = 0,
  spriteMiss,
  spriteSmoke,
  spriteTrampoline,
  spriteShark,
  spriteJumper,
  spriteJumperDeath
};

enum moveType
{
  moveNone    = 0,
  moveLeft,
  moveRight
};

extern Boolean GameInitialize()                                       __GAME__;
extern void   GameReset(PreferencesType *, Int8)                      __GAME__;
extern void   GameResetPreferences(PreferencesType *, Int8)           __GAME__;
extern void   GameProcessKeyInput(PreferencesType *, UInt32)          __GAME__;
extern void   GameProcessStylusInput(PreferencesType *, Coord, Coord) __GAME__;
extern void   GameMovement(PreferencesType *)                         __GAME__;
extern void   GameDraw(PreferencesType *)                             __GAME__;
extern void   GameGetSpritePosition(UInt8, UInt8, Coord *, Coord*)    __GAME__;
extern void   GameTerminate()                                         __GAME__;

#endif 
