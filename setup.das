
D:\Developmemts\github\xbook2\src\arch\x86\boot\setup.elf:     file format elf32-i386


Disassembly of section .text:

00091000 <_start>:
   91000:	66 b8 10 00          	mov    ax,0x10
   91004:	8e d8                	mov    ds,eax
   91006:	8e c0                	mov    es,eax
   91008:	8e e0                	mov    fs,eax
   9100a:	8e e8                	mov    gs,eax
   9100c:	8e d0                	mov    ss,eax
   9100e:	bc 00 00 09 00       	mov    esp,0x90000
   91013:	c6 05 40 81 0b 00 50 	mov    BYTE PTR ds:0xb8140,0x50
   9101a:	c6 05 41 81 0b 00 07 	mov    BYTE PTR ds:0xb8141,0x7
   91021:	c6 05 42 81 0b 00 52 	mov    BYTE PTR ds:0xb8142,0x52
   91028:	c6 05 43 81 0b 00 07 	mov    BYTE PTR ds:0xb8143,0x7
   9102f:	c6 05 44 81 0b 00 4f 	mov    BYTE PTR ds:0xb8144,0x4f
   91036:	c6 05 45 81 0b 00 07 	mov    BYTE PTR ds:0xb8145,0x7
   9103d:	c6 05 46 81 0b 00 54 	mov    BYTE PTR ds:0xb8146,0x54
   91044:	c6 05 47 81 0b 00 07 	mov    BYTE PTR ds:0xb8147,0x7
   9104b:	c6 05 48 81 0b 00 45 	mov    BYTE PTR ds:0xb8148,0x45
   91052:	c6 05 49 81 0b 00 07 	mov    BYTE PTR ds:0xb8149,0x7
   91059:	c6 05 4a 81 0b 00 43 	mov    BYTE PTR ds:0xb814a,0x43
   91060:	c6 05 4b 81 0b 00 07 	mov    BYTE PTR ds:0xb814b,0x7
   91067:	c6 05 4c 81 0b 00 54 	mov    BYTE PTR ds:0xb814c,0x54
   9106e:	c6 05 4d 81 0b 00 07 	mov    BYTE PTR ds:0xb814d,0x7
   91075:	c6 05 4e 81 0b 00 53 	mov    BYTE PTR ds:0xb814e,0x53
   9107c:	c6 05 4f 81 0b 00 07 	mov    BYTE PTR ds:0xb814f,0x7
   91083:	e9 05 02 00 00       	jmp    9128d <setup_entry>

00091088 <outb>:
   91088:	52                   	push   edx
   91089:	8b 54 24 08          	mov    edx,DWORD PTR [esp+0x8]
   9108d:	8a 44 24 0c          	mov    al,BYTE PTR [esp+0xc]
   91091:	ee                   	out    dx,al
   91092:	5a                   	pop    edx
   91093:	c3                   	ret    

00091094 <enable_paging>:
   91094:	8b 44 24 04          	mov    eax,DWORD PTR [esp+0x4]
   91098:	0f 22 d8             	mov    cr3,eax
   9109b:	0f 20 c0             	mov    eax,cr0
   9109e:	0d 00 00 00 80       	or     eax,0x80000000
   910a3:	0f 22 c0             	mov    cr0,eax
   910a6:	c3                   	ret    
   910a7:	90                   	nop

000910a8 <read_file>:
   910a8:	55                   	push   ebp
   910a9:	89 e5                	mov    ebp,esp
   910ab:	83 ec 28             	sub    esp,0x28
   910ae:	8b 45 10             	mov    eax,DWORD PTR [ebp+0x10]
   910b1:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
   910b4:	01 d0                	add    eax,edx
   910b6:	89 45 f4             	mov    DWORD PTR [ebp-0xc],eax
   910b9:	8b 45 14             	mov    eax,DWORD PTR [ebp+0x14]
   910bc:	89 44 24 08          	mov    DWORD PTR [esp+0x8],eax
   910c0:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   910c3:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
   910c7:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   910ca:	89 04 24             	mov    DWORD PTR [esp],eax
   910cd:	e8 5c 06 00 00       	call   9172e <memcpy>
   910d2:	8b 45 14             	mov    eax,DWORD PTR [ebp+0x14]
   910d5:	c9                   	leave  
   910d6:	c3                   	ret    

