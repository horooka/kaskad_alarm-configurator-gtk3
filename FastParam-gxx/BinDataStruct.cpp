//---------------------------------------------------------------------------

#pragma pack(push, 1)
#include "BinDataStruct.h"
#pragma pack(pop)
#include "StationNames.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//---------------------------------------------------------------------------
StMachins *Machins = NULL;

const char *CfgMaMask = "/Station_";
const char *CfgPlus = "/Cfg/Kbin/";
const char *CfgOnePlus = "/Kbin/";
//---------------------------------------------------------------------------
std::string StGUID::GetStr() {
    char buf[40];
    unsigned int d1_val = *(unsigned int *)d1;
    unsigned short d2_val = *(unsigned short *)d2;
    unsigned short d3_val = *(unsigned short *)d3;
    snprintf(buf, sizeof(buf),
             "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", d1_val,
             d2_val, d3_val, (unsigned char)d4[0], (unsigned char)d4[1],
             (unsigned char)d5[0], (unsigned char)d5[1], (unsigned char)d5[2],
             (unsigned char)d5[3], (unsigned char)d5[4], (unsigned char)d5[5]);

    return std::string(buf);
}
// ������ ������ �������������
StControllers ReadControllers(std::string fname) {
    StControllers StCtrl;
    memset(&StCtrl, 0, sizeof(StControllers));
    FILE *f = fopen(fname.c_str(), "rb");
    if (f != NULL) {
        fread(&StCtrl.CntrlHead, sizeof(StCntrlHead), 1, f);
        if (StCtrl.CntrlHead.controlcount > 0) {
            StCtrl.Controllers =
                new StController[StCtrl.CntrlHead.controlcount];
            fread(StCtrl.Controllers, sizeof(StController),
                  StCtrl.CntrlHead.controlcount, f);
        }
        fclose(f);
    }
    return StCtrl;
}
// ������ ���������� � ����������
void ReadParams(std::string fname) {
    StParams StPrm;
    memset(&StPrm, 0, sizeof(StParams));
    FILE *f = fopen((char *)fname.c_str(), "rb");
    if (f != NULL) {
        fread((char *)(&StPrm.ParHead), sizeof(StParHead), 1, f);
        if (StPrm.ParHead.paramcount > 0) {
            StPrm.Params = new StParam[StPrm.ParHead.paramcount];
            if (StPrm.ParHead.divisor == 3) {
                fread(StPrm.Params8, sizeof(StParam8), StPrm.ParHead.paramcount,
                      f);
            } else if (StPrm.ParHead.divisor == 2) {
                fread(StPrm.Params4, sizeof(StParam4), StPrm.ParHead.paramcount,
                      f);
            } else if (StPrm.ParHead.divisor == 1) {
                fread(StPrm.Params2, sizeof(StParam2), StPrm.ParHead.paramcount,
                      f);
            } else {
                fread(StPrm.Params, sizeof(StParam), StPrm.ParHead.paramcount,
                      f);
            }
        }
        fclose(f);
    }
}
// ������ ������ ������������
void StControllers::ReadControllers(std::string fname, std::string grname) {
    memset(this, 0, sizeof(StControllers));
    MaName = grname;
    Controllers = NULL;
    FILE *f = fopen((char *)(fname + "KLogic.kbin").c_str(), "rb");
    if (f != NULL) {
        fread(&CntrlHead, sizeof(StCntrlHead), 1, f);
        if (CntrlHead.controlcount > 0) {
            CrtControllers(CntrlHead.controlcount);
            fread(Controllers, sizeof(StController), CntrlHead.controlcount, f);
            CrtStParams(CntrlHead.controlcount);
            CrtStTrees(CntrlHead.controlcount);
            CrtStParamsControl(CntrlHead.controlcount);
        }
        CrtIdx();
        fclose(f);
    }
    //
    ControllersIP = NULL;
    f = fopen((char *)(fname + "KLogicIP.kbin").c_str(), "rb");
    if (f != NULL) {
        fread(&CntrlHeadIP, sizeof(StCntrlHead), 1, f);
        if (CntrlHeadIP.controlcount > 0) {
            CrtControllersIP(CntrlHeadIP.controlcount);
            fread(ControllersIP, sizeof(StControllerIP),
                  CntrlHeadIP.controlcount, f);
        }
        fclose(f);
    }
}
// �������� ������� ������������
void StControllers::CrtIdx() {
    IdxCount = IdxFId = IdxLId = 0;
    if (CntrlHead.controlcount > 0) {
        IdxFId = Controllers[0].id;
        IdxLId = Controllers[CntrlHead.controlcount - 1].id;
        for (unsigned int i = 0; i < CntrlHead.controlcount; i++) {
            if (IdxFId > Controllers[i].id)
                IdxFId = Controllers[i].id;
            if (IdxLId < Controllers[i].id)
                IdxLId = Controllers[i].id;
        }
        IdxCount = IdxLId - IdxFId + 1;
        if (IdxCount >= 0xFFFF) {
            IdxCount = IdxFId = IdxLId = 0;
            MaNames = NULL;
        } else {
            MaNames = new std::string[CntrlHead.controlcount];
            CntrlIdIdx = new unsigned short int[IdxCount];
            memset(CntrlIdIdx, 0xFF, IdxCount * sizeof(short int));
            for (unsigned int i = 0; i < CntrlHead.controlcount; i++) {
                CntrlIdIdx[Controllers[i].id - IdxFId] = i;
                MaNames[i] = MaName + "\\\\" + Controllers[i].name;
            }
        }
    }
}
// ������� ����������
StController *StControllers::GetCntrl(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        int idx = CntrlIdIdx[id - IdxFId];
        if (idx != 0xFFFF)
            return &Controllers[idx];
    }
    return NULL;
}
// ������� ���������� IP
StControllerIP *StControllers::GetCntrlIP(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        int idx = CntrlIdIdx[id - IdxFId];
        if (idx != 0xFFFF && ControllersIP)
            return &ControllersIP[idx];
    }
    return NULL;
}
// ������� ����������
char *StControllers::GetMaName(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        int idx = CntrlIdIdx[id - IdxFId];
        if (idx != 0xFFFF)
            return const_cast<char *>(MaNames[idx].c_str());
    }
    return NULL;
}
// ������� ������
void *StControllers::GetRefData(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        int idx = CntrlIdIdx[id - IdxFId];
        if (idx != 0xFFFF)
            return RefsData[idx];
    }
    return NULL;
}
// ������� ������ ����������
StParams *StControllers::GetParams(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        int idx = CntrlIdIdx[id - IdxFId];
        if (idx != 0xFFFF)
            return &Params[idx];
    }
    return NULL;
}
// ������� ������
StTree *StControllers::GetTree(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        int idx = CntrlIdIdx[id - IdxFId];
        if (idx != 0xFFFF)
            return &Trees[idx];
    }
    return NULL;
}
// ������� ������ ��������� ����������
StParamsControl *StControllers::GetParamsControl(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        int idx = CntrlIdIdx[id - IdxFId];
        if (idx != 0xFFFF)
            return &ParamsControl[idx];
    }
    return NULL;
}
// ������� �� �������
int StControllers::GetCntrlIdIdx(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        return CntrlIdIdx[id - IdxFId];
    }
    return 0xFFFF;
}
// ������� �� �� ������
int StControllers::GetCntrlIdxId(unsigned int id) {
    if (IdxCount > 0 && id < IdxCount) {
        return Controllers[id].id;
    }
    return -1;
}
// ������� �� ����������� �� �����
int StControllers::GetCntrlIdByName(char *cname) {
    for (int i = 0; i < CntrlHead.controlcount; i++) {
        if (strcmp(&Controllers[i].name[0], cname) == 0)
            return Controllers[i].id;
    }
    return -1;
}
// ��������� � ������� �� �����������
int StControllers::CheckCntrlIdByName(char *cname, int cid) {
    StController *Cntrl = GetCntrl(cid);
    if (Cntrl && strcmp(&Cntrl->name[0], cname) == 0)
        return cid;
    return GetCntrlIdByName(cname);
}
// ��������� ��������� � ������
void StControllers::LoadParamsTree(std::string dname, unsigned int id,
                                   StGUID *lstg) {
    int i = GetCntrlIdIdx(id);
    if (i != 0xFFFF) {
        StGUID stg = Controllers[i].configid;
        if (lstg)
            stg = *lstg;
        if (!Params[i].loaded)
            Params[i].ReadParams(dname + stg.GetStr() + "_info.kbin");
        if (!Trees[i].loaded) {
            Trees[i].ReadTree2(dname + stg.GetStr() + "_tree.kbin");
            Trees[i].IdxCount = Params[i].IdxCount;
            Trees[i].IdxFId = Params[i].IdxFId;
            Trees[i].IdxLId = Params[i].IdxLId;
            Trees[i].ParamsRefIdx = Params[i].ParamsIdx;
            Trees[i].Cntrl = &Controllers[i];
            Trees[i].CrtIdx();
        }
        if (!ParamsControl[i].loaded)
            ParamsControl[i].ReadParams(dname + stg.GetStr() + "_cparam.kbin");
    }
}
// ��������� ��� ��������� � �������
void StControllers::LoadAllParamsTree(std::string dname) {
    for (int i = 0; i < CntrlHead.controlcount; i++) {
        if (!Params[i].loaded)
            Params[i].ReadParams(dname + Controllers[i].configid.GetStr() +
                                 "_info.kbin");
        if (!Trees[i].loaded) {
            Trees[i].ReadTree(dname + Controllers[i].configid.GetStr() +
                              "_tree.kbin");
            Trees[i].IdxCount = Params[i].IdxCount;
            Trees[i].IdxFId = Params[i].IdxFId;
            Trees[i].IdxLId = Params[i].IdxLId;
            Trees[i].ParamsRefIdx = Params[i].ParamsIdx;
            Trees[i].Cntrl = &Controllers[i];
            Trees[i].CrtIdx();
        }
        if (!ParamsControl[i].loaded)
            ParamsControl[i].ReadParams(
                dname + Controllers[i].configid.GetStr() + "_cparam.kbin");
    }
}
// ��������� ����������� ���������
bool StControllers::FillParamAddrStruct(unsigned int id, TPAA &ParamAddrArray,
                                        void *pGrpArr) {
    StTree *Tree = GetTree(id);
    if (Tree) {
        StParams *Param = GetParams(id);
        int nid = 0;
        if (ParamAddrArray.size() < Tree->ParamCount)
            ParamAddrArray.resize(Tree->ParamCount);
        for (int i = 0; i < Tree->TreeHead.childcount; i++) {
            RecParamAddrStruct(&Tree->TreeChilds[i], Param, ParamAddrArray,
                               (std::vector<TIntArr> *)pGrpArr, nid);
        }
        if ((unsigned int)Tree->ParamCount != (unsigned int)nid)
            ParamAddrArray.resize(nid);
        return true;
    }
    return false;
}
unsigned char arrouttype[] = {
    0, 0,  0, 0,  0, 0,  0, 0, 0,  0,   0, 0, 0, 0, 14, 0,   16, 0, 18, 0, 20,
    0, 22, 0, 24, 0, 26, 0, 0, 0,  0,   0, 0, 0, 0, 0,  0,   0,  0, 0,  0, 0,
    0, 0,  0, 0,  0, 0,  0, 0, 0,  0,   0, 0, 0, 0, 0,  0,   0,  0, 0,  0, 0,
    0, 0,  0, 0,  0, 0,  0, 0, 0,  0,   0, 0, 0, 0, 0,  0,   0,  0, 0,  0, 0,
    0, 0,  0, 0,  0, 0,  0, 0, 92, 0,   0, 0, 0, 0, 0,  0,   0,  0, 0,  0, 0,
    0, 0,  0, 0,  0, 0,  0, 0, 0,  0,   0, 0, 0, 0, 0,  120, 0,  0, 0,  0, 0,
    0, 0,  0, 0,  0, 0,  0, 0, 0,  135, 0, 0, 0, 0, 0,  0,   0,  0, 0,  0, 0,
    0, 0,  0, 0,  0, 0,  0, 0, 0,  0,   0, 0, 0, 0};
