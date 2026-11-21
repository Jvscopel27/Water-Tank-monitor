#include "../diagnostics/ph/PhDiagnostic/DiagnosticStats.h"
#include <assert.h>
using namespace phdiag;
int main() {
  Window w;
  assert(w.snapshot().count==0 && w.snapshot().variationPercent()==0);
  assert(!w.snapshot().usable());
  w.add(1000,800);
  assert(w.snapshot().rawMean==1000 && !w.snapshot().constant());
  for(unsigned i=1;i<10;++i) w.add(1000,800);
  auto a=w.snapshot();assert(a.usable() && a.mvMean==800);
  w.add(1100,900);
  assert(w.snapshot().count==10 && w.snapshot().rawMean==1010);
  assert(w.snapshot().mvMean==810 && !w.snapshot().constant());
  for(unsigned i=0;i<10;++i) w.add(1100,900);
  auto b=w.snapshot();auto c=compare(a,b);
  assert(c.valid && c.significant && c.deltaMv==100);
  assert(compare(b,a).deltaMv==-100 && compare(b,a).significant);
  assert(compare(a,a).valid && !compare(a,a).significant);
  Window low,high,noise;
  for(unsigned i=0;i<10;++i) { low.add(0,142);high.add(4095,3124);noise.add(i%2?1200:1000,i%2?1000:800); }
  assert(low.snapshot().low==10 && !low.snapshot().usable());
  assert(high.snapshot().high==10 && !compare(a,high.snapshot()).valid);
  for(unsigned i=0;i<10;++i) high.add(1000,800);
  assert(high.snapshot().high==0 && high.snapshot().rawMax==1000);
  assert(high.snapshot().mvMax==800 && high.snapshot().usable());
  assert(low.snapshot().variationPercent()==0); // zero nao causa divisao por zero
  assert(!compare(a,noise.snapshot()).valid);
  Window rails;rails.add(40,142);rails.add(4055,3100);
  assert(rails.snapshot().low==1 && rails.snapshot().high==1);
  Window edge;
  for(unsigned i=0;i<10;++i) edge.add(1020,820);
  assert(!compare(a,edge.snapshot()).significant); // limiar estrito de 20 mV
  Window varying;
  for(unsigned i=0;i<10;++i) varying.add(i%2?1012:1000,i%2?810:800);
  assert(varying.snapshot().constant());
  Window other;
  for(unsigned i=0;i<10;++i) other.add(i%2?1052:1040,i%2?850:840);
  assert(compare(varying.snapshot(),other.snapshot()).thresholdMv==60);
  assert(!compare(varying.snapshot(),other.snapshot()).significant);
  w=Window{}; // mudar etapa elimina janela anterior
  assert(w.snapshot().count==0 && !w.snapshot().usable());
}
