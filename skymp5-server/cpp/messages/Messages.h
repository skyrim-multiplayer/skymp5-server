#pragma once
#include "ActivateMessage.h"
#include "ChangeValuesMessage.h"
#include "CreateActorMessage.h"
#include "CustomPacketMessage.h"
#include "DeathStateContainerMessage.h"
#include "HostStartMessage.h"
#include "HostStopMessage.h"
#include "OnEquipMessage.h"
#include "OpenContainerMessage.h"
#include "PlayerBowShotMessage.h"
#include "PutItemMessage.h"
#include "SetInventoryMessage.h"
#include "SetRaceMenuOpenMessage.h"
#include "SpSnippetMessage.h"
#include "SpellCastMessage.h"
#include "TakeItemMessage.h"
#include "TeleportMessage.h"
#include "TeleportMessage2.h"
#include "UpdateAnimVariablesMessage.h"
#include "UpdateAnimationMessage.h"
#include "UpdateAppearanceMessage.h"
#include "UpdateEquipmentMessage.h"
#include "UpdateGameModeDataMessage.h"
#include "UpdateMovementMessage.h"
#include "UpdatePropertyMessage.h"

#define REGISTER_MESSAGES                                                     \
  REGISTER_MESSAGE(ActivateMessage)                                           \
  REGISTER_MESSAGE(ConsoleCommandMessage)                                     \
  REGISTER_MESSAGE(CraftItemMessage)                                          \
  REGISTER_MESSAGE(CustomEventMessage)                                        \
  REGISTER_MESSAGE(DestroyActorMessage)                                       \
  REGISTER_MESSAGE(DropItemMessage)                                           \
  REGISTER_MESSAGE(FinishSpSnippetMessage)                                    \
  REGISTER_MESSAGE(HitMessage)                                                \
  REGISTER_MESSAGE(HostMessage)                                               \
  REGISTER_MESSAGE(UpdateMovementMessage)                                     \
  REGISTER_MESSAGE(UpdateAnimationMessage)                                    \
  REGISTER_MESSAGE(DeathStateContainerMessage)                                \
  REGISTER_MESSAGE(ChangeValuesMessage)                                       \
  REGISTER_MESSAGE(TeleportMessage)                                           \
  REGISTER_MESSAGE(UpdatePropertyMessage)                                     \
  REGISTER_MESSAGE(OpenContainerMessage)                                      \
  REGISTER_MESSAGE(UpdateEquipmentMessage)                                    \
  REGISTER_MESSAGE(CustomPacketMessage)                                       \
  REGISTER_MESSAGE(HostStartMessage)                                          \
  REGISTER_MESSAGE(HostStopMessage)                                           \
  REGISTER_MESSAGE(OnEquipMessage)                                            \
  REGISTER_MESSAGE(PlayerBowShotMessage)                                      \
  REGISTER_MESSAGE(PutItemMessage)                                            \
  REGISTER_MESSAGE(SetInventoryMessage)                                       \
  REGISTER_MESSAGE(SetRaceMenuOpenMessage)                                    \
  REGISTER_MESSAGE(SpellCastMessage)                                          \
  REGISTER_MESSAGE(SpSnippetMessage)                                          \
  REGISTER_MESSAGE(TakeItemMessage)                                           \
  REGISTER_MESSAGE(TeleportMessage2)                                          \
  REGISTER_MESSAGE(UpdateAnimVariablesMessage)                                \
  REGISTER_MESSAGE(UpdateAppearanceMessage)                                   \
  REGISTER_MESSAGE(UpdateGameModeDataMessage)                                 \
  REGISTER_MESSAGE(CreateActorMessage)
