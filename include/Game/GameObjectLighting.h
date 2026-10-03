#ifndef _GAMEOBJECTLIGHTING_H_
#define _GAMEOBJECTLIGHTING_H_

#include "NL/nlColour.h"
#include "types.h"

class GLView;
struct GameObjectLight;
class nlMatrix4;
class ImpostorModel;
class LightingLookup;
class TweakValueFloat;
class StadiumLight;

extern TweakValueFloat gShadowLookupScaleX;
extern TweakValueFloat gShadowLookupScaleY;
extern TweakValueFloat gShadowLookupTransX;
extern TweakValueFloat gShadowLookupTransY;
extern LightingLookup* gpShadowLightingLookup;

class nlVector2;
class nlVector3;
struct glModel;
u32 GetGameObjectLightRamp();
nlColour fn_80183C9C(const nlVector2* arg0, bool arg1);
int fn_80183DEC(const nlVector3*);
void fn_80183E4C();
void fn_80183E8C(ImpostorModel*, glModel*);
void fn_80183F78(ImpostorModel*, glModel*);
void UpdateGameObjectLighting();
void InitializeGameObjectLighting();
void PrepareStadiumLight(StadiumLight* light);
bool AlwaysUseCameraRelativeCharacterLighting();

// Shared lighting hooks used by the material programs.
bool fn_80183C54();
int IsGameObjectLightingEnabled();
int ShouldUseGameObjectLightTexture(int character);
int ShouldDoubleGameObjectLighting();
int GetGameObjectLightCount(bool character, bool includeEffects);
GameObjectLight* GetGameObjectLight(int index, bool character);
void SetGameObjectLightingMode(int mode);
void SetGameObjectLightTexture(unsigned long texture);
void LoadGameObjectSpecularLight(int index, GameObjectLight* light, float exponent, const nlMatrix4& viewMatrix);
void SetGameObjectSpecularLightingEnabled(int enabled, int count);
unsigned long GetGameObjectLightTexture();
void LoadGameObjectLights(int count, GLView* view, bool character);
void SetGameObjectLightingEnabled(bool enabled, int count, bool useVertexColour);
void SetGameObjectAmbientLightingEnabled(int enabled);
void ApplyGameObjectShadowLighting(int skinned, unsigned long shadowLevel);
void RestoreGameObjectShadowLighting();
void SetGameObjectShadowModelMatrix(unsigned long matrix);
void SetGameObjectShadowViewMatrix(const nlMatrix4* matrix);
void fn_80182164();
void fn_80183764(unsigned long texture);

#endif // _GAMEOBJECTLIGHTING_H_
