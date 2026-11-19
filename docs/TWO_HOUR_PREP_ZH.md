# 11 月 30 日汇报：只用两小时的准备方案

你不需要现在练习后再向工具汇报。下面是一份可在 11 月 29 日前自行完成的 120 分钟清单。程序、报告、截图、仓库和讲稿已经准备；这两小时用于让你能独立讲清楚并回答基本问题。

## 已经替你完成的事

- 老师已通过邮件同意选题；11 月 30 日的报名在第 20 行，姓名后已补上仓库 URL。
- 程序已实现：按速率着色、固定图例、单粒子轨迹、跨周期边界时断线，以及对应的控制和数据记录。
- 当前版本已重新编译和验证；完整报告、真实效果图、英文讲稿和问题答案已放入仓库。
- GitHub 仓库地址：[csci596-final-md-visualization](https://github.com/niujiazhen/csci596-final-md-visualization)。公开访问状态以 `PRE_PRESENTATION_STATUS.md` 的实际核验记录为准。
- 本地可运行版本和完整备份位于课程目录下的 `FinalProject` 文件夹和 `CSCI596_FinalProject.zip`。

## 120 分钟安排

| 累计时间 | 用时 | 你只需要做什么 | 完成标志 |
| --- | --- | --- | --- |
| 0–20 分钟 | 20 分钟 | 阅读本页“必须讲清楚的五点”和 `START_HERE_ZH.md` 的基础解释 | 不看稿也能用中文解释项目目标、颜色和轨迹 |
| 20–35 分钟 | 15 分钟 | 双击 `run_demo.cmd`，用空格、C、T 和方向键操作一次 | 亲眼理解两项功能；按 Esc 正常退出 |
| 35–55 分钟 | 20 分钟 | 按本页代码导航只读四处关键逻辑 | 能指出速率计算、颜色映射、边界断线和不改动力学的依据 |
| 55–80 分钟 | 25 分钟 | 读 `PRESENTATION.md` 的英文稿，计时两遍，超时则用下方简短备用稿 | 能在约 50–60 秒内讲完；不必逐字背诵 |
| 80–100 分钟 | 20 分钟 | 练习本页五个必答问题；有余力再看英文稿末尾的问题表 | 每题能用一两句话回答 |
| 100–115 分钟 | 15 分钟 | 真正打开 GitHub，边滚动 README 边讲一遍 | 按“问题→图片→验证→结论”的顺序结束，不临场找文件 |
| 115–120 分钟 | 5 分钟 | 保存仓库和讲稿链接，检查离线备份、充电与网络 | 当天能直接进入汇报状态 |

## 必须讲清楚的五点

1. **问题是什么？** 原作业把粒子画成同一种颜色，只显示当前位置，不容易看出哪个粒子快、某个粒子刚才经过哪里。
2. **你增加了什么？** 按瞬时速率着色，使用固定 0–4 的约化速率刻度；跟踪编号 0 的粒子，显示最近 1,000 个模拟步的位置，并标出这个粒子。
3. **为什么轨迹会断？** 盒子是周期边界。球从右边出去会从左边回来。如果位置从 9.9 变成 0.1，直接连线会画出穿过盒子的假路径，因此在边界处断开。
4. **怎么验证？** 使用保存下来的原始模拟代码作为独立对照。运行 3,000 步后，全部 108 个粒子的位置和速度输出一致，包括初始状态的 3,001 个能量采样也一致。
5. **贡献边界是什么？** 力学模型、积分方法和基础显示程序沿用课程及作业代码。本项目增加可视化与验证，没有声称模拟变快、发现新物理规律或从零重写所有代码。

## 只读这四处代码

你不用从头通读整个项目。

| 文件与函数 | 要理解的内容 |
| --- | --- |
| `src/visualization.h`：`particle_speed`、`speed_color` | 速度有三个分量；先计算 `sqrt(vx²+vy²+vz²)`，再按固定范围映射为颜色。高于 4 使用红色。 |
| `src/visualization.h`：`trail_add`、`trail_connect` | 环形缓冲区只保留最近 1,000 个点；相邻点跨越周期边界时不连线。 |
| `src/md_visualizer.c`：`observe_step`、`properties` | 每个模拟步采样一次，独立于显示帧率。观测只读取位置和速度；用局部变量计算统计量。程序检查真实步长位移小于半盒长，以支持边界判断。 |
| `original/md.c`：`SingleStep` | 原有速度-Verlet方法负责推进运动，仍由原始代码执行。无需先掌握完整推导。 |

## 五个必答问题：直接练这些答案

**What does the color mean?**

It shows instantaneous speed in reduced Lennard-Jones units. It is not the temperature of an individual particle.

**Why use a fixed color scale?**

The same color should represent the same speed across frames. Speeds above four are clipped to red.

**Why are there gaps in the trail?**

They occur at periodic boundary crossings. Splitting the trail avoids drawing a false line through the box.

**How did you check correctness?**

I compared the extended program with the original simulation. After three thousand steps, all particle positions and velocities match, and all energy samples match too.

**What is new compared with Assignment 7?**

I added speed colors with a fixed legend, a highlighted particle trail, periodic boundary handling, controls, and verification. The base physics and viewer are inherited.

如果不理解老师的追问，可以说 “Could you please rephrase the question?”。如果某件事没有测过，就明确说 “I have not measured that yet.”，不要临场编造结果。

## 如果正常英文稿仍超过一分钟

使用下面的简短备用稿。它与现有实现一致，保留了问题、方法、验证和结论：

Hello, I'm Jiazhen Niu. My project adds speed colors and a particle trail to our molecular dynamics viewer.

Blue means slower motion, and red means faster motion.

The white trail shows one particle's recent path. At periodic boundaries, I split the trail to avoid false lines across the box.

I checked the extended program against the original simulation. After three thousand steps, all particle positions and velocities match.

The result is a clearer view of motion without changing the simulation.

## 汇报当天怎么展示

只使用 GitHub 仓库；不需要制作幻灯片，也不用现场编译或运行程序。

1. 上课前打开 README，浏览器缩放到看得清标题和效果图的大小。
2. 先说原来不容易观察什么，再展示三张同一步的对照图。
3. 指一下固定速度图例和白色轨迹，解释边界断线。
4. 展示 README 的验证段落，用一句话总结，控制在一分钟左右。
5. 将老师的反馈记下来，12 月 11 日最终提交前再修改。

## 两小时内可以跳过的事

- 不用补完整门课，不用学习 MPI、CUDA 或推导全部 Lennard-Jones 公式。
- 不用重做已有作业、增加新功能、录制视频或设计幻灯片。
- 不用现在向 Brightspace 提交最终项目；最终仓库链接截止 12 月 11 日，具体时刻查看 Brightspace。
- 不必把两次计时或答案发回本聊天；自行确认理解与时长即可。如有具体难点，再来调整。

最后五分钟确认：能解释五个核心点；讲稿不超时；知道 GitHub 怎么滚动；电脑、网络和备份可用。以上是留给你本人的准备，不能把已经写好的讲稿当作已经练习过。
