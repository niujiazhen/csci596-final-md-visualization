# Sources and contribution boundaries

## Preserved material

- `original/md.c` and `original/md.h`: course molecular-dynamics force model, initialization and velocity-Verlet integration, copied unchanged from the existing AS07 directory.
- `original/md_animation.c`: unchanged copy of the existing AS07 viewer, retained as provenance. Its original relative include paths refer to its former AS07 location; this archived file is not a build target.
- `original/md_animation.in`: unchanged AS07 example input. `examples/default.in` is the same input used for this project.
- `src/atomv_sphere.h`: inherited sphere geometry, copied unchanged from AS07, originally from the course `atomv.c`.

No third-party code license was found or inferred from these copies. Do not apply a blanket license to inherited code without checking its terms.

## Final Project additions

`src/md_visualizer.c` is an extension of the archived AS07 viewer. New features are speed coloring, fixed-scale legend, atom-0 halo, per-integration-step trail sampling, periodic segmentation, feature controls, read-only observation, CSV/state/summary export, headless and screenshot modes, and input/displacement checks. It uses the original integrator directly rather than a rewritten force model.

`src/visualization.h` contains the new speed map and trail helpers. Test drivers, build/run scripts, project report and guides support reproducibility. There are no team collaborators to divide work between. Codex assisted implementation, testing and writing; the student must review the work and comply with the course's assistance policy.

## Course references

- [CSCI 596 course page](https://aiichironakano.github.io/cs596.html)
- [Final Project lecture notes](https://aiichironakano.github.io/cs596/FinalNote.pdf)
- [GitHub and Final Project submission guide](https://aiichironakano.github.io/cs596/Git-VG.pdf)
- [Visualizing molecular dynamics](https://aiichironakano.github.io/cs596/Visual.pdf)

The user-supplied two-page `Final.pdf` is the source for the November 20 topic discussion, approximately one-minute repository-only presentation, December 11 submission deadline and programming/proposal options. It is not redistributed in this repository.
