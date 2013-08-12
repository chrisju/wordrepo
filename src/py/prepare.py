#!/usr/bin/env python3
#coding=utf8

import os
import sys

d = sys.argv[1]
print(d)

os.system('rm -rf ../data/tmp')
os.mkdir('../data/tmp')

for root,dirs,files in os.walk(d):
    for f in files:
        f1 = os.path.join(root,f)
        #print(f1)

        f2 = f1.replace('/','_')
        f2 = f2.replace('.','_')
        f2 = os.path.join('../data/tmp',f2)
        #print(f2)
        cmd = '%s "%s" "%s"' % ('./tidy',f1,f2)
        os.system(cmd)