// ��������� ����������� ���������
void StControllers::RecParamAddrStruct(StTreeChild *TreeChild,
                                       StParams *TreeParam,
                                       TPAA &ParamAddrArray,
                                       std::vector<TIntArr> *pGrpArr,
                                       int &nid) {
    for (int i = 0; i < TreeChild->TreeChHead.paramcount; i++) {
        StDynParam DynParam;
        if (TreeParam->GetDynParam(TreeChild->params[i], &DynParam)) {
            ParamAddrArray[nid].TagID = DynParam.id;
            ParamAddrArray[nid].ModbusAddr = DynParam.gaddr;
            if (DynParam.ParamArchive) {
                ParamAddrArray[nid].ArchiveGUID =
                    *(StGUID *)&DynParam.ParamArchive->archiveid;
                ParamAddrArray[nid].NumberinArchive =
                    DynParam.ParamArchive->num;
            } else {
                memset(&ParamAddrArray[nid].ArchiveGUID, 0, sizeof(StGUID));
                ParamAddrArray[nid].NumberinArchive = 0;
            }
            if (DynParam.ParamInfo) {
                ParamAddrArray[nid].Multiplier = DynParam.ParamInfo->scale;
                ParamAddrArray[nid].Summand = DynParam.ParamInfo->offset;
                ParamAddrArray[nid].NechFrom = DynParam.ParamInfo->insensfrom;
                ParamAddrArray[nid].NechTo = DynParam.ParamInfo->insensto;
                ParamAddrArray[nid].NechVal = DynParam.ParamInfo->insensval;
            } else {
                ParamAddrArray[nid].Multiplier = 1;
                ParamAddrArray[nid].Summand = 0;
                ParamAddrArray[nid].NechFrom = 0;
                ParamAddrArray[nid].NechTo = 0;
                ParamAddrArray[nid].NechVal = 0;
            }
            ParamAddrArray[nid].Invert = DynParam.invert;
            ParamAddrArray[nid].Type = (DynParam.type == 3 ? 0 : DynParam.type);
            ParamAddrArray[nid].Write =
                (TreeChild->Task ? !arrouttype[DynParam.imgid]
                                 : arrouttype[DynParam.imgid]);
            if (DynParam.ParamScope) {
                ParamAddrArray[nid].AMA = DynParam.ParamScope->upcrash;
                ParamAddrArray[nid].AMI = DynParam.ParamScope->downcrash;
                ParamAddrArray[nid].PMA = DynParam.ParamScope->upwarning;
                ParamAddrArray[nid].PMI = DynParam.ParamScope->downwarning;
            } else {
                ParamAddrArray[nid].AMA = 0;
                ParamAddrArray[nid].AMI = 0;
                ParamAddrArray[nid].PMA = 0;
                ParamAddrArray[nid].PMI = 0;
            }
            ParamAddrArray[nid].KoefFilt = 100;
            nid++;
        }
    }
    for (int i = 0; i < TreeChild->TreeChHead.childcount; i++) {
        RecParamAddrStruct(&TreeChild->TreeChilds[i], TreeParam, ParamAddrArray,
                           pGrpArr, nid);
    }
}
//------------------------------------------------------------------------------
// ������ ���������� � ����������
void StParams::ReadParams(std::string fname) {
    memset(this, 0, sizeof(StParams));
    FILE *f = fopen((char *)fname.c_str(), "rb");
    if (f != NULL) {
        fread(&ParHead.verrev, 2, 1, f);
        fread(ParHead.service, 5, 1, f);
        fread(&ParHead.divisor, 1, 1, f);
        fread(&ParHead.paramcount, 2, 1, f);
        fread(&ParHead.paramcount2, 2, 1, f);
        fread(&ParHead.scopecount, 4, 1, f);
        fread(&ParHead.infocount, 4, 1, f);
        fread(&ParHead.archivecount, 4, 1, f);
        if (ParHead.paramcount > 0) {
            CrtParams(ParHead.paramcount);
            if (ParHead.divisor == 3) {
                fread(Params8, sizeof(StParam8), ParHead.paramcount, f);
            } else if (ParHead.divisor == 2) {
                fread(Params4, sizeof(StParam4), ParHead.paramcount, f);
            } else if (ParHead.divisor == 1) {
                fread(Params2, sizeof(StParam2), ParHead.paramcount, f);
            } else {
                fread(Params, sizeof(StParam), ParHead.paramcount, f);
            }
        }
        if (ParHead.scopecount > 0) {
            CrtParamsScope(ParHead.scopecount);
            fread(ParamScopes, sizeof(StParamScope), ParHead.scopecount, f);
        }
        if (ParHead.infocount > 0) {
            CrtParamsInfo(ParHead.infocount);
            fread(ParamInfos, sizeof(StParamInfo), ParHead.infocount, f);
        }
        if (ParHead.archivecount > 0) {
            CrtParamsArchive(ParHead.archivecount);
            fread(ParamArchives, sizeof(StParamArchive), ParHead.archivecount,
                  f);
        }
        CrtIdx();
        fclose(f);
    }
    loaded = 1;
}
// ��������� ������
void StParams::SetParamsDataIdx(unsigned long int id, unsigned long int ididx,
                                unsigned long int /*pid*/,
                                unsigned long int /*imgid*/, void *param,
                                const char *cipher) {
    ParamsIdx[ididx] = &ParamsDataIdx[id];
    ParamsDataIdx[id].param = param;
    ParamsDataIdx[id].FullName = cipher;
}
// �������� �������
void StParams::CrtIdx() {
    IdxCount = IdxFId = IdxLId = 0;
    IdxScopeCount = IdxScopeFId = IdxScopeLId = 0;
    if (ParHead.paramcount > 0) {
        if (ParHead.divisor == 3) {
            IdxFId = Params8[0].id;
            IdxLId = Params8[ParHead.paramcount - 1].id;
        } else if (ParHead.divisor == 2) {
            IdxFId = Params4[0].id;
            IdxLId = Params4[ParHead.paramcount - 1].id;
        } else if (ParHead.divisor == 1) {
            IdxFId = Params2[0].id;
            IdxLId = Params2[ParHead.paramcount - 1].id;
        } else {
            IdxFId = Params[0].id;
            IdxLId = Params[ParHead.paramcount - 1].id;
        }
        IdxCount = IdxLId - IdxFId + 1;
        if (IdxCount > 0xFFFFFF) {
            IdxCount = IdxFId = IdxLId = 0;
            ParamsIdx = NULL;
        } else {
            ParamsIdx = new StParamsDataIdx *[IdxCount];
            memset(ParamsIdx, 0, IdxCount * sizeof(void *));
            CrtParamDataIdxStruct(ParHead.paramcount);
            if (ParHead.divisor == 3) {
                for (unsigned int i = 0; i < ParHead.paramcount; i++) {
                    SetParamsDataIdx(i, Params8[i].id - IdxFId, Params8[i].id,
                                     Params8[i].imgid, &Params8[i],
                                     &Params8[i].cipher[0]);
                }
            } else if (ParHead.divisor == 2) {
                for (unsigned int i = 0; i < ParHead.paramcount; i++) {
                    SetParamsDataIdx(i, Params4[i].id - IdxFId, Params4[i].id,
                                     Params4[i].imgid, &Params4[i],
                                     &Params4[i].cipher[0]);
                }
            } else if (ParHead.divisor == 1) {
                for (unsigned int i = 0; i < ParHead.paramcount; i++) {
                    SetParamsDataIdx(i, Params2[i].id - IdxFId, Params2[i].id,
                                     Params2[i].imgid, &Params2[i],
                                     &Params2[i].cipher[0]);
                }
            } else {
                for (unsigned int i = 0; i < ParHead.paramcount; i++) {
                    SetParamsDataIdx(i, Params[i].id - IdxFId, Params[i].id,
                                     Params[i].imgid, &Params[i],
                                     &Params[i].cipher[0]);
                }
            }
        }
        if (ParHead.scopecount > 0) {
            IdxScopeFId = ParamScopes[0].id;
            IdxScopeLId = ParamScopes[ParHead.scopecount - 1].id;
            if (IdxScopeLId < IdxScopeFId) {
                IdxScopeCount = IdxScopeFId = IdxScopeLId = 0;
                ParamScopesIdx = NULL;
            } else {
                IdxScopeCount = IdxScopeLId - IdxScopeFId + 1;
                if (IdxScopeCount > 0xFFFFFF) {
                    IdxScopeCount = IdxScopeFId = IdxScopeLId = 0;
                    ParamScopesIdx = NULL;
                } else {
                    ParamScopesIdx = new StParamScope *[IdxScopeCount];
                    memset(ParamScopesIdx, 0,
                           IdxScopeCount * sizeof(StParamScope *));
                    if (ParamScopesIdx) {
                        for (unsigned int i = 0; i < ParHead.scopecount; i++) {
                            unsigned long scope_id = ParamScopes[i].id;
                            if (scope_id >= IdxScopeFId && scope_id <= IdxScopeLId) {
                                unsigned long idx = scope_id - IdxScopeFId;
                                if (idx < IdxScopeCount)
                                    ParamScopesIdx[idx] = &ParamScopes[i];
                            }
                        }
                    }
                }
            }
        }
        if (ParHead.infocount > 0) {
            IdxInfoFId = ParamInfos[0].id;
            IdxInfoLId = ParamInfos[ParHead.infocount - 1].id;
            if (IdxInfoLId < IdxInfoFId) {
                IdxInfoCount = IdxInfoFId = IdxInfoLId = 0;
                ParamInfoIdx = NULL;
            } else {
                IdxInfoCount = IdxInfoLId - IdxInfoFId + 1;
                if (IdxInfoCount > 0xFFFFFF) {
                    IdxInfoCount = IdxInfoFId = IdxInfoLId = 0;
                    ParamInfoIdx = NULL;
                } else {
                    ParamInfoIdx = new StParamInfo *[IdxInfoCount];
                    memset(ParamInfoIdx, 0, IdxInfoCount * sizeof(StParamInfo *));
                    if (ParamInfoIdx) {
                        for (unsigned int i = 0; i < ParHead.infocount; i++) {
                            unsigned long info_id = ParamInfos[i].id;
                            if (info_id >= IdxInfoFId && info_id <= IdxInfoLId) {
                                unsigned long idx = info_id - IdxInfoFId;
                                if (idx < IdxInfoCount)
                                    ParamInfoIdx[idx] = &ParamInfos[i];
                            }
                        }
                    }
                }
            }
        }
        if (ParHead.archivecount > 0) {
            IdxArchiveFId = ParamArchives[0].id;
            IdxArchiveLId = ParamArchives[ParHead.archivecount - 1].id;
            if (IdxArchiveLId < IdxArchiveFId) {
                IdxArchiveCount = IdxArchiveFId = IdxArchiveLId = 0;
                ParamArchivesIdx = NULL;
            } else {
                IdxArchiveCount = IdxArchiveLId - IdxArchiveFId + 1;
                if (IdxArchiveCount > 0xFFFFFF) {
                    IdxArchiveCount = IdxArchiveFId = IdxArchiveLId = 0;
                    ParamArchivesIdx = NULL;
                } else {
                    ParamArchivesIdx = new StParamArchive *[IdxArchiveCount];
                    memset(ParamArchivesIdx, 0,
                           IdxArchiveCount * sizeof(StParamArchive *));
                    if (ParamArchivesIdx) {
                        for (unsigned int i = 0; i < ParHead.archivecount; i++) {
                            unsigned long arch_id = ParamArchives[i].id;
                            if (arch_id >= IdxArchiveFId && arch_id <= IdxArchiveLId) {
                                unsigned long idx = arch_id - IdxArchiveFId;
                                if (idx < IdxArchiveCount)
                                    ParamArchivesIdx[idx] = &ParamArchives[i];
                            }
                        }
                    }
                }
            }
        }
    }
}
// ������� ��������
StParam *StParams::GetParam(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        return (StParam *)ParamsIdx[id - IdxFId];
    } else if (ParHead.paramcount > 0) {
        return GetParamBin(id, 0, ParHead.paramcount - 1);
    }
    return NULL;
}
// ������� �������
StParamScope *StParams::GetParamScope(unsigned int id) {
    if (IdxScopeCount > 0 && ParamScopesIdx && id >= IdxScopeFId && id <= IdxScopeLId) {
        return ParamScopesIdx[id - IdxScopeFId];
    }
    return NULL;
}
// ������� �������
StParamInfo *StParams::GetParamInfo(unsigned int id) {
    if (IdxInfoCount > 0 && id >= IdxInfoFId && id <= IdxInfoLId) {
        return ParamInfoIdx[id - IdxInfoFId];
    }
    return NULL;
}
// ������� �����
StParamArchive *StParams::GetParamArchive(unsigned int id) {
    if (IdxArchiveCount > 0 && id >= IdxArchiveFId && id <= IdxArchiveLId) {
        return ParamArchivesIdx[id - IdxArchiveFId];
    }
    return NULL;
}
// ������� ��������
bool StParams::GetDynParam(unsigned int id, StDynParam *DynParam) {
    try {
        if (id == 42) {
            id = id + 0;
        }
        if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
            if (!ParamsIdx[id - IdxFId])
                return false;
            if (ParHead.divisor == 3) {
                StParam8 *Param = (StParam8 *)ParamsIdx[id - IdxFId]->param;
                DynParam->id = (unsigned long)Param->id;
                DynParam->cipher = &Param->cipher[0];
                DynParam->name = &Param->name[0];
                DynParam->measure = &Param->measure[0];
                DynParam->imgid = Param->imgid;
                DynParam->type = Param->type;
                DynParam->chanel = Param->chanel;
                DynParam->invert = Param->invert;
                DynParam->gaddr = Param->gaddr;
            } else if (ParHead.divisor == 2) {
                StParam4 *Param = (StParam4 *)ParamsIdx[id - IdxFId]->param;
                DynParam->id = (unsigned long)Param->id;
                DynParam->cipher = &Param->cipher[0];
                DynParam->name = &Param->name[0];
                DynParam->measure = &Param->measure[0];
                DynParam->imgid = Param->imgid;
                DynParam->type = Param->type;
                DynParam->chanel = Param->chanel;
                DynParam->invert = Param->invert;
                DynParam->gaddr = Param->gaddr;
            } else if (ParHead.divisor == 1) {
                StParam2 *Param = (StParam2 *)ParamsIdx[id - IdxFId]->param;
                DynParam->id = (unsigned long)Param->id;
                DynParam->cipher = &Param->cipher[0];
                DynParam->name = &Param->name[0];
                DynParam->measure = &Param->measure[0];
                DynParam->imgid = Param->imgid;
                DynParam->type = Param->type;
                DynParam->chanel = Param->chanel;
                DynParam->invert = Param->invert;
                DynParam->gaddr = Param->gaddr;
            } else {
                StParam *Param = (StParam *)ParamsIdx[id - IdxFId]->param;
                DynParam->id = (unsigned long)Param->id;
                DynParam->cipher = &Param->cipher[0];
                DynParam->name = &Param->name[0];
                DynParam->measure = &Param->measure[0];
                DynParam->imgid = Param->imgid;
                DynParam->type = Param->type;
                DynParam->chanel = Param->chanel;
                DynParam->invert = Param->invert;
                DynParam->gaddr = Param->gaddr;
            }
            if (!DynParam->name[0])
                DynParam->name = const_cast<char *>(
                    ParamsIdx[id - IdxFId]->FullName.c_str());
            // DynParam->NeedParamStruct=&ParamsIdx[id-IdxFId]->NeedParamStruct;
            // DynParam->imgid=&ParamsIdx[id-IdxFId]->imgid;
            DynParam->ParamScope = GetParamScope(id);
            DynParam->ParamInfo = GetParamInfo(id);
            DynParam->ParamArchive = GetParamArchive(id);
            return true;
        }
        return false;
    } catch (...) {
        return false;
    }
}
// ������� ��������, �����
StParam *StParams::GetParamBin(unsigned int x, int left, int right) {
    if (left > right) {
        return NULL;
    }
    int mid = (left + right) / 2;
    if (Params[mid].id == x)
        return &Params[mid];
    if (Params[mid].id < x)
        return GetParamBin(x, mid + 1, right);
    if (Params[mid].id > x)
        return GetParamBin(x, left, mid - 1);
    return NULL;
}
//------------------------------------------------------------------------------
// ������ ��������� ����������
void StParamsControl::ReadParams(std::string fname) {
    // IdxCount=0;IdxFId=0;IdxLId=0;
    // memset(&ParHead,0,sizeof(StParControlHead));
    memset(this, 0, sizeof(StParamsControl));
    FILE *f = fopen((char *)fname.c_str(), "rb");
    if (f != NULL) {
        // fseek(f,0,SEEK_END);
        // int fsize=ftell(f);
        // fseek(f,0,SEEK_SET);
        fread(&ParHead.verrev, 4, 1, f);
        fread(ParHead.service, 4, 1, f);
        fread(&ParHead.paramcount, 2, 1, f);
        fread(&ParHead.paramcount2, 2, 1, f);
        if (ParHead.paramcount > 0) {
            CrtParams(ParHead.paramcount);
            fread(Params, sizeof(StParamControl), ParHead.paramcount, f);
        }
        // �������������
        CrtIdx();
        fclose(f);
    }
    loaded = 1;
}
// �������� �������
void StParamsControl::CrtIdx() {
    IdxCount = IdxFId = IdxLId = 0;
    if (ParHead.paramcount > 0) {
        // ����� ����������
        IdxFId = Params[0].id;
        IdxLId = Params[ParHead.paramcount - 1].id;
        IdxCount = IdxLId - IdxFId + 1;
        if (IdxCount > 0xFFFFFF) {
            IdxCount = IdxFId = IdxLId = 0;
            ParamsIdx = NULL;
        } else {
            ParamsIdx = new StParamControl *[IdxCount];
            memset(ParamsIdx, 0, IdxCount * sizeof(StParamControl *));
            for (unsigned int i = 0; i < ParHead.paramcount; i++) {
                ParamsIdx[Params[i].id - IdxFId] = &Params[i];
            }
        }
    }
}
// ������� ��������� ��������
bool StParamsControl::GetDynParam(unsigned int id,
                                  StDynParamControl *DynParam) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        if (!ParamsIdx[id - IdxFId])
            return false;
        StParamControl *Param = ParamsIdx[id - IdxFId];
        DynParam->id = (unsigned long)Param->id;
        DynParam->cipher = &Param->cipher[0];
        DynParam->name = &Param->name[0];
        DynParam->measure = &Param->measure[0];
        DynParam->imgid = Param->imgid;
        DynParam->type = Param->type;
        DynParam->chanel = Param->chanel;
        // DynParam->imgid=;
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
// ������ ������ ����������
void StTree::ReadTree2(std::string fname) {
    memset(&TreeHead, 0, sizeof(StTreeHead));
    FILE *f = fopen((char *)fname.c_str(), "rb");
    if (f != NULL) {
        fread((char *)(&TreeHead), sizeof(StTreeHead), 1, f);
        if (TreeHead.childcount > 0) {
            TreeChilds = new StTreeChild[TreeHead.childcount];
            for (int i = 0; i < TreeHead.childcount; i++) {
                TreeChilds[i].ReadChild(f, NULL);
            }
        } else
            TreeChilds = NULL;
        fclose(f);
    }
    loaded = 1;
}
// ������ �����
void StTree::ReadTree(std::string fname) {
    // memset(&TreeHead,0,sizeof(StTreeHead));
    memset(this, 0, sizeof(StTree));
    FILE *f = fopen((char *)fname.c_str(), "rb");
    if (f != NULL) {
        fread((char *)(&TreeHead), sizeof(StTreeHead), 1, f);
        if (TreeHead.blocklen > 0 && TreeHead.childcount > 0) {
            char *buf = new char[TreeHead.blocklen];
            int pos = 0;
            TreeChilds = new StTreeChild[TreeHead.childcount];
            fread(&buf[0], TreeHead.blocklen, 1, f);
            for (int i = 0; i < TreeHead.childcount; i++) {
                TreeChilds[i].ReadChildBuf(buf, pos, NULL, this);
            }
            delete[] buf;
        } else
            TreeChilds = NULL;
        fclose(f);
    }
    loaded = 1;
}
// ������ �����
void StTreeChild::ReadChild(FILE *f, StTreeChild *parentStTree) {
    fread(&TreeChHead, sizeof(StTreeChHead), 1, f);
    parentTree = parentStTree;
    if (parentTree)
        GroupName += parentTree->TreeChHead.name;
    if (TreeChHead.paramcount > 0) {
        params = new long unsigned int[TreeChHead.paramcount];
        for (int i = 0; i < TreeChHead.paramcount; i++) {
            unsigned int id32;
            fread(&id32, sizeof(id32), 1, f);
            params[i] = id32;
        }
    } else
        params = NULL;
    if (TreeChHead.childcount > 0) {
        TreeChilds = new StTreeChild[TreeChHead.childcount];
        for (int i = 0; i < TreeChHead.childcount; i++) {
            TreeChilds[i].ReadChild(f, this);
        }
    } else
        TreeChilds = NULL;
    // return 0;// TreeChHead.blocklen+sizeof(StTreeChHead)+sizeof(long
    // int)*TreeChHead.paramcount;
}
// ������ ����� �� ������
void StTreeChild::ReadChildBuf(char *buf, int &pos, StTreeChild *parentStTree,
                               StTree *RootTree) {
    memcpy(&TreeChHead, &buf[pos], sizeof(StTreeChHead));
    pos += sizeof(StTreeChHead);
    parentTree = parentStTree;
    Task =
        (TreeChHead.imgid == 9 ? true
                               : (parentStTree ? parentStTree->Task : false));
    GroupName =
        (std::string)(parentTree ? (std::string)parentTree->GroupName + "\\\\"
                                 : (std::string) "") +
        TreeChHead.name;
    ForFullName =
        (std::string)(parentTree ? (std::string)parentTree->ForFullName + "."
                                 : (std::string) "") +
        TreeChHead.name;
    if (TreeChHead.paramcount > 0) {
        RootTree->ParamCount += TreeChHead.paramcount;
        params = new long unsigned int[TreeChHead.paramcount];
        const int param_id_size = 4; /* kbin format: 32-bit param IDs */
        for (int i = 0; i < TreeChHead.paramcount; i++) {
            unsigned int id32;
            memcpy(&id32, &buf[pos], param_id_size);
            pos += param_id_size;
            params[i] = id32;
        }
    } else
        params = NULL;
    if (TreeChHead.childcount > 0) {
        TreeChilds = new StTreeChild[TreeChHead.childcount];
        for (int i = 0; i < TreeChHead.childcount; i++) {
            TreeChilds[i].ReadChildBuf(buf, pos, this, RootTree);
        }
    } else
        TreeChilds = NULL;
    // return 0;// TreeChHead.blocklen+sizeof(StTreeChHead)+sizeof(long
    // int)*TreeChHead.paramcount;
}
// ����� �� �����
StTreeChild *StTreeChild::GetChildByName(std::vector<std::string> *NameList,
                                         int ListPos) {
    // ����������� �������� ��������, ��������� ����������
    for (int i = 0; i < TreeChHead.childcount; i++) {
        if (strcmp(TreeChilds[i].TreeChHead.name,
                   NameList->at(ListPos).c_str()) == 0) {
            if (NameList->size() > static_cast<size_t>(ListPos + 1)) {
                return TreeChilds[i].GetChildByName(NameList, ListPos + 1);
            } else
                return &TreeChilds[i];
        }
    }
    return NULL;
}
// �������� �����
int StTreeChild::CheckChildByName(char *cname, int cid) {
    if (TreeChHead.childcount > cid && cid >= 0)
        if (strcmp(TreeChilds[cid].TreeChHead.name, cname) == 0)
            return cid;
    return GetChildIdxByName(cname);
}
// ������� �� �� �����
int StTreeChild::GetChildIdxByName(char *cname) {
    for (int i = 0; i < TreeChHead.childcount; i++) {
        if (strcmp(TreeChilds[i].TreeChHead.name, cname) == 0)
            return i;
    }
    return -1;
}
// �������� �������
void StTree::CrtIdx() {
    if (IdxCount > 0) {
        ParamsIdx = new StTreeChild *[IdxCount];
        memset(ParamsIdx, 0, IdxCount * sizeof(StTreeChild *));
        for (unsigned int i = 0; i < TreeHead.childcount; i++) {
            RecIdx(&TreeChilds[i]);
        }
    } else
        ParamsIdx = NULL;
}
// �������� ������� ��������
void StTree::RecIdx(StTreeChild *Child) {
    for (int i = 0; i < Child->TreeChHead.paramcount; i++) {
        if (Child->params[i] >= IdxFId && Child->params[i] <= IdxLId) {
            ParamsIdx[Child->params[i] - IdxFId] = Child;
            ParamsRefIdx[Child->params[i] - IdxFId]->FullName =
                (std::string)Cntrl->name + "." + Child->ForFullName + "." +
                ParamsRefIdx[Child->params[i] - IdxFId]->FullName;
        }
    }
    for (unsigned int i = 0; i < Child->TreeChHead.childcount; i++) {
        RecIdx(&Child->TreeChilds[i]);
    }
}
// ������� �����
StTreeChild *StTree::GetChild(unsigned int id) {
    if (IdxCount > 0 && id >= IdxFId && id <= IdxLId) {
        return ParamsIdx[id - IdxFId];
    }
    return NULL;
}
// ����� �� �����
StTreeChild *StTree::GetChildByName(std::vector<std::string> *NameList,
                                    int ListPos) {
    // ����������� �������� ��������, ��������� ����������
    for (int i = 0; i < TreeHead.childcount; i++) {
        if (strcmp(TreeChilds[i].TreeChHead.name,
                   NameList->at(ListPos).c_str()) == 0) {
            if (NameList->size() > static_cast<size_t>(ListPos + 1)) {
                return TreeChilds[i].GetChildByName(NameList, ListPos + 1);
            } else
                return &TreeChilds[i];
        }
    }
    return NULL;
}
// �������� �����
int StTree::CheckChildByName(char *cname, int cid) {
    if (TreeHead.childcount > cid && cid >= 0)
        if (strcmp(TreeChilds[cid].TreeChHead.name, cname) == 0)
            return cid;
    return GetChildIdxByName(cname);
}
// ������� �� �� �����
int StTree::GetChildIdxByName(char *cname) {
    for (int i = 0; i < TreeHead.childcount; i++) {
        if (strcmp(TreeChilds[i].TreeChHead.name, cname) == 0)
            return i;
    }
    return -1;
}
// ������� �� �� ����� ������� �
int StTree::GetChildIdxByNameAt(char *cname, int aid) {
    aid++;
    if (TreeHead.childcount > aid)
        for (int i = aid >= 0 ? aid : 0; i < TreeHead.childcount; i++) {
            if (strcmp(TreeChilds[i].TreeChHead.name, cname) == 0)
                return i;
        }
    return -1;
}
//------------------------------StMachins---------------------------------------
StMachins::StMachins(std::string Path, bool one) : RefData(NULL) {
    InitPath = Path;
    oneMa = one;
    memset(Machins, 0, sizeof(Machins));
    // MNames="�������";
};
StMachins::~StMachins() {
    for (unsigned int i = 0; i < sizeof(Machins) / sizeof(void *); i++) {
        if (Machins[i]) {
            Machins[i]->Del();
            delete Machins[i];
        }
    }
};
// ������� � ��������� ������ ������������
void StMachins::CrtMachine(unsigned char id) {
    if (!Machins[id]) {
        Machins[id] = new StControllers;
        Machins[id]->ReadControllers(
            InitPath +
                (oneMa ? std::string(CfgOnePlus)
                       : std::string(CfgMaMask) + std::to_string(id) + CfgPlus),
            Stations->GetStationName(id));
        // Machins[id]->MaName=Stations->GetStationName(id);
    }
};
// ���������������� ����������
void StMachins::InitMachineCntrl(unsigned char MaId, short unsigned int CntId) {
    if (oneMa)
        MaId = 0;
    CrtMachine(MaId);
    // ��� ������
    StGUID *stg = NULL;
    int i = Machins[MaId]->GetCntrlIdIdx(CntId);
    if (!oneMa && i != 0xFFFF && Machins[MaId]->Controllers[i].type == 0x01) {
        unsigned char stid =
            *(unsigned char *)(&Machins[MaId]->Controllers[i].configid);
        int lctid = Machins[stid]->GetCntrlIdIdx(CntId);
        if (lctid != 0xFFFF)
            stg = &Machins[stid]->Controllers[lctid].configid;
    }
    Machins[MaId]->LoadParamsTree(
        InitPath +
            (oneMa ? (std::string)CfgOnePlus
                   : (std::string)CfgMaMask + std::to_string(MaId) + CfgPlus),
        CntId, stg);
}
// ����������������
char StMachins::InitMachine(unsigned char MaId) {
    if (oneMa)
        MaId = 0;
    CrtMachine(MaId);
    return (Machins[MaId]->CntrlHead.controlcount > 0 ? 0 : -1);
}
// ���������������� ���
void StMachins::InitAllMachine() {
    for (int i = 0; i < (oneMa ? 1 : MachinsCount); i++) {
        CrtMachine(i);
    }
    CrtUse();
}
// ����������������
void StMachins::CrtUse() {
    UseCount = 0;
    for (int i = 0; i < (oneMa ? 1 : MachinsCount); i++) {
        if (Machins[i]->CntrlHead.controlcount) {
            MachinsUse[UseCount] = i;
            UseCount++;
        }
    }
}
// ������� ��
int StMachins::GetMaId(unsigned char Id) {
    if (Id < UseCount) {
        return MachinsUse[Id];
    }
    return -1;
}
// ������� �����������
StControllers *StMachins::GetControllers(unsigned char MaId) {
    if (oneMa)
        MaId = 0;
    CrtMachine(MaId);
    return Machins[MaId];
}
// ������� ������
StTree *StMachins::GetTree(unsigned char MaId, short unsigned int CntId) {
    if (oneMa)
        MaId = 0;
    InitMachineCntrl(MaId, CntId);
    return Machins[MaId]->GetTree(CntId);
}
// ������� ����������
StController *StMachins::GetCntrl(unsigned char MaId,
                                  short unsigned int CntId) {
    if (oneMa)
        MaId = 0;
    CrtMachine(MaId);
    return Machins[MaId]->GetCntrl(CntId);
}
// ������� ���������� IP
StControllerIP *StMachins::GetCntrlIP(unsigned char MaId,
                                      short unsigned int CntId) {
    if (oneMa)
        MaId = 0;
    CrtMachine(MaId);
    return Machins[MaId]->GetCntrlIP(CntId);
}
// ������� ������ ��� ����������
char *StMachins::GetCntrlMaName(unsigned char MaId, short unsigned int CntId) {
    if (oneMa)
        MaId = 0;
    CrtMachine(MaId);
    return Machins[MaId]->GetMaName(CntId);
}
// ������� �� ����������� �� �����
int StMachins::GetCntrlIdByName(unsigned char MaId, char *cname) {
    if (oneMa)
        MaId = 0;
    CrtMachine(MaId);
    return Machins[MaId]->GetCntrlIdByName(cname);
}
// ������� ������
void *StMachins::GetRefData(unsigned char MaId, short unsigned int CntId) {
    if (oneMa)
        MaId = 0;
    CrtMachine(MaId);
    return Machins[MaId]->GetRefData(CntId);
}
// ������� ��������� ���������
StParamsControl *StMachins::GetParamsControl(unsigned char MaId,
                                             short unsigned int CntId) {
    if (oneMa)
        MaId = 0;
    InitMachineCntrl(MaId, CntId);
    return Machins[MaId]->GetParamsControl(CntId);
}
// ������� ��������
StParam *StMachins::GetParam(unsigned char MaId, short unsigned int CntId,
                             unsigned int ParamId) {
    if (oneMa)
        MaId = 0;
    InitMachineCntrl(MaId, CntId);
    StParams *pr = Machins[MaId]->GetParams(CntId);
    if (pr) {
        return pr->GetParam(ParamId);
    }
    return NULL;
}
// ������� ��������
bool StMachins::GetDynParam(unsigned char MaId, short unsigned int CntId,
                            unsigned int ParamId, StDynParam *DynParam) {
    if (oneMa)
        MaId = 0;
    InitMachineCntrl(MaId, CntId);
    StParams *pr = Machins[MaId]->GetParams(CntId);
    if (pr && pr->GetDynParam(ParamId, DynParam)) {
        StTree *tr = Machins[MaId]->GetTree(CntId);
        if (tr) {
            DynParam->TreeChild = tr->GetChild(ParamId);
        } else
            DynParam->TreeChild = NULL;
        return true;
    }
    return false;
}
// ������� ��������� ��������
bool StMachins::GetDynParamControl(unsigned char MaId, short unsigned int CntId,
                                   unsigned int ParamId,
                                   StDynParamControl *DynParam) {
    if (oneMa)
        MaId = 0;
    InitMachineCntrl(MaId, CntId);
    StParamsControl *pr = Machins[MaId]->GetParamsControl(CntId);
    if (pr && pr->GetDynParam(ParamId, DynParam)) {
        return true;
    }
    return false;
}
//------------------------------StMachins---------------------------------------
