cirCmd.o: cirCmd.cpp cirMgr.h cirDef.h fec.h cirGate.h \
 ../../include/myHashMap.h cirCmd.h ../../include/cmdParser.h \
 ../../include/cmdCharDef.h ../../include/util.h ../../include/rnGen.h \
 ../../include/myUsage.h
cirFraig.o: cirFraig.cpp cirMgr.h cirDef.h fec.h cirGate.h \
 ../../include/myHashMap.h ../../include/sat.h ../../include/Solver.h \
 ../../include/SolverTypes.h ../../include/Global.h \
 ../../include/VarOrder.h ../../include/Heap.h ../../include/Proof.h \
 ../../include/File.h ../../include/util.h ../../include/rnGen.h \
 ../../include/myUsage.h
cirGate.o: cirGate.cpp cirGate.h cirDef.h ../../include/myHashMap.h fec.h \
 cirMgr.h ../../include/util.h ../../include/rnGen.h \
 ../../include/myUsage.h
cirMgr.o: cirMgr.cpp cirMgr.h cirDef.h fec.h cirGate.h \
 ../../include/myHashMap.h ../../include/util.h ../../include/rnGen.h \
 ../../include/myUsage.h
cirOpt.o: cirOpt.cpp cirMgr.h cirDef.h fec.h cirGate.h \
 ../../include/myHashMap.h ../../include/util.h ../../include/rnGen.h \
 ../../include/myUsage.h
cirSim.o: cirSim.cpp cirMgr.h cirDef.h fec.h cirGate.h \
 ../../include/myHashMap.h ../../include/util.h ../../include/rnGen.h \
 ../../include/myUsage.h
fec.o: fec.cpp fec.h cirDef.h cirGate.h ../../include/myHashMap.h
