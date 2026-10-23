"""Build real freestanding Cortex-M4 FBL ELFs/maps with pinned official HAL/CMSIS.
Requires check_cortex_sources.py and fetch_validation_dependencies.py first.
This Clang/LLD build does not replace the required CubeIDE/GCC application builds.
"""
from pathlib import Path
import os,subprocess,re,sys,json,argparse
p=argparse.ArgumentParser();p.add_argument('--activation',action='store_true',help='Compile the functional handover path; never deploys or accesses hardware');a=p.parse_args()
root=Path(__file__).resolve().parents[1];build=root/'tools/.build';vendor=root/'tools/.runtime/vendor';cc=str(root/'tools/.runtime/ziglang/zig.exe')
env=dict(os.environ,ZIG_GLOBAL_CACHE_DIR=str(build/'zig-cache'),ZIG_LOCAL_CACHE_DIR=str(build/'zig-local'))
def run(cmd):
 r=subprocess.run(cmd,env=env,capture_output=True,text=True)
 if r.returncode:print(r.stdout+r.stderr,flush=True);raise RuntimeError('command failed')
run([cc,'build-obj',str(root/'tools/.runtime/ziglang/lib/compiler_rt.zig'),'-target','thumb-freestanding-eabihf','-mcpu=cortex_m4','-O','ReleaseSmall','-ffunction-sections','-femit-bin='+str(build/'compiler_rt.o')])
results=[]
for ecu,dirname in [('CBM','CBM - Central Body Module'),('DMU','DMU - Digital Main Unit')]:
 r=root/dirname/(ecu+'_FBL');out=build/(ecu.lower()+'-fbl-arm'+('-controlled' if a.activation else ''));out.mkdir(exist_ok=True)
 includes=[r/'Core/Inc',build/'arm-include',vendor/'hal/Inc',vendor/'hal/Inc/Legacy',vendor/'device/Include',vendor/'cmsis/CMSIS/Core/Include']
 includes += [p for p in r.glob('BSW/**') if p.is_dir()]
 flags=['-target','thumb-freestanding-eabi','-mcpu=cortex_m4','-mfloat-abi=hard','-mfpu=fpv4-sp-d16','-Os','-ffreestanding','-ffunction-sections','-fdata-sections','-D'+('STM32F411xE' if ecu=='CBM' else 'STM32F407xx'),'-DUSE_HAL_DRIVER',*['-I'+str(p) for p in includes]]
 flags+=['-g','-fno-jump-tables','-fno-builtin','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage']
 if a.activation:flags+=['-DFBL_UPDATER_ACTIVATION_ALLOWED=1']
 sources=[p for p in (r/'Core/Src').glob('*.c') if p.name not in ['syscalls.c','sysmem.c']]+list((r/'BSW').rglob('*.c'))+list((r/'Core/Startup').glob('*.s'))
 drivers=re.findall(r'<name>Drivers/STM32F4xx_HAL_Driver/([^<]+)</name>',(r/'.project').read_text())
 sources += [vendor/'hal/Src'/name for name in drivers]
 objects=[]
 try:
  for i,src in enumerate(sources):
   obj=out/(str(i)+'-'+src.stem+'.o');run([cc,'cc',*flags,'-c',str(src),'-o',str(obj)]);objects.append(obj)
  elf=out/(ecu+'_FBL.elf');mapfile=out/(ecu+'_FBL.map');ld=next(r.glob('*_FLASH.ld'))
  run([cc,'cc',*flags,'-nostdlib',*map(str,objects),str(build/'compiler_rt.o'),'-Wl,-T,'+str(ld),'-Wl,-e,Reset_Handler','-Wl,--gc-sections','-o',str(elf)])
  (out/'objects.json').write_text(json.dumps(list(map(str,objects))))
  run([sys.executable,str(root/'tools/verify_ram_updater.py'),str(elf)])
  run([sys.executable,str(root/'tools/package_fbl_image.py'),str(elf),*(['--controlled'] if a.activation else [])])
  print(ecu,'FBL ELF linked and RAM closure verified',elf,flush=True);results.append(dict(ecu=ecu,result='linked',activation=bool(a.activation),elf=str(elf),map=str(mapfile)))
 except RuntimeError:results.append(dict(ecu=ecu,result='failed'))
(build/('fbl-link-results-controlled.json' if a.activation else 'fbl-link-results.json')).write_text(json.dumps(results,indent=2))
sys.exit(int(any(x['result']!='linked' for x in results)))
