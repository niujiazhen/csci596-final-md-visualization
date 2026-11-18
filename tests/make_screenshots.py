"""Pillow is needed only to convert native OpenGL PPM captures to PNG."""
from pathlib import Path
import subprocess
from PIL import Image, ImageStat
ROOT = Path(__file__).resolve().parents[1]
subprocess.run([str(ROOT/'bin/md_visualizer.exe'), '--snapshot', 'output/demo',
                '--snapshot-step', '1600', '--output', 'output/snapshot', 'examples/default.in'],
               cwd=ROOT, check=True)
for mode in ('uniform', 'speed', 'trail'):
    src = ROOT/f'output/demo_{mode}.ppm'
    with Image.open(src) as im:
        assert im.size == (1280, 720)
        assert max(ImageStat.Stat(im).stddev) > 10, 'Blank capture'
        im.save(src.with_suffix('.png'))
    src.unlink()
print('Saved three native OpenGL screenshots at the same simulation step (1600).')
