"""Compile and execute independent ECU C harnesses; compiler may be supplied with --cc."""
from pathlib import Path
import argparse,os,subprocess,sys,json
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--cc',default=str(root/'tools/.runtime/ziglang/zig.exe'));a=p.parse_args()
build=root/'tools/.build';build.mkdir(exist_ok=True)
env=dict(os.environ,ZIG_GLOBAL_CACHE_DIR=str(build/'zig-cache'),ZIG_LOCAL_CACHE_DIR=str(build/'zig-local'))
results=[]
for ecu,dirname in [('CBM','CBM - Central Body Module/CBM_APPL'),('DMU','DMU - Digital Main Unit/DMU_APPL')]:
 r=root/dirname;m=r/'BSW/Mem';d=r/'BSW/Diag/Dem'
 sources=[r/'tests/test_persistence.c',m/'NvM_Engine.c',m/'MemIf/MemIf.c',m/'Crc/Crc.c',d/'Dem_Core.c',d/'Dem_Cfg.c']
 exe=build/(ecu.lower()+'-persistence.exe')
 cmd=[a.cc,'cc','-target','x86_64-windows-gnu','-mcpu=baseline','-std=c11','-Wall','-Wextra','-Werror','-DDEM_HOST_TEST','-I'+str(r/'BSW'),'-I'+str(m),'-I'+str(d),*map(str,sources),'-o',str(exe)]
 print('BUILD',ecu,'persistence',flush=True)
 b=subprocess.run(cmd,env=env,capture_output=True,text=True);print(b.stdout+b.stderr,end='',flush=True)
 if b.returncode: results.append(dict(ecu=ecu,test='persistence',build=b.returncode));continue
 t=subprocess.run([str(exe)],capture_output=True,text=True);print(t.stdout+t.stderr,end='',flush=True)
 results.append(dict(ecu=ecu,test='persistence',build=0,run=t.returncode))
for ecu,dirname in [('CBM','CBM - Central Body Module/CBM_FBL'),('DMU','DMU - Digital Main Unit/DMU_FBL')]:
 r=root/dirname;exe=build/(ecu.lower()+'-programming.exe')
 sources=[r/'tests/test_programming.c',r/'Core/Src/Fbl.c',r/'Core/Src/Fbl_Updater.c',r/'BSW/Crc/Crc.c']
 if ecu=='CBM':sources += [r/'BSW/Mem/MemIf/MemIf.c',r/'BSW/Mem/NvM_Cfg.c']
 cmd=[a.cc,'cc','-target','x86_64-windows-gnu','-mcpu=baseline','-std=c11','-Wall','-Wextra','-Werror','-I'+str(r/'Core/Inc'),'-I'+str(r/'BSW/Crc'),*map(str,sources),'-o',str(exe)]
 print('BUILD',ecu,'programming',flush=True)
 b=subprocess.run(cmd,env=env,capture_output=True,text=True);print(b.stdout+b.stderr,end='',flush=True)
 if b.returncode: results.append(dict(ecu=ecu,test='programming',build=b.returncode));continue
 t=subprocess.run([str(exe)],capture_output=True,text=True);print(t.stdout+t.stderr,end='',flush=True)
 results.append(dict(ecu=ecu,test='programming',build=0,run=t.returncode))
for ecu,dirname in [('CBM','CBM - Central Body Module/CBM_APPL'),('DMU','DMU - Digital Main Unit/DMU_APPL')]:
 r=root/dirname;m=r/'BSW/Mem';d=r/'BSW/Diag/Dem';c=r/'BSW/Com'
 sources=[r/'tests/test_diagnostics.c',m/'NvM_Engine.c',m/'MemIf/MemIf.c',m/'Crc/Crc.c',d/'Dem_Core.c',d/'Dem_Cfg.c',r/'BSW/Diag/Dcm/Dcm.c',r/'BSW/Diag/Dcm/Dcm_Cfg.c']
 for module in ['Can','CanIf','CanTp','PduR','Com','CanNm','ComM','CanSM']:sources.append(c/module/(module+'.c'))
 sources.append(c/'Com/Com_Cfg.c')
 includes=[r/'BSW',m,d,r/'BSW/Diag/Dcm',*[c/x for x in ['Can','CanIf','CanTp','PduR','Com','CanNm','ComM','CanSM']]]
 exe=build/(ecu.lower()+'-diagnostics.exe')
 cmd=[a.cc,'cc','-target','x86_64-windows-gnu','-mcpu=baseline','-std=c11','-Wall','-Wextra','-Werror','-DDEM_HOST_TEST','-D'+ecu+'_HOST_TEST',*['-I'+str(x) for x in includes],*map(str,sources),'-o',str(exe)]
 print('BUILD',ecu,'diagnostics',flush=True)
 b=subprocess.run(cmd,env=env,capture_output=True,text=True);print(b.stdout+b.stderr,end='',flush=True)
 if b.returncode:results.append(dict(ecu=ecu,test='diagnostics',build=b.returncode));continue
 t=subprocess.run([str(exe)],capture_output=True,text=True);print(t.stdout+t.stderr,end='',flush=True)
 results.append(dict(ecu=ecu,test='diagnostics',build=0,run=t.returncode))
