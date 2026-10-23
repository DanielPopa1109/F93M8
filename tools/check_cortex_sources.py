"""Compile changed BSW and FBL board sources for Cortex-M4 against official STM32 headers.
This is object compilation, not an application firmware link or hardware verification.
"""
from pathlib import Path
import os,subprocess,json,sys
root=Path(__file__).resolve().parents[1];build=root/'tools/.build';vendor=root/'tools/.runtime/vendor'
env=dict(os.environ,ZIG_GLOBAL_CACHE_DIR=str(build/'zig-cache'),ZIG_LOCAL_CACHE_DIR=str(build/'zig-local'))
compiler=str(root/'tools/.runtime/ziglang/zig.exe');results=[]
for ecu,dirname in [('CBM','CBM - Central Body Module'),('DMU','DMU - Digital Main Unit')]:
 for kind in ['APPL','FBL']:
  r=root/dirname/(ecu+'_'+kind);out=build/(ecu.lower()+'-'+kind.lower()+'-arm');out.mkdir(exist_ok=True)
  includes=[r/'Core/Inc',build/'arm-include',vendor/'hal/Inc',vendor/'hal/Inc/Legacy',vendor/'device/Include',vendor/'cmsis/CMSIS/Core/Include',vendor/'cmsis/CMSIS/RTOS2/Include']
  includes += [p for p in r.glob('BSW/**') if p.is_dir() and 'Debug' not in p.parts]
  includes += [p for p in r.glob('APPL/**') if p.is_dir()]
  sources=[]
  if kind=='FBL':sources=[p for p in (r/'Core/Src').glob('*.c') if p.name not in ['syscalls.c','sysmem.c']]+list((r/'BSW').rglob('*.c'))
  else:
   for base in ['BSW/Mem','BSW/Diag/Dem','BSW/Diag/Dcm','BSW/Com/Can','BSW/Com/CanIf','BSW/Com/CanTp','BSW/Com/PduR','BSW/Com/Com','BSW/Com/CanNm','BSW/Com/ComM','BSW/Com/CanSM']:
    sources+=list((r/base).glob('*.c'))
   if ecu=='DMU':sources+=list((r/'BSW/Mem/Nvm').glob('*.c'))+[r/'BSW/Com/CanH/CanH.c',r/'Core/Src/can.c',r/'BSW/Sys/EcuM/EcuM.c']
   else:sources += [r/'BSW/Com/CanSpi/CanSpi.c',r/'BSW/Sys/EcuM/EcuM.c']
  errors=[]
  for source in sources:
   rel=str(source.relative_to(r));obj=out/(rel.replace('/','_').replace(chr(92),'_')+'.o')
   cmd=[compiler,'cc','-target','thumb-freestanding-eabi','-mcpu=cortex_m4','-mfloat-abi=hard','-mfpu=fpv4-sp-d16','-std=gnu11','-Os','-ffreestanding','-ffunction-sections','-fdata-sections','-D'+('STM32F411xE' if ecu=='CBM' else 'STM32F407xx'),'-DUSE_HAL_DRIVER',*['-I'+str(p) for p in includes],'-c',str(source),'-o',str(obj)]
   result=subprocess.run(cmd,env=env,capture_output=True,text=True)
   if result.returncode:errors.append(dict(file=rel,output=result.stdout+result.stderr));print(ecu,kind,rel,result.stderr,flush=True)
  results.append(dict(ecu=ecu,project=kind,compiled=len(sources)-len(errors),total=len(sources),errors=errors))
  print(ecu,kind,len(sources)-len(errors),'/',len(sources),'Cortex-M objects',flush=True)
(build/'cortex-results.json').write_text(json.dumps(results,indent=2))
sys.exit(int(any(x['errors'] for x in results)))