000910d7 <setup_kernel>:
   910d7:	55                   	push   ebp
   910d8:	89 e5                	mov    ebp,esp
   910da:	81 ec 88 00 00 00    	sub    esp,0x88
   910e0:	c7 04 24 88 1a 09 00 	mov    DWORD PTR [esp],0x91a88
   910e7:	e8 59 05 00 00       	call   91645 <print_str>
   910ec:	c7 44 24 08 34 00 00 	mov    DWORD PTR [esp+0x8],0x34
   910f3:	00 
   910f4:	c7 44 24 04 00 00 00 	mov    DWORD PTR [esp+0x4],0x0
   910fb:	00 
   910fc:	8d 45 a8             	lea    eax,[ebp-0x58]
   910ff:	89 04 24             	mov    DWORD PTR [esp],eax
   91102:	e8 f7 05 00 00       	call   916fe <memset>
   91107:	c7 45 ec 00 00 01 00 	mov    DWORD PTR [ebp-0x14],0x10000
   9110e:	c7 44 24 0c 34 00 00 	mov    DWORD PTR [esp+0xc],0x34
   91115:	00 
   91116:	c7 44 24 08 00 00 00 	mov    DWORD PTR [esp+0x8],0x0
   9111d:	00 
   9111e:	8d 45 a8             	lea    eax,[ebp-0x58]
   91121:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
   91125:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   91128:	89 04 24             	mov    DWORD PTR [esp],eax
   9112b:	e8 78 ff ff ff       	call   910a8 <read_file>
   91130:	c7 44 24 08 07 00 00 	mov    DWORD PTR [esp+0x8],0x7
   91137:	00 
   91138:	c7 44 24 04 97 1a 09 	mov    DWORD PTR [esp+0x4],0x91a97
   9113f:	00 
   91140:	8d 45 a8             	lea    eax,[ebp-0x58]
   91143:	89 04 24             	mov    DWORD PTR [esp],eax
   91146:	e8 1b 06 00 00       	call   91766 <memcmp>
   9114b:	85 c0                	test   eax,eax
   9114d:	75 30                	jne    9117f <setup_kernel+0xa8>
   9114f:	0f b7 45 b8          	movzx  eax,WORD PTR [ebp-0x48]
   91153:	66 83 f8 02          	cmp    ax,0x2
   91157:	75 26                	jne    9117f <setup_kernel+0xa8>
   91159:	0f b7 45 ba          	movzx  eax,WORD PTR [ebp-0x46]
   9115d:	66 83 f8 03          	cmp    ax,0x3
   91161:	75 1c                	jne    9117f <setup_kernel+0xa8>
   91163:	8b 45 bc             	mov    eax,DWORD PTR [ebp-0x44]
   91166:	83 f8 01             	cmp    eax,0x1
   91169:	75 14                	jne    9117f <setup_kernel+0xa8>
   9116b:	0f b7 45 d4          	movzx  eax,WORD PTR [ebp-0x2c]
   9116f:	66 3d 00 04          	cmp    ax,0x400
   91173:	77 0a                	ja     9117f <setup_kernel+0xa8>
   91175:	0f b7 45 d2          	movzx  eax,WORD PTR [ebp-0x2e]
   91179:	66 83 f8 20          	cmp    ax,0x20
   9117d:	74 24                	je     911a3 <setup_kernel+0xcc>
   9117f:	c7 04 24 a0 1a 09 00 	mov    DWORD PTR [esp],0x91aa0
   91186:	e8 ba 04 00 00       	call   91645 <print_str>
   9118b:	c7 45 dc 02 80 0b 00 	mov    DWORD PTR [ebp-0x24],0xb8002
   91192:	8b 45 dc             	mov    eax,DWORD PTR [ebp-0x24]
   91195:	c6 00 71             	mov    BYTE PTR [eax],0x71
   91198:	8b 45 dc             	mov    eax,DWORD PTR [ebp-0x24]
   9119b:	83 c0 01             	add    eax,0x1
   9119e:	c6 00 05             	mov    BYTE PTR [eax],0x5
   911a1:	eb fe                	jmp    911a1 <setup_kernel+0xca>
   911a3:	8b 45 c4             	mov    eax,DWORD PTR [ebp-0x3c]
   911a6:	89 45 f4             	mov    DWORD PTR [ebp-0xc],eax
   911a9:	0f b7 45 d2          	movzx  eax,WORD PTR [ebp-0x2e]
   911ad:	66 89 45 ea          	mov    WORD PTR [ebp-0x16],ax
   911b1:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [ebp-0x10],0x0
   911b8:	e9 9d 00 00 00       	jmp    9125a <setup_kernel+0x183>
   911bd:	0f b7 45 ea          	movzx  eax,WORD PTR [ebp-0x16]
   911c1:	89 44 24 08          	mov    DWORD PTR [esp+0x8],eax
   911c5:	c7 44 24 04 00 00 00 	mov    DWORD PTR [esp+0x4],0x0
   911cc:	00 
   911cd:	8d 45 88             	lea    eax,[ebp-0x78]
   911d0:	89 04 24             	mov    DWORD PTR [esp],eax
   911d3:	e8 26 05 00 00       	call   916fe <memset>
   911d8:	0f b7 45 ea          	movzx  eax,WORD PTR [ebp-0x16]
   911dc:	89 44 24 0c          	mov    DWORD PTR [esp+0xc],eax
   911e0:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   911e3:	89 44 24 08          	mov    DWORD PTR [esp+0x8],eax
   911e7:	8d 45 88             	lea    eax,[ebp-0x78]
   911ea:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
   911ee:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   911f1:	89 04 24             	mov    DWORD PTR [esp],eax
   911f4:	e8 af fe ff ff       	call   910a8 <read_file>
   911f9:	8b 45 88             	mov    eax,DWORD PTR [ebp-0x78]
   911fc:	83 f8 01             	cmp    eax,0x1
   911ff:	75 4e                	jne    9124f <setup_kernel+0x178>
   91201:	8b 4d 98             	mov    ecx,DWORD PTR [ebp-0x68]
   91204:	8b 55 8c             	mov    edx,DWORD PTR [ebp-0x74]
   91207:	8b 45 90             	mov    eax,DWORD PTR [ebp-0x70]
   9120a:	89 4c 24 0c          	mov    DWORD PTR [esp+0xc],ecx
   9120e:	89 54 24 08          	mov    DWORD PTR [esp+0x8],edx
   91212:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
   91216:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   91219:	89 04 24             	mov    DWORD PTR [esp],eax
   9121c:	e8 87 fe ff ff       	call   910a8 <read_file>
   91221:	8b 55 9c             	mov    edx,DWORD PTR [ebp-0x64]
   91224:	8b 45 98             	mov    eax,DWORD PTR [ebp-0x68]
   91227:	39 c2                	cmp    edx,eax
   91229:	76 24                	jbe    9124f <setup_kernel+0x178>
   9122b:	8b 55 9c             	mov    edx,DWORD PTR [ebp-0x64]
   9122e:	8b 45 98             	mov    eax,DWORD PTR [ebp-0x68]
   91231:	29 c2                	sub    edx,eax
   91233:	8b 4d 90             	mov    ecx,DWORD PTR [ebp-0x70]
   91236:	8b 45 98             	mov    eax,DWORD PTR [ebp-0x68]
   91239:	01 c8                	add    eax,ecx
   9123b:	89 54 24 08          	mov    DWORD PTR [esp+0x8],edx
   9123f:	c7 44 24 04 00 00 00 	mov    DWORD PTR [esp+0x4],0x0
   91246:	00 
   91247:	89 04 24             	mov    DWORD PTR [esp],eax
   9124a:	e8 af 04 00 00       	call   916fe <memset>
   9124f:	0f b7 45 ea          	movzx  eax,WORD PTR [ebp-0x16]
   91253:	01 45 f4             	add    DWORD PTR [ebp-0xc],eax
   91256:	83 45 f0 01          	add    DWORD PTR [ebp-0x10],0x1
   9125a:	0f b7 45 d4          	movzx  eax,WORD PTR [ebp-0x2c]
   9125e:	0f b7 c0             	movzx  eax,ax
   91261:	3b 45 f0             	cmp    eax,DWORD PTR [ebp-0x10]
   91264:	0f 87 53 ff ff ff    	ja     911bd <setup_kernel+0xe6>
   9126a:	c7 45 e4 02 80 0b 00 	mov    DWORD PTR [ebp-0x1c],0xb8002
   91271:	8b 45 e4             	mov    eax,DWORD PTR [ebp-0x1c]
   91274:	c6 00 66             	mov    BYTE PTR [eax],0x66
   91277:	8b 45 e4             	mov    eax,DWORD PTR [ebp-0x1c]
   9127a:	83 c0 01             	add    eax,0x1
   9127d:	c6 00 05             	mov    BYTE PTR [eax],0x5
   91280:	8b 45 c0             	mov    eax,DWORD PTR [ebp-0x40]
   91283:	89 45 e0             	mov    DWORD PTR [ebp-0x20],eax
   91286:	8b 45 e0             	mov    eax,DWORD PTR [ebp-0x20]
   91289:	ff d0                	call   eax
   9128b:	c9                   	leave  
   9128c:	c3                   	ret    

0009128d <setup_entry>:
   9128d:	55                   	push   ebp
   9128e:	89 e5                	mov    ebp,esp
   91290:	83 ec 28             	sub    esp,0x28
   91293:	c7 45 f4 02 80 0b 00 	mov    DWORD PTR [ebp-0xc],0xb8002
   9129a:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   9129d:	c6 00 61             	mov    BYTE PTR [eax],0x61
   912a0:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   912a3:	83 c0 01             	add    eax,0x1
   912a6:	c6 00 05             	mov    BYTE PTR [eax],0x5
   912a9:	c7 04 24 51 00 00 00 	mov    DWORD PTR [esp],0x51
   912b0:	e8 26 00 00 00       	call   912db <put_char2>
   912b5:	c7 04 24 57 00 00 00 	mov    DWORD PTR [esp],0x57
   912bc:	e8 1a 00 00 00       	call   912db <put_char2>
   912c1:	c7 04 24 45 00 00 00 	mov    DWORD PTR [esp],0x45
   912c8:	e8 0e 00 00 00       	call   912db <put_char2>
   912cd:	c7 04 24 52 00 00 00 	mov    DWORD PTR [esp],0x52
   912d4:	e8 02 00 00 00       	call   912db <put_char2>
   912d9:	eb fe                	jmp    912d9 <setup_entry+0x4c>

000912db <put_char2>:
   912db:	55                   	push   ebp
   912dc:	89 e5                	mov    ebp,esp
   912de:	83 ec 14             	sub    esp,0x14
   912e1:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   912e4:	88 45 ec             	mov    BYTE PTR [ebp-0x14],al
   912e7:	8b 15 24 2d 09 00    	mov    edx,DWORD PTR ds:0x92d24
   912ed:	89 d0                	mov    eax,edx
   912ef:	c1 e0 02             	shl    eax,0x2
   912f2:	01 d0                	add    eax,edx
   912f4:	c1 e0 04             	shl    eax,0x4
   912f7:	89 c2                	mov    edx,eax
   912f9:	a1 20 2d 09 00       	mov    eax,ds:0x92d20
   912fe:	01 d0                	add    eax,edx
   91300:	05 00 c0 05 00       	add    eax,0x5c000
   91305:	01 c0                	add    eax,eax
   91307:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
   9130a:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9130d:	0f b6 55 ec          	movzx  edx,BYTE PTR [ebp-0x14]
   91311:	88 10                	mov    BYTE PTR [eax],dl
   91313:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   91317:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9131a:	c6 00 07             	mov    BYTE PTR [eax],0x7
   9131d:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   91321:	a1 20 2d 09 00       	mov    eax,ds:0x92d20
   91326:	83 c0 02             	add    eax,0x2
   91329:	a3 20 2d 09 00       	mov    ds:0x92d20,eax
   9132e:	a1 20 2d 09 00       	mov    eax,ds:0x92d20
   91333:	85 c0                	test   eax,eax
   91335:	75 12                	jne    91349 <put_char2+0x6e>
   91337:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9133a:	c6 00 45             	mov    BYTE PTR [eax],0x45
   9133d:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   91341:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91344:	c6 00 05             	mov    BYTE PTR [eax],0x5
   91347:	eb fe                	jmp    91347 <put_char2+0x6c>
   91349:	a1 20 2d 09 00       	mov    eax,ds:0x92d20
   9134e:	83 f8 4f             	cmp    eax,0x4f
   91351:	7e 17                	jle    9136a <put_char2+0x8f>
   91353:	c7 05 20 2d 09 00 00 	mov    DWORD PTR ds:0x92d20,0x0
   9135a:	00 00 00 
   9135d:	a1 24 2d 09 00       	mov    eax,ds:0x92d24
   91362:	83 c0 01             	add    eax,0x1
   91365:	a3 24 2d 09 00       	mov    ds:0x92d24,eax
   9136a:	c9                   	leave  
   9136b:	c3                   	ret    