for ecu,dirname in [('CBM','CBM - Central Body Module/CBM_APPL'),('DMU','DMU - Digital Main Unit/DMU_APPL')]:
 r=root/dirname;m=r/'BSW/Mem';d=r/'BSW/Diag/Dem';exe=build/(ecu.lower()+'-migration.exe')
 sources=[r/'tests/test_migration.c',m/'NvM_Engine.c',m/'NvM_Cfg.c',m/'MemIf/MemIf.c',m/'Crc/Crc.c',d/'Dem.c',d/'Dem_Core.c',d/'Dem_Cfg.c']
 includes=[r/'BSW',m,d,r/'APPL/InputCtrl',r/'APPL/ActCtrl',r/'BSW/Sys/EcuM']
 cmd=[a.cc,'cc','-target','x86_64-windows-gnu','-mcpu=baseline','-std=c11','-Wall','-Wextra','-Werror','-DDEM_HOST_TEST',*['-I'+str(p) for p in includes],*map(str,sources),'-o',str(exe)]
 print('BUILD',ecu,'migration',flush=True);b=subprocess.run(cmd,env=env,capture_output=True,text=True);print(b.stdout+b.stderr,end='',flush=True)
 if b.returncode:results.append(dict(ecu=ecu,test='migration',build=b.returncode));continue
 t=subprocess.run([str(exe)],capture_output=True,text=True);print(t.stdout+t.stderr,end='',flush=True);results.append(dict(ecu=ecu,test='migration',build=0,run=t.returncode))
for ecu,dirname in [('CBM','CBM - Central Body Module/CBM_FBL'),('DMU','DMU - Digital Main Unit/DMU_FBL')]:
 r=root/dirname;c=r/'BSW/Com';exe=build/(ecu.lower()+'-fbl-transport.exe')
 sources=[r/'tests/test_transport.c',r/'Core/Src/Fbl.c',r/'Core/Src/Fbl_Updater.c',r/'BSW/Crc/Crc.c']
 sources += [c/x/(x+'.c') for x in ['Can','CanIf','CanTp','PduR']]
 if ecu=='CBM':sources += [r/'BSW/Mem/MemIf/MemIf.c',r/'BSW/Mem/NvM_Cfg.c']
 includes=[r/'Core/Inc',r/'BSW/Crc',*[c/x for x in ['Can','CanIf','CanTp','PduR']]]
 cmd=[a.cc,'cc','-target','x86_64-windows-gnu','-mcpu=baseline','-std=c11','-Wall','-Wextra','-Werror',*['-I'+str(p) for p in includes],*map(str,sources),'-o',str(exe)]
 print('BUILD',ecu,'FBL transport',flush=True);b=subprocess.run(cmd,env=env,capture_output=True,text=True);print(b.stdout+b.stderr,end='',flush=True)
 if b.returncode:results.append(dict(ecu=ecu,test='fbl_transport',build=b.returncode));continue
 t=subprocess.run([str(exe)],capture_output=True,text=True);print(t.stdout+t.stderr,end='',flush=True);results.append(dict(ecu=ecu,test='fbl_transport',build=0,run=t.returncode))
for ecu,dirname in [('CBM','CBM - Central Body Module/CBM_FBL'),('DMU','DMU - Digital Main Unit/DMU_FBL')]:
 r=root/dirname;exe=build/(ecu.lower()+'-ram-updater.exe')
 sources=[r/'tests/test_ram_updater.c',r/'Core/Src/ram_blob.c',r/'Core/Src/Fbl_Updater.c',r/'BSW/Crc/Crc.c']
 cmd=[a.cc,'cc','-target','x86_64-windows-gnu','-mcpu=baseline','-std=c11','-Wall','-Wextra','-Werror','-DRAM_HOST_TEST','-I'+str(r/'Core/Inc'),*map(str,sources),'-o',str(exe)]
 print('BUILD',ecu,'RAM updater',flush=True);b=subprocess.run(cmd,env=env,capture_output=True,text=True);print(b.stdout+b.stderr,end='',flush=True)
 if b.returncode:results.append(dict(ecu=ecu,test='ram_updater',build=b.returncode));continue
 image=build/(ecu.lower()+'-fbl-arm-controlled')/(ecu+'_FBL.bin')
 t=subprocess.run([str(exe),*([str(image)] if image.exists() else [])],capture_output=True,text=True);print(t.stdout+t.stderr,end='',flush=True);results.append(dict(ecu=ecu,test='ram_updater',build=0,run=t.returncode,linkedImage=str(image) if image.exists() else None))
(build/'host-results.json').write_text(json.dumps(results,indent=2))
sys.exit(int(any(x.get('build') or x.get('run') for x in results)))
