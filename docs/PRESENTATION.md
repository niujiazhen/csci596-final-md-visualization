# November 30: one-minute repository presentation

**Presenter:** Jiazhen Niu · **Course:** CSCI 596 · **Date:** November 30, 2026

Use the GitHub repository itself, as required by the assignment. Show the README and its screenshots; do not switch to slides. Topic approval and signup are complete.

## Spoken script

Hello, I'm Jiazhen Niu. My project makes particle motion easier to understand in our molecular dynamics viewer.

The original viewer shows identical colors, so speeds and individual paths are difficult to see.

I added a fixed speed color scale, from blue to red, and a recent path for one highlighted particle. At periodic boundaries, the path is split to avoid false lines across the box.

The screenshots compare both features at the same simulation step.

To check correctness, I compared the extended program with the original simulation. After three thousand steps, all particle positions and velocities match, and all energy samples match too.

The result is a clearer visualization without changing the simulated motion.

## Repository navigation

| Time | Show | Say |
| --- | --- | --- |
| 0–12 seconds | README title and main image | Name, goal and original limitation |
| 12–35 seconds | Three-image comparison in README | Fixed colors, highlighted particle and boundary-aware trail |
| 35–55 seconds | README verification paragraph | Original-reference comparison and unchanged results |
| 55–60 seconds | Remain on README | One-sentence conclusion |

At approximately 125–140 words per minute, the script should fit about one minute; verify with an actual rehearsal. If you run over, omit “The screenshots compare both features at the same simulation step.” Do not spend the slot installing, launching or browsing source code.

## Short answers for questions

| Question | Answer |
| --- | --- |
| What did you add beyond AS07? | Speed colors with a fixed legend, a marked particle's recent trail, periodic segmentation, controls and numerical verification. The base physics and viewer are inherited. |
| What does the color mean? | Instantaneous speed in reduced Lennard-Jones units, from blue at 0 to red at 4 or above; it is not individual temperature. |
| Why a fixed scale? | A given color should mean the same speed across frames. |
| Why does the trajectory have gaps? | A particle crossing one periodic face reappears on the other. Connecting those displayed positions would draw a false long line. |
| Why is the half-box test valid? | Samples are one integration step apart, and the program checks that true per-step displacement is below half a box length on every axis. |
| Why only one particle and 1,000 points? | It keeps the view readable and bounds storage. It shows recent history, not a diffusion measurement. |
| How was correctness checked? | Final positions and velocities of all 108 particles match an independent driver of the original integrator after 3,000 steps. All 3,001 energy samples also match. |
| Are the three screenshots different simulations? | No, they are native OpenGL captures of the same step, 1600, with different display settings. |
| Is this faster? | No speedup is claimed. The contribution is visualization and validated observation. |
| What remains before the final deadline? | Incorporate presentation feedback, finalize the repository, and submit its URL by December 11. |
