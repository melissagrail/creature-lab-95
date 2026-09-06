#ifndef TINIKAMI_BRAIN_API_H
#define TINIKAMI_BRAIN_API_H
#include "api.h"
#ifdef __cplusplus
extern "C" {
#endif
CR_API void *tb_create(const char *path);
CR_API void tb_destroy(void *brain);
// Output = mean[4], masked logits[6], value[1], next memory[96]. Caller owns all buffers.
CR_API int32_t tb_forward(void *brain, const float *observation, const float *memory,
                          float *output);
CR_API int32_t tb_action(void *brain, const float *observation, float *memory, int32_t *action);
// Three persistent preferences in [-1,1]: aggression, energy reserve, territoriality.
// Neutral legacy entry points above remain compatible with format-2 models.
CR_API int32_t tb_forward_personality(void *brain, const float *observation, const float *memory,
                                      const float *personality, float *output);
CR_API int32_t tb_action_personality(void *brain, const float *observation, float *memory,
                                     const float *personality, int32_t *action);
#ifdef __cplusplus
}
#endif
#endif