0009136c <set_cursor>:
   9136c:	55                   	push   ebp
   9136d:	89 e5                	mov    ebp,esp
   9136f:	83 ec 28             	sub    esp,0x28
   91372:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   91375:	66 89 45 f4          	mov    WORD PTR [ebp-0xc],ax
   91379:	c7 44 24 04 0e 00 00 	mov    DWORD PTR [esp+0x4],0xe
   91380:	00 
   91381:	c7 04 24 d4 03 00 00 	mov    DWORD PTR [esp],0x3d4
   91388:	e8 fb fc ff ff       	call   91088 <outb>
   9138d:	0f b7 45 f4          	movzx  eax,WORD PTR [ebp-0xc]
   91391:	66 c1 e8 08          	shr    ax,0x8
   91395:	0f b7 c0             	movzx  eax,ax
   91398:	25 ff 00 00 00       	and    eax,0xff
   9139d:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
   913a1:	c7 04 24 d5 03 00 00 	mov    DWORD PTR [esp],0x3d5
   913a8:	e8 db fc ff ff       	call   91088 <outb>
   913ad:	c7 44 24 04 0f 00 00 	mov    DWORD PTR [esp+0x4],0xf
   913b4:	00 
   913b5:	c7 04 24 d4 03 00 00 	mov    DWORD PTR [esp],0x3d4
   913bc:	e8 c7 fc ff ff       	call   91088 <outb>
   913c1:	0f b7 45 f4          	movzx  eax,WORD PTR [ebp-0xc]
   913c5:	25 ff 00 00 00       	and    eax,0xff
   913ca:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
   913ce:	c7 04 24 d5 03 00 00 	mov    DWORD PTR [esp],0x3d5
   913d5:	e8 ae fc ff ff       	call   91088 <outb>
   913da:	c9                   	leave  
   913db:	c3                   	ret    

000913dc <flush>:
   913dc:	55                   	push   ebp
   913dd:	89 e5                	mov    ebp,esp
   913df:	5d                   	pop    ebp
   913e0:	c3                   	ret    

000913e1 <scroll_sceen>:
   913e1:	55                   	push   ebp
   913e2:	89 e5                	mov    ebp,esp
   913e4:	83 ec 10             	sub    esp,0x10
   913e7:	c7 45 f8 00 80 0b 00 	mov    DWORD PTR [ebp-0x8],0xb8000
   913ee:	83 7d 08 ff          	cmp    DWORD PTR [ebp+0x8],0xffffffff
   913f2:	75 77                	jne    9146b <scroll_sceen+0x8a>
   913f4:	c7 45 fc 00 0f 00 00 	mov    DWORD PTR [ebp-0x4],0xf00
   913fb:	eb 35                	jmp    91432 <scroll_sceen+0x51>
   913fd:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91400:	03 45 f8             	add    eax,DWORD PTR [ebp-0x8]
   91403:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
   91406:	81 ea a0 00 00 00    	sub    edx,0xa0
   9140c:	03 55 f8             	add    edx,DWORD PTR [ebp-0x8]
   9140f:	0f b6 12             	movzx  edx,BYTE PTR [edx]
   91412:	88 10                	mov    BYTE PTR [eax],dl
   91414:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91417:	83 c0 01             	add    eax,0x1
   9141a:	03 45 f8             	add    eax,DWORD PTR [ebp-0x8]
   9141d:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
   91420:	81 ea 9f 00 00 00    	sub    edx,0x9f
   91426:	03 55 f8             	add    edx,DWORD PTR [ebp-0x8]
   91429:	0f b6 12             	movzx  edx,BYTE PTR [edx]
   9142c:	88 10                	mov    BYTE PTR [eax],dl
   9142e:	83 6d fc 02          	sub    DWORD PTR [ebp-0x4],0x2
   91432:	81 7d fc a0 00 00 00 	cmp    DWORD PTR [ebp-0x4],0xa0
   91439:	7f c2                	jg     913fd <scroll_sceen+0x1c>
   9143b:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
   91442:	eb 19                	jmp    9145d <scroll_sceen+0x7c>
   91444:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91447:	03 45 f8             	add    eax,DWORD PTR [ebp-0x8]
   9144a:	c6 00 00             	mov    BYTE PTR [eax],0x0
   9144d:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91450:	83 c0 01             	add    eax,0x1
   91453:	03 45 f8             	add    eax,DWORD PTR [ebp-0x8]
   91456:	c6 00 07             	mov    BYTE PTR [eax],0x7
   91459:	83 45 fc 02          	add    DWORD PTR [ebp-0x4],0x2
   9145d:	81 7d fc 9f 00 00 00 	cmp    DWORD PTR [ebp-0x4],0x9f
   91464:	7e de                	jle    91444 <scroll_sceen+0x63>
   91466:	e9 85 00 00 00       	jmp    914f0 <scroll_sceen+0x10f>
   9146b:	83 7d 08 01          	cmp    DWORD PTR [ebp+0x8],0x1
   9146f:	75 7f                	jne    914f0 <scroll_sceen+0x10f>
   91471:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
   91478:	eb 35                	jmp    914af <scroll_sceen+0xce>
   9147a:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9147d:	03 45 f8             	add    eax,DWORD PTR [ebp-0x8]
   91480:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
   91483:	81 c2 a0 00 00 00    	add    edx,0xa0
   91489:	03 55 f8             	add    edx,DWORD PTR [ebp-0x8]
   9148c:	0f b6 12             	movzx  edx,BYTE PTR [edx]
   9148f:	88 10                	mov    BYTE PTR [eax],dl
   91491:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91494:	83 c0 01             	add    eax,0x1
   91497:	03 45 f8             	add    eax,DWORD PTR [ebp-0x8]
   9149a:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
   9149d:	81 c2 a1 00 00 00    	add    edx,0xa1
   914a3:	03 55 f8             	add    edx,DWORD PTR [ebp-0x8]
   914a6:	0f b6 12             	movzx  edx,BYTE PTR [edx]
   914a9:	88 10                	mov    BYTE PTR [eax],dl
   914ab:	83 45 fc 02          	add    DWORD PTR [ebp-0x4],0x2
   914af:	81 7d fc ff 0e 00 00 	cmp    DWORD PTR [ebp-0x4],0xeff
   914b6:	7e c2                	jle    9147a <scroll_sceen+0x99>
   914b8:	c7 45 fc 00 0f 00 00 	mov    DWORD PTR [ebp-0x4],0xf00
   914bf:	eb 19                	jmp    914da <scroll_sceen+0xf9>
   914c1:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   914c4:	03 45 f8             	add    eax,DWORD PTR [ebp-0x8]
   914c7:	c6 00 00             	mov    BYTE PTR [eax],0x0
   914ca:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   914cd:	83 c0 01             	add    eax,0x1
   914d0:	03 45 f8             	add    eax,DWORD PTR [ebp-0x8]
   914d3:	c6 00 07             	mov    BYTE PTR [eax],0x7
   914d6:	83 45 fc 02          	add    DWORD PTR [ebp-0x4],0x2
   914da:	81 7d fc 9f 0f 00 00 	cmp    DWORD PTR [ebp-0x4],0xf9f
   914e1:	7e de                	jle    914c1 <scroll_sceen+0xe0>
   914e3:	a1 2c 2d 09 00       	mov    eax,ds:0x92d2c
   914e8:	83 e8 01             	sub    eax,0x1
   914eb:	a3 2c 2d 09 00       	mov    ds:0x92d2c,eax
   914f0:	e8 e7 fe ff ff       	call   913dc <flush>
   914f5:	c9                   	leave  
   914f6:	c3                   	ret    

