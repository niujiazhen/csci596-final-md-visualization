"""Run with Python 3 from any directory; standard library only."""
import csv
import hashlib
import json
import math
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def run(*args, success=True):
    p = subprocess.run([str(x) for x in args], cwd=ROOT, capture_output=True, text=True)
    if success and p.returncode:
        raise RuntimeError(f'{args}: {p.stderr}\n{p.stdout}')
    if not success and p.returncode == 0:
        raise AssertionError(f'Expected failure: {args}')
    return p.stdout.strip()

def read_csv(path):
    with open(path, newline='') as f:
        return list(csv.DictReader(f))

def main():
    report = {'unit_tests': run(ROOT/'bin/test_visualization.exe'),
              'controls': run(ROOT/'bin/test_controls.exe')}
    with tempfile.TemporaryDirectory(prefix='csci596-final-') as tmp:
        tmp = Path(tmp)
        reference = tmp/'reference.csv'
        energy = tmp/'reference_energy.csv'
        run(ROOT/'bin/reference.exe', ROOT/'examples/default.in', reference, energy)
        run(ROOT/'bin/md_visualizer.exe', '--headless', '--output', ROOT/'output/verified', ROOT/'examples/default.in')
        state = ROOT/'output/verified_state.csv'
        assert state.read_bytes() == reference.read_bytes(), 'All-particle final state differs'
        rows = read_csv(ROOT/'output/verified_metrics.csv')
        energies = read_csv(energy)
        assert len(rows) == len(energies) == 3001
        for row, ref in zip(rows, energies):
            assert row['step'] == ref['step']
            assert row['total_per_atom'] == ref['total_per_atom'], 'Energy differs at a step'
            assert all(math.isfinite(float(x)) for x in row.values())
        report['all_particle_final_state'] = 'byte-identical to original integrator reference'
        report['energy_comparison'] = 'all 3001 samples identical to original reference'
        report['final_state_sha256'] = hashlib.sha256(state.read_bytes()).hexdigest()
        # Every visualization toggle combination must preserve both physics and metrics.
        for flags in [[], ['--no-color'], ['--no-trail'], ['--no-color', '--no-trail']]:
            prefix = tmp/('case'+str(len(list(tmp.glob('case*_state.csv')))))
            run(ROOT/'bin/md_visualizer.exe', '--headless', '--output', prefix, *flags, ROOT/'examples/default.in')
            assert Path(str(prefix)+'_state.csv').read_bytes() == state.read_bytes()
            assert Path(str(prefix)+'_metrics.csv').read_bytes() == (ROOT/'output/verified_metrics.csv').read_bytes()
        report['toggle_combinations'] = 'all four combinations preserve full output'
        summary = json.loads((ROOT/'output/verified_summary.json').read_text())
        assert summary['trail_samples'] == summary['max_trail_samples'] == 1000
        assert summary['boundary_breaks'] > 0
        assert summary['max_drift_over_half_box'] < 1
        report['simulation'] = summary
        # Check input failure paths, including the physical box constraint.
        for name, contents in [('malformed','bad input'), ('small_box','1 1 1\n0.8\n1\n.005\n10\n1\n'),
                               ('bad_dt','3 3 3\n0.8\n1\n0\n10\n1\n')]:
            inp = tmp/(name+'.in'); inp.write_text(contents)
            run(ROOT/'bin/md_visualizer.exe','--headless', inp, success=False)
        run(ROOT/'bin/md_visualizer.exe','--headless','--snapshot','invalid', success=False)
        run(ROOT/'bin/md_visualizer.exe','--unknown', success=False)
        report['invalid_input_checks'] = 'malformed input, small box, zero dt, incompatible and unknown flags rejected'
    (ROOT/'output/verification.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report, indent=2))

if __name__ == '__main__':
    main()
