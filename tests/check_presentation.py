"""Check the browsable repository materials without network access."""
import csv
import hashlib
import json
from pathlib import Path
import re
import struct
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]

def main():
    checked_links = 0
    for md in [ROOT/'README.md', *sorted((ROOT/'docs').glob('*.md'))]:
        text = md.read_text(encoding='utf-8')
        assert not re.search(r'USC\s+ID\s*[:=]\s*\d{10}', text, re.I), f'Student ID in {md.name}'
        for target in re.findall(r'\]\(([^)]+)\)', text):
            if '://' in target or target.startswith(('#', 'mailto:')):
                continue
            path = (md.parent/unquote(target.split('#')[0])).resolve()
            assert path.is_relative_to(ROOT), f'Link leaves project: {target}'
            assert path.is_file(), f'Broken link in {md.name}: {target}'
            checked_links += 1
    script = (ROOT/'docs/PRESENTATION.md').read_text(encoding='utf-8')
    script = script.split('## Spoken script')[1].split('## Repository navigation')[0]
    words = len(script.split())
    assert 100 <= words <= 125, f'Unexpected speech length: {words}'
    images = {}
    for mode in ('uniform','speed','trail'):
        raw = (ROOT/f'output/demo_{mode}.png').read_bytes()
        assert raw[:8] == b'\x89PNG\r\n\x1a\n'
        width, height = struct.unpack('>II', raw[16:24])
        assert (width,height) == (1280,720)
        images[mode] = hashlib.sha256(raw).hexdigest()
    assert len(set(images.values())) == 3, 'Comparison images are not distinct'
    def rows(path):
        with path.open(newline='') as f:
            return list(csv.DictReader(f))
    snapshot = rows(ROOT/'output/snapshot_metrics.csv')
    full = rows(ROOT/'output/verified_metrics.csv')
    assert len(full) == 3001 and len(snapshot) == 1601
    assert snapshot == full[:1601], 'Native screenshot run differs from headless run'
    verification = json.loads((ROOT/'output/verification.json').read_text())
    assert verification['simulation']['steps'] == 3000
    assert verification['simulation']['atoms'] == 108
    readme = (ROOT/'README.md').read_text(encoding='utf-8')
    assert 'Presentation: November 30, 2026' in readme
    assert 'Instructor approval of the topic and classroom presentation are still pending' not in readme
    result = {
        'checked_on_client_date': '2026-11-19',
        'relative_links_checked': checked_links,
        'speech_word_count': words,
        'estimated_seconds_at_125_wpm': round(words/125*60,1),
        'actual_timed_rehearsal': 'student must complete; not inferred from word count',
        'screenshot_dimensions': [1280,720],
        'screenshot_sha256': images,
        'snapshot_vs_full_run': 'samples 0 through 1600 match',
        'remote_repository': 'checked separately; local materials do not prove upload',
    }
    (ROOT/'output/presentation_checks.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))

if __name__ == '__main__':
    main()