000914f7 <put_char>:
   914f7:	55                   	push   ebp
   914f8:	89 e5                	mov    ebp,esp
   914fa:	83 ec 18             	sub    esp,0x18
   914fd:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   91500:	88 45 ec             	mov    BYTE PTR [ebp-0x14],al
   91503:	8b 15 2c 2d 09 00    	mov    edx,DWORD PTR ds:0x92d2c
   91509:	89 d0                	mov    eax,edx
   9150b:	c1 e0 02             	shl    eax,0x2
   9150e:	01 d0                	add    eax,edx
   91510:	c1 e0 04             	shl    eax,0x4
   91513:	89 c2                	mov    edx,eax
   91515:	a1 28 2d 09 00       	mov    eax,ds:0x92d28
   9151a:	01 d0                	add    eax,edx
   9151c:	05 00 c0 05 00       	add    eax,0x5c000
   91521:	01 c0                	add    eax,eax
   91523:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
   91526:	0f be 45 ec          	movsx  eax,BYTE PTR [ebp-0x14]
   9152a:	83 f8 0a             	cmp    eax,0xa
   9152d:	74 13                	je     91542 <put_char+0x4b>
   9152f:	83 f8 0d             	cmp    eax,0xd
   91532:	0f 84 e7 00 00 00    	je     9161f <put_char+0x128>
   91538:	83 f8 08             	cmp    eax,0x8
   9153b:	74 31                	je     9156e <put_char+0x77>
   9153d:	e9 9a 00 00 00       	jmp    915dc <put_char+0xe5>
   91542:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91545:	c6 00 00             	mov    BYTE PTR [eax],0x0
   91548:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   9154c:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9154f:	c6 00 07             	mov    BYTE PTR [eax],0x7
   91552:	c7 05 28 2d 09 00 00 	mov    DWORD PTR ds:0x92d28,0x0
   91559:	00 00 00 
   9155c:	a1 2c 2d 09 00       	mov    eax,ds:0x92d2c
   91561:	83 c0 01             	add    eax,0x1
   91564:	a3 2c 2d 09 00       	mov    ds:0x92d2c,eax
   91569:	e9 b8 00 00 00       	jmp    91626 <put_char+0x12f>
   9156e:	a1 28 2d 09 00       	mov    eax,ds:0x92d28
   91573:	85 c0                	test   eax,eax
   91575:	0f 88 a7 00 00 00    	js     91622 <put_char+0x12b>
   9157b:	a1 2c 2d 09 00       	mov    eax,ds:0x92d2c
   91580:	85 c0                	test   eax,eax
   91582:	0f 88 9a 00 00 00    	js     91622 <put_char+0x12b>
   91588:	a1 28 2d 09 00       	mov    eax,ds:0x92d28
   9158d:	83 e8 01             	sub    eax,0x1
   91590:	a3 28 2d 09 00       	mov    ds:0x92d28,eax
   91595:	a1 28 2d 09 00       	mov    eax,ds:0x92d28
   9159a:	85 c0                	test   eax,eax
   9159c:	79 2a                	jns    915c8 <put_char+0xd1>
   9159e:	c7 05 28 2d 09 00 4f 	mov    DWORD PTR ds:0x92d28,0x4f
   915a5:	00 00 00 
   915a8:	a1 2c 2d 09 00       	mov    eax,ds:0x92d2c
   915ad:	83 e8 01             	sub    eax,0x1
   915b0:	a3 2c 2d 09 00       	mov    ds:0x92d2c,eax
   915b5:	a1 2c 2d 09 00       	mov    eax,ds:0x92d2c
   915ba:	85 c0                	test   eax,eax
   915bc:	79 0a                	jns    915c8 <put_char+0xd1>
   915be:	c7 05 2c 2d 09 00 00 	mov    DWORD PTR ds:0x92d2c,0x0
   915c5:	00 00 00 
   915c8:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   915cb:	83 e8 02             	sub    eax,0x2
   915ce:	c6 00 00             	mov    BYTE PTR [eax],0x0
   915d1:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   915d4:	83 e8 01             	sub    eax,0x1
   915d7:	c6 00 07             	mov    BYTE PTR [eax],0x7
   915da:	eb 46                	jmp    91622 <put_char+0x12b>
   915dc:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   915df:	0f b6 55 ec          	movzx  edx,BYTE PTR [ebp-0x14]
   915e3:	88 10                	mov    BYTE PTR [eax],dl
   915e5:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   915e9:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   915ec:	c6 00 07             	mov    BYTE PTR [eax],0x7
   915ef:	a1 28 2d 09 00       	mov    eax,ds:0x92d28
   915f4:	83 c0 01             	add    eax,0x1
   915f7:	a3 28 2d 09 00       	mov    ds:0x92d28,eax
   915fc:	a1 28 2d 09 00       	mov    eax,ds:0x92d28
   91601:	83 f8 4f             	cmp    eax,0x4f
   91604:	7e 1f                	jle    91625 <put_char+0x12e>
   91606:	c7 05 28 2d 09 00 00 	mov    DWORD PTR ds:0x92d28,0x0
   9160d:	00 00 00 
   91610:	a1 2c 2d 09 00       	mov    eax,ds:0x92d2c
   91615:	83 c0 01             	add    eax,0x1
   91618:	a3 2c 2d 09 00       	mov    ds:0x92d2c,eax
   9161d:	eb 06                	jmp    91625 <put_char+0x12e>
   9161f:	90                   	nop
   91620:	eb 04                	jmp    91626 <put_char+0x12f>
   91622:	90                   	nop
   91623:	eb 01                	jmp    91626 <put_char+0x12f>
   91625:	90                   	nop
   91626:	eb 0c                	jmp    91634 <put_char+0x13d>
   91628:	c7 04 24 01 00 00 00 	mov    DWORD PTR [esp],0x1
   9162f:	e8 ad fd ff ff       	call   913e1 <scroll_sceen>
   91634:	a1 2c 2d 09 00       	mov    eax,ds:0x92d2c
   91639:	83 f8 18             	cmp    eax,0x18
   9163c:	7f ea                	jg     91628 <put_char+0x131>
   9163e:	e8 99 fd ff ff       	call   913dc <flush>
   91643:	c9                   	leave  
   91644:	c3                   	ret    

00091645 <print_str>:
   91645:	55                   	push   ebp
   91646:	89 e5                	mov    ebp,esp
   91648:	83 ec 04             	sub    esp,0x4
   9164b:	eb 15                	jmp    91662 <print_str+0x1d>
   9164d:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   91650:	0f b6 00             	movzx  eax,BYTE PTR [eax]
   91653:	0f be c0             	movsx  eax,al
   91656:	89 04 24             	mov    DWORD PTR [esp],eax
   91659:	e8 99 fe ff ff       	call   914f7 <put_char>
   9165e:	83 45 08 01          	add    DWORD PTR [ebp+0x8],0x1
   91662:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   91665:	0f b6 00             	movzx  eax,BYTE PTR [eax]
   91668:	84 c0                	test   al,al
   9166a:	75 e1                	jne    9164d <print_str+0x8>
   9166c:	c9                   	leave  
   9166d:	c3                   	ret    

0009166e <print_int>:
   9166e:	55                   	push   ebp
   9166f:	89 e5                	mov    ebp,esp
   91671:	83 ec 38             	sub    esp,0x38
   91674:	c7 44 24 08 18 00 00 	mov    DWORD PTR [esp+0x8],0x18
   9167b:	00 
   9167c:	c7 44 24 04 00 00 00 	mov    DWORD PTR [esp+0x4],0x0
   91683:	00 
   91684:	8d 45 e0             	lea    eax,[ebp-0x20]
   91687:	89 04 24             	mov    DWORD PTR [esp],eax
   9168a:	e8 6f 00 00 00       	call   916fe <memset>
   9168f:	c7 44 24 08 0a 00 00 	mov    DWORD PTR [esp+0x8],0xa
   91696:	00 
   91697:	8d 45 e0             	lea    eax,[ebp-0x20]
   9169a:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
   9169e:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   916a1:	89 04 24             	mov    DWORD PTR [esp],eax
   916a4:	e8 37 01 00 00       	call   917e0 <itoa>
   916a9:	8d 45 e0             	lea    eax,[ebp-0x20]
   916ac:	89 04 24             	mov    DWORD PTR [esp],eax
   916af:	e8 91 ff ff ff       	call   91645 <print_str>
   916b4:	c9                   	leave  
   916b5:	c3                   	ret    

000916b6 <print_hex>:
   916b6:	55                   	push   ebp
   916b7:	89 e5                	mov    ebp,esp
   916b9:	83 ec 38             	sub    esp,0x38
   916bc:	c7 44 24 08 18 00 00 	mov    DWORD PTR [esp+0x8],0x18
   916c3:	00 
   916c4:	c7 44 24 04 00 00 00 	mov    DWORD PTR [esp+0x4],0x0
   916cb:	00 
   916cc:	8d 45 e0             	lea    eax,[ebp-0x20]
   916cf:	89 04 24             	mov    DWORD PTR [esp],eax
   916d2:	e8 27 00 00 00       	call   916fe <memset>
   916d7:	c7 44 24 08 10 00 00 	mov    DWORD PTR [esp+0x8],0x10
   916de:	00 
   916df:	8d 45 e0             	lea    eax,[ebp-0x20]
   916e2:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
   916e6:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   916e9:	89 04 24             	mov    DWORD PTR [esp],eax
   916ec:	e8 a6 01 00 00       	call   91897 <uitoa>
   916f1:	8d 45 e0             	lea    eax,[ebp-0x20]
   916f4:	89 04 24             	mov    DWORD PTR [esp],eax
   916f7:	e8 49 ff ff ff       	call   91645 <print_str>
   916fc:	c9                   	leave  
   916fd:	c3                   	ret    

