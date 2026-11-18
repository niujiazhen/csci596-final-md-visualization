# One-minute repository presentation

Use the GitHub repository itself, as required by the assignment. Do not prepare or switch to slides. The draft below describes the implementation already completed locally; update it if the approved scope changes. Practice once aloud and trim to your speaking pace.

## Suggested spoken outline

My project extends our molecular-dynamics viewer to show particle speeds and trajectories.

The original viewer draws particles in one color, so it is hard to see which ones are moving faster or follow one particle over time.

I added a fixed speed color scale, from blue to red, and a recent trajectory for atom zero. When the atom crosses a periodic boundary, the trajectory is split to avoid a false line through the box. Both features can be toggled interactively.

The expected outcome is a clearer view of particle motion without changing the simulation. Initial verification is complete: after three thousand steps, all particle positions and velocities match the original integrator, and the energy samples match too.

The remaining course steps are topic approval, presentation, and final repository submission.

## What to show

| Time | Repository content |
| --- | --- |
| 0–15 seconds | README title and problem statement |
| 15–40 seconds | Three same-step screenshots; point out legend, white halo and trail |
| 40–60 seconds | README validation paragraph and report link |

Do not spend the one-minute slot browsing source code or installing anything. The screenshot comparison is enough to show the features from the repository.

## Questions you should be able to answer

- **Why use a fixed color scale?** So the same color means the same speed across frames. Above 4, the color saturates at red.
- **Why split the trail?** Periodic wrapping makes adjacent positions appear far apart; drawing a connecting line would misrepresent motion.
- **Why is this beyond AS07?** Speed coloring, the tracked trail, boundary handling, new controls and read-only verification are additions. The base physics and viewer are credited as inherited.
- **Did the simulation become faster?** No performance claim is made; this is a visualization project.
- **Does color mean temperature?** No. It encodes a particle's instantaneous speed in reduced units.
