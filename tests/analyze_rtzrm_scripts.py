#!/usr/bin/env python3
import argparse, collections, hashlib, struct, sys
from dataclasses import dataclass

SIGLEN={1:2,2:2,3:2,4:0,5:0,6:0,7:0,8:0,9:0,10:0,11:0,12:0,13:0,14:0,15:0,16:0,17:0,18:0,19:2,20:2,21:0,22:0,23:2,24:0,25:0,26:0,27:0,28:0,29:0,30:0,31:0,32:1,33:0,34:0,35:0,36:0,37:0,38:0,39:0,40:1,41:1,42:1,43:1,44:1,45:0,46:0,47:0,48:0,49:0,50:0,51:1,52:2,53:0,54:0,55:0,56:0,57:0,58:0,59:0,60:0}

@dataclass(frozen=True)
class V:
    kind:str
    value:object=None
UNK=V('unknown')
def C(v): return V('const',v)
def A(v): return V('arg',v)

def merge(a,b): return a if a==b else UNK

def disasm(code):
    pc=0; out=[]
    while pc<len(code):
        op=code[pc]
        if op not in SIGLEN: return None
        n=SIGLEN[op]
        if pc+1+n>len(code): return None
        out.append((pc,op,code[pc+1:pc+1+n],pc+1+n)); pc+=1+n
    return out

def parse_db(blob):
    if blob[:6] != b'ADVSYS': raise ValueError('not ADVSYS')
    version,sub=struct.unpack_from('<HH',blob,6)
    if version not in (209,210): raise ValueError(f'unsupported MADE database version {version}')
    p=28
    _unk=struct.unpack_from('<H',blob,p)[0];p+=2
    idxoff=struct.unpack_from('<I',blob,p)[0];p+=4
    count=struct.unpack_from('<H',blob,p)[0];p+=2
    stateoff=struct.unpack_from('<I',blob,p)[0];p+=4
    statesz=struct.unpack_from('<I',blob,p)[0];p+=4
    objoff=struct.unpack_from('<I',blob,p)[0];p+=4
    objsz=struct.unpack_from('<I',blob,p)[0];p+=4
    main=struct.unpack_from('<H',blob,p)[0]
    state=blob[stateoff:stateoff+statesz]
    refs=struct.unpack_from('<'+'I'*count,blob,idxoff)
    objs=[]
    for idx,ref in enumerate(refs,1):
        pos=(objoff+(ref^1)) if (ref&1) else (stateoff+ref)
        flags,cls,size=struct.unpack_from('<HHH',blob,pos)
        c1=blob[pos+4];c2=blob[pos+5]
        if cls==0x7fff: dlen=size
        elif cls==0x7ffe: dlen=size*2
        elif version==210: dlen=size*4
        else: dlen=(c1+c2)*2
        objs.append(dict(index=idx,flags=flags,cls=cls,size=size,c1=c1,c2=c2,data=blob[pos+6:pos+6+dlen]))
    return version,sub,main,state,objs

def analyze_code(code):
    ins=disasm(code)
    if not ins: return [],[]
    im={pc:(op,args,nxt) for pc,op,args,nxt in ins}
    states={0:(UNK,)*48}; q=collections.deque([0]); calls=[]; exts=[]
    while q:
        pc=q.popleft(); st=list(states[pc]); op,args,nxt=im[pc]
        while len(st)<272: st.append(UNK)
        succ=[]
        if op in (1,2): succ=[nxt,struct.unpack('<H',args)[0]]
        elif op==3: succ=[struct.unpack('<H',args)[0]]
        elif op in (4,5): st[0]=C(-1 if op==4 else 0);succ=[nxt]
        elif op==6: st.insert(0,C(0));succ=[nxt]
        elif op in (7,15,20,24,26,29,33,34,35,36,37,38,39,45,46,47,49,50,53,54,55,56,59,60): st[0]=UNK;succ=[nxt]
        elif op in (8,9,10,11,12,13,14,16,17,18,57,58): st.pop(0);st[0]=UNK;succ=[nxt]
        elif op==19: st[0]=C(struct.unpack('<h',args)[0]);succ=[nxt]
        elif op==21: st.pop(0);st[0]=UNK;succ=[nxt]
        elif op==22: st.pop(0);st.pop(0);st[0]=UNK;succ=[nxt]
        elif op==23: succ=[nxt]
        elif op in (25,): succ=[nxt]
        elif op==27: st.pop(0);st[0]=UNK;succ=[nxt]
        elif op==28: val=st.pop(0);st.pop(0);st[0]=val;succ=[nxt]
        elif op in (30,31): succ=[]
        elif op==32:
            argc=args[0]; calls.append((pc,st[argc],tuple(st[:argc]),argc));st=[UNK]+st[argc+1:];succ=[nxt]
        elif op==40: st[0]=A(args[0]);succ=[nxt]
        elif op in (41,43): succ=[nxt]
        elif op==42: st[0]=V('tmp',args[0]);succ=[nxt]
        elif op==44: st=[C(0)]*args[0]+st;succ=[nxt]
        elif op==48: st[0]=C(0);succ=[nxt]
        elif op==51:
            argc=args[0];st=[UNK]+st[argc+2:];succ=[nxt]
        elif op==52:
            fn,argc=args;exts.append((pc,fn,tuple(st[:argc]),argc));st=[UNK]+st[argc+1:];succ=[nxt]
        else: succ=[nxt]
        ns=tuple(st[:96])
        for sp in succ:
            if sp not in im: continue
            if sp not in states: states[sp]=ns;q.append(sp);continue
            old=states[sp];L=max(len(old),len(ns));aa=old+(UNK,)*(L-len(old));bb=ns+(UNK,)*(L-len(ns));mm=tuple(merge(a,b) for a,b in zip(aa,bb))
            if mm!=old: states[sp]=mm;q.append(sp)
    return calls,exts