000916fe <memset>:
   916fe:	55                   	push   ebp
   916ff:	89 e5                	mov    ebp,esp
   91701:	83 ec 14             	sub    esp,0x14
   91704:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   91707:	88 45 ec             	mov    BYTE PTR [ebp-0x14],al
   9170a:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   9170d:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
   91710:	eb 11                	jmp    91723 <memset+0x25>
   91712:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91715:	0f b6 55 ec          	movzx  edx,BYTE PTR [ebp-0x14]
   91719:	88 10                	mov    BYTE PTR [eax],dl
   9171b:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   9171f:	83 6d 10 01          	sub    DWORD PTR [ebp+0x10],0x1
   91723:	83 7d 10 00          	cmp    DWORD PTR [ebp+0x10],0x0
   91727:	75 e9                	jne    91712 <memset+0x14>
   91729:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   9172c:	c9                   	leave  
   9172d:	c3                   	ret    

0009172e <memcpy>:
   9172e:	55                   	push   ebp
   9172f:	89 e5                	mov    ebp,esp
   91731:	83 ec 10             	sub    esp,0x10
   91734:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   91737:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
   9173a:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   9173d:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
   91740:	eb 13                	jmp    91755 <memcpy+0x27>
   91742:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   91745:	0f b6 10             	movzx  edx,BYTE PTR [eax]
   91748:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9174b:	88 10                	mov    BYTE PTR [eax],dl
   9174d:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   91751:	83 45 f8 01          	add    DWORD PTR [ebp-0x8],0x1
   91755:	83 7d 10 00          	cmp    DWORD PTR [ebp+0x10],0x0
   91759:	0f 95 c0             	setne  al
   9175c:	83 6d 10 01          	sub    DWORD PTR [ebp+0x10],0x1
   91760:	84 c0                	test   al,al
   91762:	75 de                	jne    91742 <memcpy+0x14>
   91764:	c9                   	leave  
   91765:	c3                   	ret    

00091766 <memcmp>:
   91766:	55                   	push   ebp
   91767:	89 e5                	mov    ebp,esp
   91769:	83 ec 10             	sub    esp,0x10
   9176c:	83 7d 08 00          	cmp    DWORD PTR [ebp+0x8],0x0
   91770:	74 06                	je     91778 <memcmp+0x12>
   91772:	83 7d 0c 00          	cmp    DWORD PTR [ebp+0xc],0x0
   91776:	75 0e                	jne    91786 <memcmp+0x20>
   91778:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
   9177b:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   9177e:	89 d1                	mov    ecx,edx
   91780:	29 c1                	sub    ecx,eax
   91782:	89 c8                	mov    eax,ecx
   91784:	eb 58                	jmp    917de <memcmp+0x78>
   91786:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   91789:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
   9178c:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   9178f:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
   91792:	c7 45 f4 00 00 00 00 	mov    DWORD PTR [ebp-0xc],0x0
   91799:	eb 36                	jmp    917d1 <memcmp+0x6b>
   9179b:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9179e:	0f b6 10             	movzx  edx,BYTE PTR [eax]
   917a1:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   917a4:	0f b6 00             	movzx  eax,BYTE PTR [eax]
   917a7:	38 c2                	cmp    dl,al
   917a9:	74 1a                	je     917c5 <memcmp+0x5f>
   917ab:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   917ae:	0f b6 00             	movzx  eax,BYTE PTR [eax]
   917b1:	0f be d0             	movsx  edx,al
   917b4:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   917b7:	0f b6 00             	movzx  eax,BYTE PTR [eax]
   917ba:	0f be c0             	movsx  eax,al
   917bd:	89 d1                	mov    ecx,edx
   917bf:	29 c1                	sub    ecx,eax
   917c1:	89 c8                	mov    eax,ecx
   917c3:	eb 19                	jmp    917de <memcmp+0x78>
   917c5:	83 45 f4 01          	add    DWORD PTR [ebp-0xc],0x1
   917c9:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   917cd:	83 45 f8 01          	add    DWORD PTR [ebp-0x8],0x1
   917d1:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   917d4:	3b 45 10             	cmp    eax,DWORD PTR [ebp+0x10]
   917d7:	7c c2                	jl     9179b <memcmp+0x35>
   917d9:	b8 00 00 00 00       	mov    eax,0x0
   917de:	c9                   	leave  
   917df:	c3                   	ret    

000917e0 <itoa>:
   917e0:	55                   	push   ebp
   917e1:	89 e5                	mov    ebp,esp
   917e3:	83 ec 10             	sub    esp,0x10
   917e6:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   917e9:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
   917ec:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   917ef:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
   917f2:	83 7d 08 00          	cmp    DWORD PTR [ebp+0x8],0x0
   917f6:	79 5b                	jns    91853 <itoa+0x73>
   917f8:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   917fb:	c6 00 2d             	mov    BYTE PTR [eax],0x2d
   917fe:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   91802:	83 45 f8 01          	add    DWORD PTR [ebp-0x8],0x1
   91806:	f7 5d 08             	neg    DWORD PTR [ebp+0x8]
   91809:	eb 48                	jmp    91853 <itoa+0x73>
   9180b:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   9180e:	89 c2                	mov    edx,eax
   91810:	c1 fa 1f             	sar    edx,0x1f
   91813:	f7 7d 10             	idiv   DWORD PTR [ebp+0x10]
   91816:	89 d0                	mov    eax,edx
   91818:	89 45 f4             	mov    DWORD PTR [ebp-0xc],eax
   9181b:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   9181e:	89 c2                	mov    edx,eax
   91820:	c1 fa 1f             	sar    edx,0x1f
   91823:	f7 7d 10             	idiv   DWORD PTR [ebp+0x10]
   91826:	89 45 08             	mov    DWORD PTR [ebp+0x8],eax
   91829:	83 7d f4 09          	cmp    DWORD PTR [ebp-0xc],0x9
   9182d:	76 13                	jbe    91842 <itoa+0x62>
   9182f:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   91832:	83 c0 57             	add    eax,0x57
   91835:	89 c2                	mov    edx,eax
   91837:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9183a:	88 10                	mov    BYTE PTR [eax],dl
   9183c:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   91840:	eb 11                	jmp    91853 <itoa+0x73>
   91842:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   91845:	83 c0 30             	add    eax,0x30
   91848:	89 c2                	mov    edx,eax
   9184a:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9184d:	88 10                	mov    BYTE PTR [eax],dl
   9184f:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   91853:	83 7d 08 00          	cmp    DWORD PTR [ebp+0x8],0x0
   91857:	7f b2                	jg     9180b <itoa+0x2b>
   91859:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9185c:	c6 00 00             	mov    BYTE PTR [eax],0x0
   9185f:	83 6d fc 01          	sub    DWORD PTR [ebp-0x4],0x1
   91863:	eb 25                	jmp    9188a <itoa+0xaa>
   91865:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   91868:	0f b6 00             	movzx  eax,BYTE PTR [eax]
   9186b:	88 45 f3             	mov    BYTE PTR [ebp-0xd],al
   9186e:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91871:	0f b6 10             	movzx  edx,BYTE PTR [eax]
   91874:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   91877:	88 10                	mov    BYTE PTR [eax],dl
   91879:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9187c:	0f b6 55 f3          	movzx  edx,BYTE PTR [ebp-0xd]
   91880:	88 10                	mov    BYTE PTR [eax],dl
   91882:	83 45 f8 01          	add    DWORD PTR [ebp-0x8],0x1
   91886:	83 6d fc 01          	sub    DWORD PTR [ebp-0x4],0x1
   9188a:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   9188d:	3b 45 fc             	cmp    eax,DWORD PTR [ebp-0x4]
   91890:	72 d3                	jb     91865 <itoa+0x85>
   91892:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   91895:	c9                   	leave  
   91896:	c3                   	ret    

