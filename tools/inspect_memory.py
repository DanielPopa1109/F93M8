from pathlib import Path
import struct,json,re,sys
root=Path(__file__).resolve().parents[1];build=root/'tools/.build'
def elf_map(path):
 data=path.read_bytes();h=struct.unpack_from('<16sHHIIIIIHHHHHH',data,0);assert h[0][:5]==b'\x7fELF\x01'
 shoff,shsize,shnum,shstr=h[6],h[11],h[12],h[13]
 sections=[struct.unpack_from('<IIIIIIIIII',data,shoff+i*shsize) for i in range(shnum)]
 names=data[sections[shstr][4]:sections[shstr][4]+sections[shstr][5]]
 def name(blob,off):return blob[off:blob.index(0,off)].decode()
 rows=[];symbols={}
 for sec in sections:
  n=name(names,sec[0]);rows.append(dict(section=n,address=sec[3],size=sec[5],allocated=bool(sec[2]&2)))
  if sec[1]==2:
   strings=sections[sec[6]];table=data[strings[4]:strings[4]+strings[5]]
   for off in range(sec[4],sec[4]+sec[5],sec[9]):
    s=struct.unpack_from('<IIIBBH',data,off);symbols[name(table,s[0])]=s[1]
 loads=[]
 for i in range(h[10]):
  ph=struct.unpack_from('<IIIIIIII',data,h[5]+i*h[9])
  if ph[0]==1:loads.append(dict(vaddr=ph[2],load=ph[3],fileBytes=ph[4],memoryBytes=ph[5]))
 return rows,symbols,loads
results=[]
for ecu,limit in [('CBM',0x08008000),('DMU',0x08008000)]:
 p=build/(ecu.lower()+'-fbl-arm')/(ecu+'_FBL.elf')
 if not p.exists():continue
 rows,symbols,loads=elf_map(p);flashEnd=max(x['load']+x['fileBytes'] for x in loads if 0x08000000<=x['load']<0x08080000)
 assert flashEnd<=limit
 text='ELF-derived section map (not a GNU linker map)\n'+''.join(f"{x['section']:24} 0x{x['address']:08X} {x['size']:8} bytes\n" for x in rows if x['allocated'])
 text+='\n'+''.join(f"{key} = 0x{symbols[key]:08X}\n" for key in ['_estack','_sdata','_edata','_sbss','_ebss','_end'] if key in symbols)
 if not p.with_suffix('.map').exists():p.with_suffix('.map').write_text(text)
 r=dict(ecu=ecu,flashBytes=flashEnd-0x08000000,flashLimit=limit-0x08000000,stackTop=symbols.get('_estack'),sections=[x for x in rows if x['allocated']]);results.append(r);print(ecu,'flash load bytes',r['flashBytes'],'/',r['flashLimit'],'bss',next(x['size'] for x in rows if x['section']=='.bss'))
# Verify flash/RAM MEMORY regions and explicit APP metadata/boot-mailbox reservations.
for p in root.glob('*/*_*/STM32*_FLASH.ld'):
 regions=[]
 for n,a,z in re.findall(r'(\w+)\s*\([^)]*\)\s*:\s*ORIGIN\s*=\s*(0x[0-9A-Fa-f]+),\s*LENGTH\s*=\s*(\d+[Kk]?)',p.read_text()):
  size=int(z[:-1])*1024 if z[-1:].lower()=='k' else int(z);regions.append((n,int(a,16),int(a,16)+size))
 for i,(n,a,b) in enumerate(regions):
  for m,c,d in regions[i+1:]:assert max(a,c)>=min(b,d),(str(p),n,m)
 if p.parent.name.endswith('APPL'):
  flash=next(x for x in regions if x[0]=='FLASH');expected=0x0807FC00 if p.parent.name.startswith('CBM') else 0x0805FC00;assert flash[2]==expected
  ncr=next(x for x in regions if x[0]=='NCR');assert ncr[2]<=(0x2001FFF0 if p.parent.name.startswith('CBM') else 0x1000FFF0)
 print(p.parent.name,'MEMORY/reservation PASS')
(build/'memory-results.json').write_text(json.dumps(results,indent=2))
