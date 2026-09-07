# Tinikami artwork

Original spirit and world artwork generated for this project using OpenAI image generation, under the repository's MIT license. These are new Tinikami designs; no third-party character sheets are included.

`animations/00-cinderfox.png` through `39-oathhound.png` retain the generated source sheets. The numbered `0.png` / `0.rgba` pairs are imported runtime atlases. Each has four rows (south, east, north, west) and eight columns (idle, walk A, walk B, windup, attack, hit, fall, fainted). `animations/manifest.json` records the exact layout and species mapping. All forty species have 32 authored cells.

`animations/prompts.json` and `animations/corrections.json` retain generation and correction prompts. World prompts are in `journey-prompts.json`, `biome-prompt.txt`, `biome-cleanup-prompt.txt` and `inn-icon-prompts.json`. Generated opaque matte/checker previews required an import step; source files remain intact. The cleaned biome source supersedes its original for runtime import.

Reimport with `python3 scripts/compile_animations.py` from the repository root (Pillow, NumPy and SciPy required). The importer removes background mattes, extracts authored poses, normalizes scale and foot pivots, and writes PNG and raw RGBA. It does not synthesize additional poses. `.rgba` is `TINI`, little-endian width and height, then row-major RGBA bytes; the native client needs no image library.

The native Sprite Studio, available from the campaign title, shows every frame and physical size comparison. Animation is deliberately sparse and stepped. Some generated sheets still have imperfect silhouettes and pose-to-pose consistency; a hand-authored art pass remains appropriate before a final release. Combat effects and hitbox overlays are drawn by the renderer from engine state.

The earlier combat-skin assets are also retained: `spirits.png` is the original static 8×5 roster; `environment.png` is the first terrain sheet; `environment-v2.png` supplies the quieter active combat floor and props; and `effects.png` supplies sixteen reusable motifs combined with engine-authored geometry, element and timing. Their exact prompts remain in `prompts.json` and `environment-v2-prompt.json`. `python3 scripts/compile_art.py` rebuilds their raw textures and `client/spirit_rects.hpp` with Pillow. The old static roster remains a workbench fallback, while the campaign requires all animated sheets.

Runtime terrain grading reserves contrast and saturation for actors and active effects while leaving source files intact. Precise facing remains visible in the ground chevron; H reveals physical geometry. Cosmetic pixels never determine hits or change information available to policies. See [combat garden hierarchy](../../docs/alpha/gardens.md).
