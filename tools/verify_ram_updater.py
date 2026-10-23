"""Fail a Cortex-M ELF with RAM references outside its dedicated SRAM closure.

Run on the final ELF and its original input objects, after every relink. Uses pinned analysis
libraries in tools/.runtime/analysis (pyelftools 0.32 and capstone 5.0.3).
Checks relocations, branch instructions, literal pools, vectors and memory maps.
Cannot prove voltage, bus timing, peripheral wiring, or arbitrary dynamic addresses.
"""
from pathlib import Path
import sys,json,hashlib
sys.path.insert(0,str(Path(__file__).resolve().parent/'.runtime/analysis'))
from elftools.elf.elffile import ELFFile
from capstone import Cs,CS_ARCH_ARM,CS_MODE_THUMB,CS_MODE_MCLASS,CS_GRP_JUMP,CS_GRP_CALL
from capstone.arm import ARM_OP_IMM,ARM_OP_REG,ARM_OP_MEM,ARM_REG_PC,ARM_REG_LR

def verify(path):
 errors=[]
 with path.open('rb') as f:
  elf=ELFFile(f);sections=list(elf.iter_sections());tab=elf.get_section_by_name('.symtab')
  syms={s.name:s['st_value'] for s in tab.iter_symbols()}
  blob=elf.get_section_by_name('.ram_blob');assert blob,'missing RAM blob'
  start=blob['sh_addr'];end=start+blob['sh_size'];data=blob.data()
  bss=elf.get_section_by_name('.ram_blob_bss');stack=elf.get_section_by_name('.ram_blob_stack')
  def fail(message):errors.append(message)
  def inside(v):return start<=v<end
  def owned(v):return inside(v) or bss['sh_addr']<=v<bss['sh_addr']+bss['sh_size'] or stack['sh_addr']<=v<=stack['sh_addr']+stack['sh_size']
  if start!=0x20000000 or start&511:fail('bad vector origin/alignment')
  if end>bss['sh_addr'] or bss['sh_addr']+bss['sh_size']>0x2000F000 or stack['sh_addr']!=0x2000F000 or stack['sh_size']!=4096:fail('RAM partition overlap')
  if syms['__ram_blob_load_end']-syms['__ram_blob_load_start']!=len(data):fail('LMA/VMA size mismatch')
  if syms['__ram_blob_load_end']>0x08008000:fail('resident FLASH overflow')
  if elf['e_entry']!=syms['Reset_Handler']:fail('ELF entry does not match Reset_Handler')
  for i in range(112):
   value=int.from_bytes(data[4*i:4*i+4],'little')
   if i==0:
    if value!=0x20010000:fail('invalid RAM MSP')
   elif not value&1 or not inside(value&~1) or value&~1<start+448:fail(f'invalid RAM vector {i}: {value:#x}')
  relocs=0
  # Zig's linker frontend does not expose --emit-relocs. Inspect the exact
  # pre-link objects instead, then check their final branches/literals below.
  objects=json.loads(path.with_name('objects.json').read_text())
  for objectPath in objects:
   with open(objectPath,'rb') as of:
    obj=ELFFile(of);osections=list(obj.iter_sections())
    for section in osections:
     if section['sh_type']!='SHT_REL' or not osections[section['sh_info']].name.startswith('.ram_blob'):continue
     symbols=osections[section['sh_link']]
     for relocation in section.iter_relocations():
      relocs+=1;s=symbols.get_symbol(relocation['r_info_sym']);index=s['st_shndx']
      if isinstance(index,int):
       if not osections[index].name.startswith('.ram_blob'):fail(f'{Path(objectPath).name}: RAM relocation to {s.name or osections[index].name}')
      elif index=='SHN_UNDEF':
       if s.name not in syms or not owned(syms[s.name]):fail(f'{Path(objectPath).name}: external RAM dependency {s.name}')
      else:fail(f'{Path(objectPath).name}: unreviewed absolute RAM dependency {s.name}')
  if not relocs:fail('missing input object relocation evidence')
  md=Cs(CS_ARCH_ARM,CS_MODE_THUMB|CS_MODE_MCLASS);md.detail=True
  listing=[];instructions=0;functions=[];frames={};edges={};functionAt={}
  for symbol in tab.iter_symbols():
   address=symbol['st_value']&~1
   if symbol['st_info']['type']!='STT_FUNC' or not inside(address) or not symbol['st_size']:continue
   functions.append(symbol.name)
   functionAt[address]=symbol.name
   code=data[address-start:address-start+symbol['st_size']]
   decoded=list(md.disasm(code,address))
   if sum(i.size for i in decoded)!=len(code):fail(f'incomplete disassembly: {symbol.name}')
   listing.append(f'\n{symbol.name}:')
   frame=0;edges[address]=set()
   for ins in decoded:
    instructions+=1;listing.append(f'{ins.address:08x}  {ins.mnemonic:9} {ins.op_str}')
    # These sources use fixed local arrays only. Conservatively add every
    # stack decrement in a function (including alternate prologues).
    if ins.mnemonic in ('push','push.w'):frame+=4*len(ins.operands)
    if ins.mnemonic=='vpush':frame+=8*len(ins.operands)
    if ins.mnemonic.startswith('sub') and ins.op_str.startswith('sp,'):
     if ins.operands[-1].type!=ARM_OP_IMM:fail(f'dynamic stack allocation at {ins.address:#x}')
     else:frame+=ins.operands[-1].imm
    if ins.group(CS_GRP_CALL) or ins.group(CS_GRP_JUMP):
     if ins.mnemonic in ('cbz','cbnz'):targets=[ins.operands[-1]]
     else:targets=ins.operands
     for op in targets:
      if op.type==ARM_OP_IMM:
       if not inside(op.imm&~1):fail(f'branch escapes SRAM at {ins.address:#x}: {ins.op_str}')
       if (op.imm&~1)!=address:edges[address].add(op.imm&~1)
     if ins.mnemonic.startswith('bl') and any(op.type==ARM_OP_REG for op in targets):fail(f'unresolved indirect call at {ins.address:#x}')
     if ins.mnemonic=='bx' and ins.operands[0].reg!=ARM_REG_LR:fail(f'unresolved indirect jump at {ins.address:#x}')
    for op in ins.operands:
     if op.type==ARM_OP_MEM and op.mem.base==ARM_REG_PC:
      literal=((ins.address+4)&~3)+op.mem.disp
      if not inside(literal) or not inside(literal+3):fail(f'literal outside blob at {ins.address:#x}')
    if ins.mnemonic in ('tbb','tbh'):fail(f'jump table requires explicit proof at {ins.address:#x}')
   frames[address]=frame
  def stackDepth(address,active):
   if address in active:raise AssertionError('recursive updater call graph')
   return frames[address]+max([stackDepth(target,active|{address}) for target in edges[address] if target in frames]+[0])
  stackBound=max(stackDepth(address,set()) for address in frames)+104
  if stackBound>4096:fail(f'conservative stack bound {stackBound} exceeds reservation')
  # All allocated flash load images must fit resident sectors; normal RAM cannot overlap updater.
  flashEnd=max(seg['p_paddr']+seg['p_filesz'] for seg in elf.iter_segments() if seg['p_type']=='PT_LOAD' and 0x08000000<=seg['p_paddr']<0x08080000)
  if flashEnd>0x08008000:fail('combined resident/blob image exceeds 32 KiB')
  for s in sections:
   if s['sh_flags']&2 and s['sh_size'] and s.name not in ('.ram_blob','.ram_blob_bss','.ram_blob_stack') and 0x20000000<=s['sh_addr']<0x20010000:fail(f'resident runtime overlaps updater: {s.name}')
  if not functions:fail('no updater functions')
  mapText='ELF-derived section/symbol map; Zig frontend does not expose a GNU linker map.\n'
  mapText+=''.join(f'{s.name:26} {s["sh_addr"]:08x} {s["sh_size"]:8} bytes\n' for s in sections if s['sh_flags']&2)
  mapText+='\n'+''.join(f'{value:08x} {name}\n' for name,value in sorted(syms.items(),key=lambda x:x[1]) if name)
  path.with_suffix('.map').write_text(mapText)
 result=dict(elf=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest(),flashBytes=flashEnd-0x08000000,
             blobBytes=len(data),blobBssBytes=bss['sh_size'],stackBytes=stack['sh_size'],relocations=relocs,
             instructions=instructions,stackBoundBytes=stackBound,functions=functions,errors=errors)
 path.with_suffix('.ram.disasm').write_text('\n'.join(listing)+'\n')
 path.with_suffix('.ram.json').write_text(json.dumps(result,indent=2))
 if errors:raise AssertionError('\n'.join(errors))
 print(f'{path.name}: PASS SRAM closure; flash={result["flashBytes"]}/32768 blob={len(data)} state={bss["sh_size"]} stack=4096 relocs={relocs} instructions={instructions}')
 return result

if __name__=='__main__':
 for item in sys.argv[1:]:verify(Path(item))
