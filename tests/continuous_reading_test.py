"""Exercise actual firmware freshness/state functions without ESP32 hardware."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
source = (root/'firmware/WaterTankMonitor/WaterTankMonitor.ino').read_text()
start = source.index('bool isCurrent(')
end = source.index('const char* assessment(', start)
code = '''#include <cassert>
#include <cstring>
#include "Measurement.h"
using namespace wtm;
Reading readings[4];
bool simulatedChannel(unsigned) { return false; }
''' + source[start:end] + '''
int main() {
  const uint64_t now = 3600000;
  readings[1].hasValue = true;
  readings[1].stamp = now;
  readings[1].state = "atual";
  assert(isCurrent(1, now)); // Fresh pH remains current beyond any old session limit.
  readings[1].state = "fora_faixa_calibrada";
  assert(!isCurrent(1, now));
  readings[1].stamp = now + 5000;
  readings[1].state = "atual";
  assert(isCurrent(1, now + 5000)); // Next valid sample recovers immediately.
  assert(strcmp(readingState(1, now + 5000), "atual") == 0);
  assert(strcmp(readingState(1, now + 30000), "leitura_antiga") == 0);
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d); (p/'test.cpp').write_text(code)
 subprocess.run(['g++','-std=c++11','-I'+str(root/'firmware/WaterTankMonitor'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Freshness and automatic recovery: PASS')
