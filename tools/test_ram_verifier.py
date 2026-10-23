"""Relink both controlled FBLs with an intentionally forbidden SRAM->FLASH call.
The negative artifacts are validation fixtures only; they are never packaged.
"""
from pathlib import Path
import os,subprocess,json,sys
root=Path(__file__).resolve().parents[1];build=root/'tools/.build';cc=root/'tools/.runtime/ziglang/zig.exe'
env=dict(os.environ,ZIG_GLOBAL_CACHE_DIR=str(build/'zig-cache'),ZIG_LOCAL_CACHE_DIR=str(build/'zig-local'))
flags=['-target','thumb-freestanding-eabi','-mcpu=cortex_m4','-mfloat-abi=hard','-mfpu=fpv4-sp-d16','-g','-ffreestanding']
for ecu,dirname in [('CBM','CBM - Central Body Module'),('DMU','DMU - Digital Main Unit')]:
 base=build/(ecu.lower()+'-fbl-arm-controlled');out=build/(ecu.lower()+'-ram-negative');out.mkdir(exist_ok=True)
 source=out/'bad.c';source.write_text('''extern void Fbl_BoardReset(void);
__attribute__((section(".ram_blob_text"),noinline)) void RamBadDependencyProbe(void){Fbl_BoardReset();}
typedef void (*P)(void);
__attribute__((section(".ram_blob_vectors"),used)) P forbidden_reference=RamBadDependencyProbe;
''')
 obj=out/'bad.o';subprocess.run([str(cc),'cc',*flags,'-c',str(source),'-o',str(obj)],env=env,check=True)
 objects=json.loads((base/'objects.json').read_text())+[str(obj)];(out/'objects.json').write_text(json.dumps(objects))
 elf=out/(ecu+'_bad.elf');ld=next((root/dirname/(ecu+'_FBL')).glob('*_FLASH.ld'))
 subprocess.run([str(cc),'cc',*flags,'-nostdlib',*objects,str(build/'compiler_rt.o'),'-Wl,-T,'+str(ld),'-Wl,-e,Reset_Handler','-Wl,--gc-sections','-o',str(elf)],env=env,check=True)
 result=subprocess.run([sys.executable,str(root/'tools/verify_ram_updater.py'),str(elf)],capture_output=True,text=True)
 assert result.returncode and 'external RAM dependency Fbl_BoardReset' in result.stderr,result.stdout+result.stderr
 assert 'branch escapes SRAM' in result.stderr or 'unresolved indirect jump' in result.stderr,result.stderr
 print(ecu,'PASS: checker rejected a real linked SRAM-to-resident-Flash reset dependency')
