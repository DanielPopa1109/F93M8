"""Export the entire resident ELF FLASH load image, including the RAM blob LMA.
This produces a raw FBL image and engineering metadata, not an FCD .pfpkg ZIP.
Never invokes a probe, connects to a target or programs hardware.
"""
from pathlib import Path
import argparse,sys,json,hashlib,zlib,struct
sys.path.insert(0,str(Path(__file__).resolve().parent/'.runtime/analysis'))
from elftools.elf.elffile import ELFFile
p=argparse.ArgumentParser();p.add_argument('elf',type=Path);p.add_argument('--controlled',action='store_true');args=p.parse_args()
with args.elf.open('rb') as f:
 elf=ELFFile(f);loads=[s for s in elf.iter_segments() if s['p_type']=='PT_LOAD' and s['p_filesz'] and 0x08000000<=s['p_paddr']<0x08080000]
 end=max(s['p_paddr']+s['p_filesz'] for s in loads);assert end<=0x08008000
 image=bytearray([255])*(end-0x08000000);used=set()
 for s in loads:
  offset=s['p_paddr']-0x08000000;contents=s.data()
  for i,b in enumerate(contents):
   assert offset+i not in used,'overlapping load segments';used.add(offset+i);image[offset+i]=b
ecu=args.elf.stem.split('_')[0];assert ecu in ('CBM','DMU')
tag=struct.unpack_from('<8I',image,512)
assert tag[:5]==(0x46424C34,1,0x54 if ecu=='CBM' else 0x56,0x411 if ecu=='CBM' else 0x407,32768),'wrong ECU/image identity'
out=args.elf.with_suffix('.bin');out.write_bytes(image)
metadata=dict(format='F93_FBL_RAW_V1',ecu=ecu,mcu='STM32F411RET6' if ecu=='CBM' else 'STM32F407VET6',
              route='0x54' if ecu=='CBM' else '0x56',address='0x08000000',size=len(image),
              crc32=f'{zlib.crc32(image):08X}',sha256=hashlib.sha256(image).hexdigest(),residentSectors=[0,1],
              deploymentInterlock=not args.controlled,hardwareVerified=False,sourceElfSha256=hashlib.sha256(args.elf.read_bytes()).hexdigest())
args.elf.with_suffix('.manifest.json').write_text(json.dumps(metadata,indent=2)+'\n')
print(f'{ecu}: raw FBL image {len(image)} bytes; includes all RAM blob load bytes')
