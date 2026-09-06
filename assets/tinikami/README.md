# Tinikami art assets

Original spirit-themed art generated for this project with the built-in `image_gen` tool. [The exact prompts](prompts.json) are saved for all three sheets. The art is included under the repository MIT license. No existing character sprites or game assets were used as input.

- `spirits.png`: 40 species, ordered by stable registry ID, nominal 8 × 5 layout.
- `environment.png`: original v6 terrain sheet, retained for reproducibility.
- `environment-v2.png`: active quiet terrain sheet: 8 × 4 cells; twelve base variants, four paths, eight ground materials and eight props. [Exact generation prompt](environment-v2-prompt.json), generated with built-in `image_gen`.
- `effects.png`: 16 reusable elemental and spell-form motifs. All 160 arts combine these with engine-authored geometry, element, timing and movement.
- `.rgba`: lossless decoded texture data, 4-byte `TINI` magic, little-endian width/height, then straight RGBA bytes. No engine or SDL_image dependency is added.

`python3 scripts/compile_art.py` requires Pillow and rebuilds raw textures plus `client/spirit_rects.hpp`. It preserves source pixels and alpha; source rectangles associate disconnected wisps with the nearest spirit and avoid nominal-cell neighbor fragments. Runtime sprites fit those rectangles while preserving aspect ratio.

This first pass uses one illustrated pose per spirit with runtime bob, facing flip, cast/recovery cues, trails and event-driven effects. It does not yet contain authored four-direction walk/attack frame sets. Precise physical facing stays visible in the ground chevron; H reveals collision geometry. Cosmetic pixels never drive hits or hide actor information from policies.

The v7 renderer compresses terrain value/chroma at load time and leaves source pixels intact. Floor materials occupy a narrow mid-value band; props keep more contrast. Large feathered tiles vary by seeded rotation, reflection, offset and variant. Active ability motifs gain a one-pixel light rim; spirits gain a dark silhouette rim. [Hierarchy and layouts](../../docs/alpha/gardens.md).
