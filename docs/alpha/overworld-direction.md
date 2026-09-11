# Overworld direction: reuse an RPG authoring environment

The SDL campaign proves transitions and combat progression, but its hand-built menus, map generation, movement and scene orchestration are not a strong production authoring workflow. The reusable C++ simulation is worth keeping. Expanding eight rough regions further will not solve the problem.

## Recommendation

Evaluate RPG Maker MZ first if the priority is mature RPG tools and immediate content authoring. It provides a map editor, event commands, dialogue, a database, menus and saving. Extend it with an original Tinikami collection/party plugin and replace the battle scene. Do not try to express our real-time combat through MZ's default battle formulas.

Godot is the stronger native integration option: its official godot-cpp GDExtension bindings can host our C++ engine without recompiling Godot. It supplies scene, animation, input, UI and tilemap tools. It is not a complete creature-collector framework, however; choosing it still means owning RPG event logic, collection, progression and save integration. The official RPG demo is a small example, not a mature finished RPG foundation.

The Emerald expansion project is a GBA ROM-hack base. Its existing exploration and creature systems are attractive, but they are coupled to that hardware/runtime and to Pokémon's game structures. Our desktop C++ controller inference and real-time arena are not a drop-in battle replacement. A desktop port or emulator/host bridge would introduce an additional platform project. Do not start there to reduce engineering work.

Pokémon Essentials is another relevant ready-made creature RPG project, but it specifically targets RPG Maker XP, not MZ. Its code would not install as an MZ plugin. Treat adapting that older framework and integrating native combat as a separate candidate, rather than assuming MZ comes with its collection systems. [Project description](https://github.com/Maruno17/pokemon-essentials).

RPG Maker is a licensed engine. Our original plugin and C++ engine can remain open source; do not assume the entire RPG Maker runtime or bundled assets can be republished under our MIT license. Check the applicable [official terms](https://www.rpgmakerweb.com/eula) before defining the distribution package.

These are engineering judgments, not completed integration benchmarks. No new framework has been installed or migration performed.

## Proposed MZ integration

- MZ owns map movement, NPC event flow, dialogue, scene transitions, inventory presentation and the enclosing save file.
- A Tinikami plugin owns collection records, bond unlocks, temperament settings, encounter preparation and reward application. Keep one authoritative record for each value; do not maintain independent RPG Maker HP/XP and C++ campaign copies.
- C++ owns combat timing, collision, terrain, energy, cooldowns, observations and controller inference. Build it with Emscripten into WebAssembly. Native C++ remains the training and test implementation.
- A replacement battle scene renders sprites and telegraphs from a versioned presentation frame. Advance fixed simulation ticks with a capped accumulator; render between ticks. Pausing the host pauses the simulation. Do not transfer the engine's collision rules into JavaScript.
- The existing C ABI already supports world creation, actions, observations, snapshots and hashes. It needs a render-facing frame export; observation tensors are a policy interface, not an appropriate display API. Expose bodies, casts, projectiles, terrain, wind and events with stable IDs and explicit units.
- The neural C ABI currently loads model files by path. Package a model in Emscripten's virtual filesystem, or add a validated byte-buffer loader. Preserve one recurrent memory per actor and reset it at round boundaries.
- Battle start accepts encounter ID, seed, party development, condition, temperament and rules/content identity. Battle completion returns a single result with rewards applied exactly once. Returning to the map must restore the same event interpreter and position.
- Preserve a backup when importing old journey saves. For the first migration, resume between encounters; mid-battle saves require simulation, controller memories and host encounter state together. Existing snapshots alone do not capture recurrent controller state.

## Small acceptance slice before choosing

Build one polished, walkable Hearthmere village and one short route, with doors, collision, NPC dialogue, sanctuary recovery, a party page and one encounter. Run our actual battle in the same game window, return with the correct HP/XP, save, quit and reload. Validate keyboard/controller focus, text advance, camera, pause, audio and map/battle transitions by playing the whole loop.

Before treating WebAssembly as proven, replay identical seeds/actions on native and Wasm and compare integer state hashes; compare neural inference tolerances and quantized actions separately. Test on the target Mac deployment. Only then choose the foundation and migrate more content. MZ installation/engine availability is still to be established; nothing has been purchased.

## Sources checked September 10, 2026

- [RPG Maker MZ features](https://www.rpgmakerweb.com/products/rpg-maker-mz): map editing, events, database, plugins, autosave.
- [Official MZ plugin introduction](https://www.rpgmakerweb.com/blog/using-plugins-in-mz): JavaScript extension mechanism.
- [Emscripten C++/JavaScript interoperation](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html).
- [Official Godot C++ integration](https://docs.godotengine.org/en/stable/tutorials/scripting/cpp/about_godot_cpp.html).
- [Official Godot RPG demo scope](https://github.com/godotengine/godot-demo-projects/blob/master/2d/role_playing_game/README.md).
- [Emerald expansion's own project description](https://github.com/rh-hideout/pokeemerald-expansion).
