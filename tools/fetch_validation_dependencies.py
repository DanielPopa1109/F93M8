"""Fetch pinned official STM32/CMSIS headers for source/build validation only."""
from pathlib import Path
import urllib.request,zipfile,io
out=Path(__file__).parent/'.runtime/vendor';out.mkdir(parents=True,exist_ok=True)
archives=[('hal','https://github.com/STMicroelectronics/stm32f4xx-hal-driver/archive/refs/tags/v1.8.3.zip'),('device','https://github.com/STMicroelectronics/cmsis-device-f4/archive/refs/tags/v2.6.9.zip'),('cmsis','https://github.com/ARM-software/CMSIS_5/archive/refs/tags/5.9.0.zip')]
for name,url in archives:
 if (out/name).exists():continue
 print('Fetch',name,flush=True)
 data=urllib.request.urlopen(url,timeout=60).read()
 with zipfile.ZipFile(io.BytesIO(data)) as z:
  target=out/name;target.mkdir()
  for member in z.infolist():
   parts=Path(member.filename).parts[1:]
   if not parts or member.is_dir():continue
   p=target.joinpath(*parts);p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(z.read(member))
print('Official validation dependencies ready')
analysis=Path(__file__).parent/'.runtime/analysis'
if not (analysis/'capstone').exists() or not (analysis/'elftools').exists():
 import subprocess,sys
 subprocess.run([sys.executable,'-m','pip','install','--target',str(analysis),'pyelftools==0.32','capstone==5.0.3'],check=True)