00091897 <uitoa>:
   91897:	55                   	push   ebp
   91898:	89 e5                	mov    ebp,esp
   9189a:	83 ec 14             	sub    esp,0x14
   9189d:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   918a0:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
   918a3:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   918a6:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
   918a9:	eb 4e                	jmp    918f9 <uitoa+0x62>
   918ab:	8b 4d 10             	mov    ecx,DWORD PTR [ebp+0x10]
   918ae:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   918b1:	ba 00 00 00 00       	mov    edx,0x0
   918b6:	f7 f1                	div    ecx
   918b8:	89 55 f4             	mov    DWORD PTR [ebp-0xc],edx
   918bb:	8b 55 10             	mov    edx,DWORD PTR [ebp+0x10]
   918be:	89 55 ec             	mov    DWORD PTR [ebp-0x14],edx
   918c1:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
   918c4:	ba 00 00 00 00       	mov    edx,0x0
   918c9:	f7 75 ec             	div    DWORD PTR [ebp-0x14]
   918cc:	89 45 08             	mov    DWORD PTR [ebp+0x8],eax
   918cf:	83 7d f4 09          	cmp    DWORD PTR [ebp-0xc],0x9
   918d3:	76 13                	jbe    918e8 <uitoa+0x51>
   918d5:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   918d8:	83 c0 57             	add    eax,0x57
   918db:	89 c2                	mov    edx,eax
   918dd:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   918e0:	88 10                	mov    BYTE PTR [eax],dl
   918e2:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   918e6:	eb 11                	jmp    918f9 <uitoa+0x62>
   918e8:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
   918eb:	83 c0 30             	add    eax,0x30
   918ee:	89 c2                	mov    edx,eax
   918f0:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   918f3:	88 10                	mov    BYTE PTR [eax],dl
   918f5:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   918f9:	83 7d 08 00          	cmp    DWORD PTR [ebp+0x8],0x0
   918fd:	75 ac                	jne    918ab <uitoa+0x14>
   918ff:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91902:	3b 45 0c             	cmp    eax,DWORD PTR [ebp+0xc]
   91905:	75 0a                	jne    91911 <uitoa+0x7a>
   91907:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   9190a:	c6 00 30             	mov    BYTE PTR [eax],0x30
   9190d:	83 45 fc 01          	add    DWORD PTR [ebp-0x4],0x1
   91911:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91914:	c6 00 00             	mov    BYTE PTR [eax],0x0
   91917:	83 6d fc 01          	sub    DWORD PTR [ebp-0x4],0x1
   9191b:	eb 25                	jmp    91942 <uitoa+0xab>
   9191d:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   91920:	0f b6 00             	movzx  eax,BYTE PTR [eax]
   91923:	88 45 f3             	mov    BYTE PTR [ebp-0xd],al
   91926:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91929:	0f b6 10             	movzx  edx,BYTE PTR [eax]
   9192c:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   9192f:	88 10                	mov    BYTE PTR [eax],dl
   91931:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
   91934:	0f b6 55 f3          	movzx  edx,BYTE PTR [ebp-0xd]
   91938:	88 10                	mov    BYTE PTR [eax],dl
   9193a:	83 45 f8 01          	add    DWORD PTR [ebp-0x8],0x1
   9193e:	83 6d fc 01          	sub    DWORD PTR [ebp-0x4],0x1
   91942:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
   91945:	3b 45 fc             	cmp    eax,DWORD PTR [ebp-0x4]
   91948:	72 d3                	jb     9191d <uitoa+0x86>
   9194a:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
   9194d:	c9                   	leave  
   9194e:	c3                   	ret    
   9194f:	90                   	nop

00091950 <setup_paging>:
   91950:	55                   	push   ebp
   91951:	89 e5                	mov    ebp,esp
   91953:	83 ec 38             	sub    esp,0x38
   91956:	c7 45 ec 02 80 0b 00 	mov    DWORD PTR [ebp-0x14],0xb8002
   9195d:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   91960:	c6 00 63             	mov    BYTE PTR [eax],0x63
   91963:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   91966:	83 c0 01             	add    eax,0x1
   91969:	c6 00 05             	mov    BYTE PTR [eax],0x5
   9196c:	c7 04 24 40 00 00 00 	mov    DWORD PTR [esp],0x40
   91973:	e8 7f fb ff ff       	call   914f7 <put_char>
   91978:	c7 04 24 23 00 00 00 	mov    DWORD PTR [esp],0x23
   9197f:	e8 73 fb ff ff       	call   914f7 <put_char>
   91984:	c7 04 24 cb 1a 09 00 	mov    DWORD PTR [esp],0x91acb
   9198b:	e8 b5 fc ff ff       	call   91645 <print_str>
   91990:	c7 45 ec 02 80 0b 00 	mov    DWORD PTR [ebp-0x14],0xb8002
   91997:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   9199a:	c6 00 64             	mov    BYTE PTR [eax],0x64
   9199d:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   919a0:	83 c0 01             	add    eax,0x1
   919a3:	c6 00 05             	mov    BYTE PTR [eax],0x5
   919a6:	c7 45 e8 00 10 20 00 	mov    DWORD PTR [ebp-0x18],0x201000
   919ad:	c7 45 e4 00 20 20 00 	mov    DWORD PTR [ebp-0x1c],0x202000
   919b4:	c7 44 24 08 00 10 00 	mov    DWORD PTR [esp+0x8],0x1000
   919bb:	00 
   919bc:	c7 44 24 04 00 00 00 	mov    DWORD PTR [esp+0x4],0x0
   919c3:	00 
   919c4:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
   919c7:	89 04 24             	mov    DWORD PTR [esp],eax
   919ca:	e8 2f fd ff ff       	call   916fe <memset>
   919cf:	c7 45 f4 00 00 00 00 	mov    DWORD PTR [ebp-0xc],0x0
   919d6:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [ebp-0x10],0x0
   919dd:	eb 1c                	jmp    919fb <setup_paging+0xab>
   919df:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
   919e2:	c1 e0 02             	shl    eax,0x2
   919e5:	03 45 e4             	add    eax,DWORD PTR [ebp-0x1c]
   919e8:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
   919eb:	83 ca 03             	or     edx,0x3
   919ee:	89 10                	mov    DWORD PTR [eax],edx
   919f0:	81 45 f4 00 10 00 00 	add    DWORD PTR [ebp-0xc],0x1000
   919f7:	83 45 f0 01          	add    DWORD PTR [ebp-0x10],0x1
   919fb:	81 7d f0 ff 07 00 00 	cmp    DWORD PTR [ebp-0x10],0x7ff
   91a02:	7e db                	jle    919df <setup_paging+0x8f>
   91a04:	8b 45 e4             	mov    eax,DWORD PTR [ebp-0x1c]
   91a07:	89 c2                	mov    edx,eax
   91a09:	83 ca 03             	or     edx,0x3
   91a0c:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
   91a0f:	89 10                	mov    DWORD PTR [eax],edx
   91a11:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
   91a14:	05 00 08 00 00       	add    eax,0x800
   91a19:	8b 55 e4             	mov    edx,DWORD PTR [ebp-0x1c]
   91a1c:	83 ca 03             	or     edx,0x3
   91a1f:	89 10                	mov    DWORD PTR [eax],edx
   91a21:	81 45 e4 00 10 00 00 	add    DWORD PTR [ebp-0x1c],0x1000
   91a28:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
   91a2b:	83 c0 04             	add    eax,0x4
   91a2e:	8b 55 e4             	mov    edx,DWORD PTR [ebp-0x1c]
   91a31:	83 ca 03             	or     edx,0x3
   91a34:	89 10                	mov    DWORD PTR [eax],edx
   91a36:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
   91a39:	05 04 08 00 00       	add    eax,0x804
   91a3e:	8b 55 e4             	mov    edx,DWORD PTR [ebp-0x1c]
   91a41:	83 ca 03             	or     edx,0x3
   91a44:	89 10                	mov    DWORD PTR [eax],edx
   91a46:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
   91a49:	05 fc 0f 00 00       	add    eax,0xffc
   91a4e:	8b 55 e8             	mov    edx,DWORD PTR [ebp-0x18]
   91a51:	83 ca 03             	or     edx,0x3
   91a54:	89 10                	mov    DWORD PTR [eax],edx
   91a56:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
   91a59:	89 04 24             	mov    DWORD PTR [esp],eax
   91a5c:	e8 33 f6 ff ff       	call   91094 <enable_paging>
   91a61:	c7 04 24 da 1a 09 00 	mov    DWORD PTR [esp],0x91ada
   91a68:	e8 d8 fb ff ff       	call   91645 <print_str>
   91a6d:	c7 45 ec 02 80 0b 00 	mov    DWORD PTR [ebp-0x14],0xb8002
   91a74:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   91a77:	c6 00 65             	mov    BYTE PTR [eax],0x65
   91a7a:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
   91a7d:	83 c0 01             	add    eax,0x1
   91a80:	c6 00 05             	mov    BYTE PTR [eax],0x5
   91a83:	c9                   	leave  
   91a84:	c3                   	ret    

Disassembly of section .rodata:

