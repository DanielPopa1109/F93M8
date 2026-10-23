"""Static checks complement executed host tests and Cortex-M compilation."""
from pathlib import Path
import json,xml.etree.ElementTree as ET,re,subprocess
root=Path(__file__).resolve().parents[1];rows=[]
for ecu,dirname in [('CBM','CBM - Central Body Module'),('DMU','DMU - Digital Main Unit')]:
 for kind in ['APPL','FBL']:
  r=root/dirname/(ecu+'_'+kind);tree=ET.parse(r/'.cproject');ET.parse(r/'.project')
  for cfg in tree.findall('.//configuration'):
   entries=cfg.find('sourceEntries')
   if entries is None:continue
   assert entries.findall('entry'),r
  objects=(r/'Debug/objects.list').read_text();make=(r/'Debug/makefile').read_text()
  sources=[]
  if kind=='FBL':sources=list((r/'BSW').rglob('*.c'))+[p for p in (r/'Core/Src').glob('Fbl*.c')]+[r/'Core/Src/ram_blob.c']
  else:
   for part in ['Mem','Diag/Dcm','Diag/Dem','Com/Can','Com/CanIf','Com/CanTp','Com/PduR','Com/Com','Com/CanNm','Com/CanSM','Com/ComM']:
    sources+=list((r/'BSW'/part).glob('*.c'))
   sources+=list((r/'BSW/Mem/MemIf').glob('*.c'))+list((r/'BSW/Mem/Crc').glob('*.c'))+list((r/'BSW/Mem/Fls').glob('*.c'))
  for p in sources:
   rel=p.relative_to(r);name=rel.as_posix();sub=(r/'Debug'/rel.parent/'subdir.mk').read_text()
   assert '../'+name in sub,(str(p),'source rule missing')
   assert './'+rel.with_suffix('.o').as_posix() in objects,(str(p),'object missing')
   assert rel.parent.as_posix()+'/subdir.mk' in make,(str(p),'make include missing')
   assert 'Common/' not in p.read_text(),p
  rows.append(dict(project=r.name,sourceRuleObjectCount=len(sources),xml='PASS'))
  print(r.name,'XML/source-rule-object PASS:',len(sources))
# Modified source dependencies stay in their own project. Relative includes may not escape it.
for ecu,dirname in [('CBM','CBM - Central Body Module'),('DMU','DMU - Digital Main Unit')]:
 for kind in ['APPL','FBL']:
  r=root/dirname/(ecu+'_'+kind)
  for p in list((r/'BSW').rglob('*.c'))+list((r/'Core/Src').glob('Fbl*.c')):
   for inc in re.findall(r'^#include\s+"([^"\n]+)"',p.read_text(errors='replace'),re.M):
    if '/' not in inc:continue
    resolved=(p.parent/inc).resolve()
    assert resolved.is_relative_to(r.resolve()),(p,inc)
print('Independent firmware source boundaries PASS')
(root/'tools/.build/static-results.json').write_text(json.dumps(rows,indent=2))
