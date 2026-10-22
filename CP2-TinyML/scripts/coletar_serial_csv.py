#!/usr/bin/env python3
"""Coleta CSV real da serial: python coletar_serial_csv.py --porta COM3 --classe reduzir --segundos 20 --saida dataset/reduzir_01.csv"""
import argparse,csv,time
from pathlib import Path
try: import serial
except ImportError: raise SystemExit('Instale pyserial: python -m pip install pyserial')
p=argparse.ArgumentParser()
p.add_argument('--porta',required=True)
p.add_argument('--classe',choices=['avancar','reduzir'],required=True)
p.add_argument('--segundos',type=float,default=20)
p.add_argument('--baud',type=int,default=115200)
p.add_argument('--saida',required=True)
a=p.parse_args()
if a.segundos<=0: p.error('--segundos deve ser positivo')
path=Path(a.saida);path.parent.mkdir(parents=True,exist_ok=True)
contador=0
with serial.Serial(a.porta,a.baud,timeout=1) as esp, path.open('w',newline='',encoding='utf-8') as f:
    writer=csv.writer(f);writer.writerow(['timestamp_ms','adc_bruto','valor_sensor','classe'])
    time.sleep(2) # reinicialização da placa
    deadline=time.monotonic()+a.segundos
    while time.monotonic()<deadline:
        try:
            line=esp.readline().decode('utf-8',errors='replace').strip()
            partes=line.split(',')
            if len(partes)!=3 or not partes[0].isdigit(): continue
            ms,bruto,valor=map(int,partes)
            if not (0<=bruto<=4095 and 0<=valor<=1023):continue
            writer.writerow([ms,bruto,valor,a.classe]);contador+=1
        except ValueError: continue
print(f'{contador} medições reais gravadas em {path}')