def label(v): return v.value if v.kind=='const' else v.kind

def main():
    ap=argparse.ArgumentParser();ap.add_argument('dat');args=ap.parse_args()
    blob=open(args.dat,'rb').read();version,sub,mainobj,state,objs=parse_db(blob)
    allcalls=[];allext=[]
    for o in objs:
        if o['cls']!=0x7fff: continue
        if disasm(o['data']) is None: continue
        calls,exts=analyze_code(o['data'])
        allcalls += [(o['index'],)+x for x in calls]
        allext += [(o['index'],)+x for x in exts]
    play=[]
    for obj,pc,target,a,argc in allcalls:
        if target==C(42) and argc==3:
            play.append((obj,pc,a))
    # Static instructions are unique by object/pc.
    uniq={ (o,p):(o,p,a) for o,p,a in play }
    play=list(uniq.values())
    pairs=collections.Counter((label(a[0]),label(a[1])) for _,_,a in play)
    front=sum(n for (layer,_mode),n in pairs.items() if layer==2)
    behind=sum(n for (layer,_mode),n in pairs.items() if layer==1)
    modes=collections.Counter()
    for (_layer,mode),n in pairs.items(): modes[mode]+=n
    # Literal/variable property references to wrapper 106 (object 48).
    refs48=[]
    for o in objs:
        if o['cls']>=0x7ffe: continue
        words=list(struct.unpack('<'+'H'*(len(o['data'])//2),o['data']))
        c1,c2=o['c1'],o['c2']
        if c1+c2>len(words): continue
        for j in range(c2):
            prop=words[j]; vw=words[c1+j]; pid=prop&0x3fff
            if prop&0x4000:
                off=vw*2
                val=struct.unpack_from('<h',state,off)[0] if off+2<=len(state) else None
            else:
                val=struct.unpack('<h',struct.pack('<H',vw))[0]
            if val==48: refs48.append((o['index'],pid))
    print(f'MD5_full={hashlib.md5(blob).hexdigest()} MD5_scummvm5000={hashlib.md5(blob[:5000]).hexdigest()} size={len(blob)} MADE={version}.{sub} objects={len(objs)} main=0x{mainobj:04X}')
    print(f'PlayMpegMovie wrapper calls={len(play)} layer1_behind={behind} layer2_front={front}')
    print('play modes:', ' '.join(f'{k}={v}' for k,v in sorted(modes.items(),key=lambda kv:str(kv[0]))))
    print(f'external106 wrapper object 48 property references={len(refs48)} {refs48}')
    if hashlib.md5(blob[:5000]).hexdigest()=='2cb1a9fa63536cf56f104a729aa72a91' and len(play)==100 and front==38 and behind==62 and modes.get(3)==84 and modes.get(1)==12 and modes.get(2)==3:
        print('RTZRM_SCRIPT_AUDIT_OK')
        return 0
    print('RTZRM_SCRIPT_AUDIT_WARNING: database differs from the audited 5/25/94 copy')
    return 0
if __name__=='__main__': sys.exit(main())
