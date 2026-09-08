#ifndef CREATURE_API_H
#define CREATURE_API_H
#include <stddef.h>
#include <stdint.h>
#ifdef _WIN32
#define CR_API __declspec(dllexport)
#else
#define CR_API __attribute__((visibility("default")))
#endif
#ifdef __cplusplus
extern "C" {
#endif
// Caller owns buffers. Functions return 0 on success, -1 on invalid arguments.
// Handles must originate from cr_create; use each handle on one thread at a time.
CR_API uint32_t cr_version(void);
// Initial tick only. Returns 0 on success, -1 for invalid limits or a running world.
CR_API int32_t cr_development(void *world, int32_t actor, int32_t arts, int32_t pace, int32_t capacity, int32_t recovery);
CR_API uint32_t cr_content_hash(void);
CR_API uint32_t cr_observation_version(void);
CR_API int32_t cr_arena_count(void);
CR_API const char *cr_arena_name(int32_t id);
CR_API int32_t cr_species_count(void);
CR_API const char *cr_species_name(int32_t id);
CR_API int32_t cr_reset_match(void *, uint32_t seed, int32_t weather, int32_t a, int32_t b,
                              int32_t arena);
CR_API int32_t cr_observation_size(void);
CR_API void *cr_create(uint32_t seed, int32_t weather);
CR_API void cr_destroy(void *world);
CR_API int32_t cr_reset(void *, uint32_t seed, int32_t weather);
CR_API int32_t cr_step(void *, const int32_t *actions10, int32_t *features20, int32_t *status4);
CR_API int32_t cr_observe(void *, int32_t agent, float *output, size_t count);
CR_API int32_t cr_scripted(void *, int32_t agent, int32_t *action5);
// Batched teacher/opponent policy: two styles (0..3) per world, ten action integers per world.
CR_API int32_t cr_batch_scripted(void *const *worlds, size_t count, const int32_t *styles,
                                 int32_t *actions);
CR_API int32_t cr_command(void *, int32_t agent, int32_t guidance);
CR_API size_t cr_snapshot(void *, uint8_t *output, size_t capacity);
CR_API int32_t cr_restore(void *, const uint8_t *, size_t size);
CR_API uint64_t cr_hash(void *);
// One foreign-function call for N arenas, both players. No auto-reset.
CR_API int32_t cr_batch_step(void *const *worlds, size_t count, const int32_t *actions,
                             float *observations, int32_t *features, int32_t *status);
#ifdef __cplusplus
}
#endif
#endif