00091a88 <.rodata>:
   91a88:	73 65                	jae    91aef <setup_paging+0x19f>
   91a8a:	74 75                	je     91b01 <setup_paging+0x1b1>
   91a8c:	70 20                	jo     91aae <setup_paging+0x15e>
   91a8e:	6b 65 72 6e          	imul   esp,DWORD PTR [ebp+0x72],0x6e
   91a92:	65 6c                	gs ins BYTE PTR es:[edi],dx
   91a94:	2e 0a 00             	or     al,BYTE PTR cs:[eax]
   91a97:	7f 45                	jg     91ade <setup_paging+0x18e>
   91a99:	4c                   	dec    esp
   91a9a:	46                   	inc    esi
   91a9b:	01 01                	add    DWORD PTR [ecx],eax
   91a9d:	01 00                	add    DWORD PTR [eax],eax
   91a9f:	00 73 65             	add    BYTE PTR [ebx+0x65],dh
   91aa2:	74 75                	je     91b19 <setup_paging+0x1c9>
   91aa4:	70 5f                	jo     91b05 <setup_paging+0x1b5>
   91aa6:	6b 65 72 6e          	imul   esp,DWORD PTR [ebp+0x72],0x6e
   91aaa:	65 6c                	gs ins BYTE PTR es:[edi],dx
   91aac:	3a 20                	cmp    ah,BYTE PTR [eax]
   91aae:	6b 65 72 6e          	imul   esp,DWORD PTR [ebp+0x72],0x6e
   91ab2:	65 6c                	gs ins BYTE PTR es:[edi],dx
   91ab4:	20 66 69             	and    BYTE PTR [esi+0x69],ah
   91ab7:	6c                   	ins    BYTE PTR es:[edi],dx
   91ab8:	65 20 66 6f          	and    BYTE PTR gs:[esi+0x6f],ah
   91abc:	72 6d                	jb     91b2b <setup_paging+0x1db>
   91abe:	61                   	popa   
   91abf:	74 20                	je     91ae1 <setup_paging+0x191>
   91ac1:	6e                   	outs   dx,BYTE PTR ds:[esi]
   91ac2:	6f                   	outs   dx,DWORD PTR ds:[esi]
   91ac3:	74 20                	je     91ae5 <setup_paging+0x195>
   91ac5:	65 6c                	gs ins BYTE PTR es:[edi],dx
   91ac7:	66 21 0a             	and    WORD PTR [edx],cx
   91aca:	00 73 65             	add    BYTE PTR [ebx+0x65],dh
   91acd:	74 75                	je     91b44 <setup_paging+0x1f4>
   91acf:	70 5f                	jo     91b30 <setup_paging+0x1e0>
   91ad1:	70 61                	jo     91b34 <setup_paging+0x1e4>
   91ad3:	67 69 6e 67 2e 0a 00 	imul   ebp,DWORD PTR [bp+0x67],0x65000a2e
   91ada:	65 
   91adb:	6e                   	outs   dx,BYTE PTR ds:[esi]
   91adc:	61                   	popa   
   91add:	62 6c 65 20          	bound  ebp,QWORD PTR [ebp+eiz*2+0x20]
   91ae1:	70 61                	jo     91b44 <setup_paging+0x1f4>
   91ae3:	67 65 20 6d 6f       	and    BYTE PTR gs:[di+0x6f],ch
   91ae8:	64 65 20 64 6f 6e    	fs and BYTE PTR gs:[edi+ebp*2+0x6e],ah
   91aee:	65 2e 0a 00          	gs or  al,BYTE PTR cs:[eax]

Disassembly of section .eh_frame:

