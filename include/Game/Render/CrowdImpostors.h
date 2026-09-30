#ifndef GAME_RENDER_CROWD_IMPOSTORS_H
#define GAME_RENDER_CROWD_IMPOSTORS_H

#include "Game/Render/CrowdModelCollection.h"
#include "Game/Render/ImpostorCharacter.h"
#include "NL/nlMemory.h"

void UpdateImpostorPositions();
void LoadCrowdCharacterList();
void InitializeCrowdImpostors(bool alternateView);
void UninitializeCrowdImpostors();
void CreateCrowdLayoutObject();
void SetCrowdModelTexture(unsigned int hash, unsigned long texture);
void SetCrowdImpostorsExcited();
void SetCrowdImpostorsIdle();
void UpdateCrowdImpostorAnimation(float value);

#endif // GAME_RENDER_CROWD_IMPOSTORS_H