00092af4 <.eh_frame>:
   92af4:	10 00                	adc    BYTE PTR [eax],al
   92af6:	00 00                	add    BYTE PTR [eax],al
   92af8:	00 00                	add    BYTE PTR [eax],al
   92afa:	00 00                	add    BYTE PTR [eax],al
   92afc:	01 00                	add    DWORD PTR [eax],eax
   92afe:	01 7c 08 0c          	add    DWORD PTR [eax+ecx*1+0xc],edi
   92b02:	04 04                	add    al,0x4
   92b04:	88 01                	mov    BYTE PTR [ecx],al
   92b06:	00 00                	add    BYTE PTR [eax],al
   92b08:	1c 00                	sbb    al,0x0
   92b0a:	00 00                	add    BYTE PTR [eax],al
   92b0c:	18 00                	sbb    BYTE PTR [eax],al
   92b0e:	00 00                	add    BYTE PTR [eax],al
   92b10:	a8 10                	test   al,0x10
   92b12:	09 00                	or     DWORD PTR [eax],eax
   92b14:	2f                   	das    
   92b15:	00 00                	add    BYTE PTR [eax],al
   92b17:	00 41 0e             	add    BYTE PTR [ecx+0xe],al
   92b1a:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92b20:	6b c5 0c             	imul   eax,ebp,0xc
   92b23:	04 04                	add    al,0x4
   92b25:	00 00                	add    BYTE PTR [eax],al
   92b27:	00 1c 00             	add    BYTE PTR [eax+eax*1],bl
   92b2a:	00 00                	add    BYTE PTR [eax],al
   92b2c:	38 00                	cmp    BYTE PTR [eax],al
   92b2e:	00 00                	add    BYTE PTR [eax],al
   92b30:	d7                   	xlat   BYTE PTR ds:[ebx]
   92b31:	10 09                	adc    BYTE PTR [ecx],cl
   92b33:	00 b6 01 00 00 41    	add    BYTE PTR [esi+0x41000001],dh
   92b39:	0e                   	push   cs
   92b3a:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92b40:	03 b2 01 c5 0c 04    	add    esi,DWORD PTR [edx+0x40cc501]
   92b46:	04 00                	add    al,0x0
   92b48:	14 00                	adc    al,0x0
   92b4a:	00 00                	add    BYTE PTR [eax],al
   92b4c:	58                   	pop    eax
   92b4d:	00 00                	add    BYTE PTR [eax],al
   92b4f:	00 8d 12 09 00 4e    	add    BYTE PTR [ebp+0x4e000912],cl
   92b55:	00 00                	add    BYTE PTR [eax],al
   92b57:	00 41 0e             	add    BYTE PTR [ecx+0xe],al
   92b5a:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92b60:	1c 00                	sbb    al,0x0
   92b62:	00 00                	add    BYTE PTR [eax],al
   92b64:	70 00                	jo     92b66 <setup_paging+0x1216>
   92b66:	00 00                	add    BYTE PTR [eax],al
   92b68:	db 12                	fist   DWORD PTR [edx]
   92b6a:	09 00                	or     DWORD PTR [eax],eax
   92b6c:	91                   	xchg   ecx,eax
   92b6d:	00 00                	add    BYTE PTR [eax],al
   92b6f:	00 41 0e             	add    BYTE PTR [ecx+0xe],al
   92b72:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92b78:	02 8d c5 0c 04 04    	add    cl,BYTE PTR [ebp+0x4040cc5]
   92b7e:	00 00                	add    BYTE PTR [eax],al
   92b80:	1c 00                	sbb    al,0x0
   92b82:	00 00                	add    BYTE PTR [eax],al
   92b84:	90                   	nop
   92b85:	00 00                	add    BYTE PTR [eax],al
   92b87:	00 6c 13 09          	add    BYTE PTR [ebx+edx*1+0x9],ch
   92b8b:	00 70 00             	add    BYTE PTR [eax+0x0],dh
   92b8e:	00 00                	add    BYTE PTR [eax],al
   92b90:	41                   	inc    ecx
   92b91:	0e                   	push   cs
   92b92:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92b98:	02 6c c5 0c          	add    ch,BYTE PTR [ebp+eax*8+0xc]
   92b9c:	04 04                	add    al,0x4
   92b9e:	00 00                	add    BYTE PTR [eax],al
   92ba0:	1c 00                	sbb    al,0x0
   92ba2:	00 00                	add    BYTE PTR [eax],al
   92ba4:	b0 00                	mov    al,0x0
   92ba6:	00 00                	add    BYTE PTR [eax],al
   92ba8:	dc 13                	fcom   QWORD PTR [ebx]
   92baa:	09 00                	or     DWORD PTR [eax],eax
   92bac:	05 00 00 00 41       	add    eax,0x41000000
   92bb1:	0e                   	push   cs
   92bb2:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92bb8:	41                   	inc    ecx
   92bb9:	0c 04                	or     al,0x4
   92bbb:	04 c5                	add    al,0xc5
   92bbd:	00 00                	add    BYTE PTR [eax],al
   92bbf:	00 1c 00             	add    BYTE PTR [eax+eax*1],bl
   92bc2:	00 00                	add    BYTE PTR [eax],al
   92bc4:	d0 00                	rol    BYTE PTR [eax],1
   92bc6:	00 00                	add    BYTE PTR [eax],al
   92bc8:	e1 13                	loope  92bdd <setup_paging+0x128d>
   92bca:	09 00                	or     DWORD PTR [eax],eax
   92bcc:	16                   	push   ss
   92bcd:	01 00                	add    DWORD PTR [eax],eax
   92bcf:	00 41 0e             	add    BYTE PTR [ecx+0xe],al
   92bd2:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92bd8:	03 12                	add    edx,DWORD PTR [edx]
   92bda:	01 c5                	add    ebp,eax
   92bdc:	0c 04                	or     al,0x4
   92bde:	04 00                	add    al,0x0
   92be0:	1c 00                	sbb    al,0x0
   92be2:	00 00                	add    BYTE PTR [eax],al
   92be4:	f0 00 00             	lock add BYTE PTR [eax],al
   92be7:	00 f7                	add    bh,dh
   92be9:	14 09                	adc    al,0x9
   92beb:	00 4e 01             	add    BYTE PTR [esi+0x1],cl
   92bee:	00 00                	add    BYTE PTR [eax],al
   92bf0:	41                   	inc    ecx
   92bf1:	0e                   	push   cs
   92bf2:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92bf8:	03 4a 01             	add    ecx,DWORD PTR [edx+0x1]
   92bfb:	c5 0c 04             	lds    ecx,FWORD PTR [esp+eax*1]
   92bfe:	04 00                	add    al,0x0
   92c00:	1c 00                	sbb    al,0x0
   92c02:	00 00                	add    BYTE PTR [eax],al
   92c04:	10 01                	adc    BYTE PTR [ecx],al
   92c06:	00 00                	add    BYTE PTR [eax],al
   92c08:	45                   	inc    ebp
   92c09:	16                   	push   ss
   92c0a:	09 00                	or     DWORD PTR [eax],eax
   92c0c:	29 00                	sub    DWORD PTR [eax],eax
   92c0e:	00 00                	add    BYTE PTR [eax],al
   92c10:	41                   	inc    ecx
   92c11:	0e                   	push   cs
   92c12:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92c18:	65 c5 0c 04          	lds    ecx,FWORD PTR gs:[esp+eax*1]
   92c1c:	04 00                	add    al,0x0
   92c1e:	00 00                	add    BYTE PTR [eax],al
   92c20:	1c 00                	sbb    al,0x0
   92c22:	00 00                	add    BYTE PTR [eax],al
   92c24:	30 01                	xor    BYTE PTR [ecx],al
   92c26:	00 00                	add    BYTE PTR [eax],al
   92c28:	6e                   	outs   dx,BYTE PTR ds:[esi]
   92c29:	16                   	push   ss
   92c2a:	09 00                	or     DWORD PTR [eax],eax
   92c2c:	48                   	dec    eax
   92c2d:	00 00                	add    BYTE PTR [eax],al
   92c2f:	00 41 0e             	add    BYTE PTR [ecx+0xe],al
   92c32:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92c38:	02 44 c5 0c          	add    al,BYTE PTR [ebp+eax*8+0xc]
   92c3c:	04 04                	add    al,0x4
   92c3e:	00 00                	add    BYTE PTR [eax],al
   92c40:	1c 00                	sbb    al,0x0
   92c42:	00 00                	add    BYTE PTR [eax],al
   92c44:	50                   	push   eax
   92c45:	01 00                	add    DWORD PTR [eax],eax
   92c47:	00 b6 16 09 00 48    	add    BYTE PTR [esi+0x48000916],dh
   92c4d:	00 00                	add    BYTE PTR [eax],al
   92c4f:	00 41 0e             	add    BYTE PTR [ecx+0xe],al
   92c52:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92c58:	02 44 c5 0c          	add    al,BYTE PTR [ebp+eax*8+0xc]
   92c5c:	04 04                	add    al,0x4
   92c5e:	00 00                	add    BYTE PTR [eax],al
   92c60:	1c 00                	sbb    al,0x0
   92c62:	00 00                	add    BYTE PTR [eax],al
   92c64:	70 01                	jo     92c67 <setup_paging+0x1317>
   92c66:	00 00                	add    BYTE PTR [eax],al
   92c68:	fe                   	(bad)  
   92c69:	16                   	push   ss
   92c6a:	09 00                	or     DWORD PTR [eax],eax
   92c6c:	30 00                	xor    BYTE PTR [eax],al
   92c6e:	00 00                	add    BYTE PTR [eax],al
   92c70:	41                   	inc    ecx
   92c71:	0e                   	push   cs
   92c72:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92c78:	6c                   	ins    BYTE PTR es:[edi],dx
   92c79:	c5 0c 04             	lds    ecx,FWORD PTR [esp+eax*1]
   92c7c:	04 00                	add    al,0x0
   92c7e:	00 00                	add    BYTE PTR [eax],al
   92c80:	1c 00                	sbb    al,0x0
   92c82:	00 00                	add    BYTE PTR [eax],al
   92c84:	90                   	nop
   92c85:	01 00                	add    DWORD PTR [eax],eax
   92c87:	00 2e                	add    BYTE PTR [esi],ch
   92c89:	17                   	pop    ss
   92c8a:	09 00                	or     DWORD PTR [eax],eax
   92c8c:	38 00                	cmp    BYTE PTR [eax],al
   92c8e:	00 00                	add    BYTE PTR [eax],al
   92c90:	41                   	inc    ecx
   92c91:	0e                   	push   cs
   92c92:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92c98:	74 c5                	je     92c5f <setup_paging+0x130f>
   92c9a:	0c 04                	or     al,0x4
   92c9c:	04 00                	add    al,0x0
   92c9e:	00 00                	add    BYTE PTR [eax],al
   92ca0:	1c 00                	sbb    al,0x0
   92ca2:	00 00                	add    BYTE PTR [eax],al
   92ca4:	b0 01                	mov    al,0x1
   92ca6:	00 00                	add    BYTE PTR [eax],al
   92ca8:	66 17                	popw   ss
   92caa:	09 00                	or     DWORD PTR [eax],eax
   92cac:	7a 00                	jp     92cae <setup_paging+0x135e>
   92cae:	00 00                	add    BYTE PTR [eax],al
   92cb0:	41                   	inc    ecx
   92cb1:	0e                   	push   cs
   92cb2:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92cb8:	02 76 c5             	add    dh,BYTE PTR [esi-0x3b]
   92cbb:	0c 04                	or     al,0x4
   92cbd:	04 00                	add    al,0x0
   92cbf:	00 1c 00             	add    BYTE PTR [eax+eax*1],bl
   92cc2:	00 00                	add    BYTE PTR [eax],al
   92cc4:	d0 01                	rol    BYTE PTR [ecx],1
   92cc6:	00 00                	add    BYTE PTR [eax],al
   92cc8:	e0 17                	loopne 92ce1 <setup_paging+0x1391>
   92cca:	09 00                	or     DWORD PTR [eax],eax
   92ccc:	b7 00                	mov    bh,0x0
   92cce:	00 00                	add    BYTE PTR [eax],al
   92cd0:	41                   	inc    ecx
   92cd1:	0e                   	push   cs
   92cd2:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92cd8:	02 b3 c5 0c 04 04    	add    dh,BYTE PTR [ebx+0x4040cc5]
   92cde:	00 00                	add    BYTE PTR [eax],al
   92ce0:	1c 00                	sbb    al,0x0
   92ce2:	00 00                	add    BYTE PTR [eax],al
   92ce4:	f0 01 00             	lock add DWORD PTR [eax],eax
   92ce7:	00 97 18 09 00 b8    	add    BYTE PTR [edi-0x47fff6e8],dl
   92ced:	00 00                	add    BYTE PTR [eax],al
   92cef:	00 41 0e             	add    BYTE PTR [ecx+0xe],al
   92cf2:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92cf8:	02 b4 c5 0c 04 04 00 	add    dh,BYTE PTR [ebp+eax*8+0x4040c]
   92cff:	00 1c 00             	add    BYTE PTR [eax+eax*1],bl
   92d02:	00 00                	add    BYTE PTR [eax],al
   92d04:	10 02                	adc    BYTE PTR [edx],al
   92d06:	00 00                	add    BYTE PTR [eax],al
   92d08:	50                   	push   eax
   92d09:	19 09                	sbb    DWORD PTR [ecx],ecx
   92d0b:	00 35 01 00 00 41    	add    BYTE PTR ds:0x41000001,dh
   92d11:	0e                   	push   cs
   92d12:	08 85 02 42 0d 05    	or     BYTE PTR [ebp+0x50d4202],al
   92d18:	03 31                	add    esi,DWORD PTR [ecx]
   92d1a:	01 c5                	add    ebp,eax
   92d1c:	0c 04                	or     al,0x4
   92d1e:	04 00                	add    al,0x0

Disassembly of section .bss:

00092d20 <cx>:
   92d20:	00 00                	add    BYTE PTR [eax],al
	...

00092d24 <cy>:
   92d24:	00 00                	add    BYTE PTR [eax],al
	...

00092d28 <cursor_x>:
   92d28:	00 00                	add    BYTE PTR [eax],al
	...

00092d2c <cursor_y>:
   92d2c:	00 00                	add    BYTE PTR [eax],al
	...

Disassembly of section .comment:

00000000 <.comment>:
   0:	47                   	inc    edi
   1:	43                   	inc    ebx
   2:	43                   	inc    ebx
   3:	3a 20                	cmp    ah,BYTE PTR [eax]
   5:	28 47 4e             	sub    BYTE PTR [edi+0x4e],al
   8:	55                   	push   ebp
   9:	29 20                	sub    DWORD PTR [eax],esp
   b:	34 2e                	xor    al,0x2e
   d:	36 2e 31 00          	ss xor DWORD PTR cs:[eax],eax
