//---------------------------------------------------------------------------

#ifndef BinDataStructH
#define BinDataStructH

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cstring>
#include <string>

#ifdef FORCE_PACKING
#pragma pack(push, 1)
#endif

//GUID
struct StGUID{
char d1[4];
char d2[2],d3[2],d4[2];
char d5[6];
std::string GetStr();
};
#define MachinsCount 256

#include "FastParamTypes.h"

//---------------------------------------------------------------------------
//char* CfgMask="\\\\KLogic\\\\Station_%u\\\\Cfg\\\\";
extern const char* CfgMaMask;
extern const char* CfgPlus;
extern const char* CfgOnePlus;
//---------------------------------------------------------------------------
//версия, ревизия
struct StVerRev{
char ver;
char rev;
};
//------------------------------------------------------------------------------
//параметр
struct StParam{
short unsigned int id;
short unsigned int id2;
unsigned char invert;
unsigned char imgid;
unsigned char type;
unsigned char chanel;
char cipher[256];
char name[256];
char measure[48];
short unsigned int gaddr;
};
//параметр, делитель 1
struct StParam2{
short unsigned int id;
short unsigned int id2;
unsigned char invert;
unsigned char imgid;
unsigned char type;
unsigned char chanel;
char cipher[128];
char name[128];
char measure[24];
short unsigned int gaddr;
};
//параметр, делитель 2
struct StParam4{
short unsigned int id;
short unsigned int id2;
unsigned char invert;
unsigned char imgid;
unsigned char type;
unsigned char chanel;
char cipher[64];
char name[64];
char measure[12];
short unsigned int gaddr;
};
//параметр, делитель 3
struct StParam8{
short unsigned int id;
short unsigned int id2;
unsigned char invert;
unsigned char imgid;
unsigned char type;
unsigned char chanel;
char cipher[32];
char name[32];
char measure[6];
short unsigned int gaddr;
};
//информация о границах
struct StParamScope{
long unsigned int id;
float uplimit;
float downlimit;
float upcrash;
float downcrash;
float upwarning;
float downwarning;
};
//дополнительная информация
struct StParamInfo{
long unsigned int id;
float scale;
float offset;
float insensfrom;
float insensto;
float insensval;
};
//информация о архиве
struct StParamArchive{
long unsigned int id;
long unsigned int num;
StGUID archiveid;
};
struct StTreeChild;
//динамический параметр
struct StDynParam{
long unsigned int id;
unsigned char invert;
unsigned char imgid;
unsigned char type;
unsigned char chanel;
const char *cipher;
const char *name;
const char *measure;
short unsigned int gaddr;
StParamScope* ParamScope;
StParamInfo* ParamInfo;
StParamArchive* ParamArchive;
StTreeChild* TreeChild;
//TPassport* NeedParamStruct;
//int* imgid;
};
//заголовок списка параметров
struct StParHead{
StVerRev verrev;
char service[5];
unsigned char divisor;
short unsigned int paramcount;
short unsigned int paramcount2;
long unsigned int scopecount;
long unsigned int infocount;
long unsigned int archivecount;
};
//индекс параметров
struct StParamsDataIdx{
 void *param;
 std::string FullName;
 //TPassport NeedParamStruct;
 //int imgid;
};
//список параметров
struct StParams{
StParHead ParHead;
StParam *Params;
StParam2 *Params2;
StParam4 *Params4;
StParam8 *Params8;
StParamScope* ParamScopes;
StParamInfo* ParamInfos;
StParamArchive* ParamArchives;
StParamsDataIdx **ParamsIdx;
StParamsDataIdx *ParamsDataIdx;
StParamScope** ParamScopesIdx;
StParamInfo** ParamInfoIdx;
StParamArchive** ParamArchivesIdx;
//std::string *FullNames;
//TPassport *NeedParamStruct;
char loaded;
unsigned int IdxCount,IdxFId,IdxLId;
unsigned int IdxScopeCount,IdxScopeFId,IdxScopeLId;
unsigned int IdxInfoCount,IdxInfoFId,IdxInfoLId;
unsigned int IdxArchiveCount,IdxArchiveFId,IdxArchiveLId;
void ReadParams(std::string fname);
void CrtParams(int count){
  if(ParHead.divisor==3){
	Params8=new StParam8[count];
  }else if(ParHead.divisor==2){
	Params4=new StParam4[count];
  }else if(ParHead.divisor==1){
	Params2=new StParam2[count];
  }else{
	Params=new StParam[count];
  }
};
void CrtParamsScope(int count){
	ParamScopes=new StParamScope[count];
};
void CrtParamsInfo(int count){
	ParamInfos=new StParamInfo[count];
};
void CrtParamsArchive(int count){
	ParamArchives=new StParamArchive[count];
};
void CrtParamDataIdxStruct(int count){
	//NeedParamStruct=new TPassport[count];
	//memset(NeedParamStruct,0,count*sizeof(TPassport));
	//FullNames=new std::string[count];
	ParamsDataIdx=new StParamsDataIdx[count];
	memset(ParamsDataIdx,0,count*sizeof(StParamsDataIdx));
};
void CrtIdx();
void DelParams(){if(Params8)delete[] Params8;if(Params4)delete[] Params4;if(Params2)delete[] Params2;if(Params)delete[] Params;};
void DelParamsScope(){if(ParamScopes)delete[] ParamScopes;};
void DelParamsInfo(){if(ParamInfos)delete[] ParamInfos;};
void DelParamsArchive(){if(ParamArchives)delete[] ParamArchives;};
void DelIdx(){if(ParamsIdx)delete[] ParamsIdx;};
void DelParamDataIdx(){if(ParamsDataIdx)delete[] ParamsDataIdx;};
void Del(){DelIdx();DelParamDataIdx();DelParams();DelParamsScope();DelParamsInfo();DelParamsArchive();};
void SetParamsDataIdx(unsigned long int id,unsigned long int ididx,unsigned long int pid,unsigned long int imgid,void *param,const char *cipher);
StParam *GetParam(unsigned int id);
StParam *GetParamBin(unsigned int id,int left,int right);
StParamScope *GetParamScope(unsigned int id);
StParamInfo *GetParamInfo(unsigned int id);
StParamArchive *GetParamArchive(unsigned int id);
bool GetDynParam(unsigned int id,StDynParam *DynParam);
};
//------------------------------------------------------------------------------
//служебные параметры
struct StParamControl{
long unsigned int id;
unsigned char service;
unsigned char imgid;
unsigned char type;
unsigned char chanel;
char cipher[256];
char name[256];
char measure[48];
};
//динамический служебный параметр
struct StDynParamControl{
long unsigned int id;
//unsigned char invert;
unsigned char imgid;
unsigned char type;
unsigned char chanel;
const char *cipher;
const char *name;
const char *measure;
//TPassport* NeedParamStruct;
};
//заголовок списка служебных параметров
struct StParControlHead{
StVerRev verrev;
char service[4];
short unsigned int paramcount;
short unsigned int paramcount2;
};
//список служебных параметров
struct StParamsControl{
StParControlHead ParHead;
StParamControl *Params;
StParamControl **ParamsIdx;
void *RefData;
char loaded;
unsigned int IdxCount,IdxFId,IdxLId;
void ReadParams(std::string fname);
void CrtParams(int count){
  Params=new StParamControl[count];
};
void CrtIdx();
void DelParams(){if(Params)delete[] Params;};
void DelIdx(){if(ParamsIdx)delete[] ParamsIdx;}
void Del(){DelIdx();DelParams();};
bool GetDynParam(unsigned int id,StDynParamControl *DynParam);
};
//------------------------------------------------------------------------------
//заголовок уровня дерева параметров
struct StTreeChHead{
char name[256];
char comment[240];
char ipaddr[16];
char service;
unsigned char imgid;
short unsigned int type;
short unsigned int paramcount;
short unsigned int paramcount2;
short unsigned int childcount;
short unsigned int childcount2;
/*short*/ unsigned int blocklen;
//short unsigned int blocklen2;
};
//уровень дерева параметров
struct StTree;
struct StTreeChild{
StTreeChHead TreeChHead;
long unsigned int *params;
StTreeChild *TreeChilds;
StTreeChild *parentTree;
std::string GroupName;
std::string ForFullName;
bool Task;
void *RefData;
void ReadChild(FILE* f,StTreeChild *parentStTree);
void ReadChildBuf(char* buf,int &pos,StTreeChild* parentStTree,StTree* RootTree);
void DelTree(){if(TreeChilds){for(int i=0;i<TreeChHead.childcount;i++)TreeChilds[i].Del();delete[] TreeChilds;}};
void DelParams(){if(params)delete[] params;};
void Del(){DelTree();DelParams();};
StTreeChild* GetChildByName(std::vector<std::string> *NameList,int ListPos);
int CheckChildByName(char *cname,int cid);
int GetChildIdxByName(char *cname);
};
//заголовок дерева параметров
struct StTreeHead{
StVerRev verrev;
char service[5];
unsigned char type;
short unsigned int id;
short unsigned int childcount;
short unsigned int childcount2;
/*short*/ unsigned int blocklen;
//short unsigned int blocklen2;
};
// 
struct StController;
//дерево дерева параметров
struct StTree{
StTreeHead TreeHead;
StTreeChild *TreeChilds;
StTreeChild **ParamsIdx;
StParamsDataIdx **ParamsRefIdx;
StController *Cntrl;
char loaded;
unsigned int IdxCount,IdxFId,IdxLId;
unsigned int ParamCount;
void ReadTree(std::string fname);
void ReadTree2(std::string fname);
void CrtIdx();
void RecIdx(StTreeChild* Child);
void DelIdx(){if(ParamsIdx)delete[] ParamsIdx;};
void DelTree(){if(TreeChilds){for(int i=0;i<TreeHead.childcount;i++)TreeChilds[i].Del();delete[] TreeChilds;}};
void Del(){DelIdx();DelTree();};
StTreeChild* GetChild(unsigned int id);
StTreeChild* GetChildByName(std::vector<std::string> *NameList,int ListPos);
int CheckChildByName(char *cname,int cid);
int GetChildIdxByName(char *cname);
int GetChildIdxByNameAt(char *cname,int aid);
};
//------------------------------------------------------------------------------
//контроллер
struct StController{
char name[256];
char comment[256];
unsigned char service;
unsigned char type;
short unsigned int id;
StGUID configid;
};
//контроллер IP порт
struct StControllerIP{
char name[256];
char comment[256];
unsigned char service;
unsigned char type;
short unsigned int id;
StGUID configid;
unsigned char conntype;
char ip[16];
char ip_reserv[16];
unsigned char useIEC104;
unsigned short port;
unsigned short klport;
unsigned short klportr;
char serviceport[2];
char service2[64];
};
//заголовок списка контроллеров
struct StCntrlHead{
StVerRev verrev;
char service[4];
short unsigned int controlcount;
short unsigned int controlcount2;
};
//список контроллеров
struct StControllers{
StCntrlHead CntrlHead;
StController *Controllers;
StCntrlHead CntrlHeadIP;
StControllerIP *ControllersIP;
StParams *Params;
StTree *Trees;
StParamsControl *ParamsControl;
void** RefsData;
void* RefData;
std::string *MaNames;
short unsigned int *CntrlIdIdx;
unsigned int IdxCount,IdxFId,IdxLId;
std::string MaName;
void CrtIdx();
StController *GetCntrl(unsigned int id);
StControllerIP *GetCntrlIP(unsigned int id);
char *GetMaName(unsigned int id);
void* GetRefData(unsigned int id);
StParams *GetParams(unsigned int id);
StTree *GetTree(unsigned int id);
StParamsControl *GetParamsControl(unsigned int id);
bool FillParamAddrStruct(unsigned int id,TPAA & ParamAddrArray,void* pGrpArr);
void RecParamAddrStruct(StTreeChild* TreeChild,StParams*TreeParam,TPAA & ParamAddrArray,std::vector<TIntArr> *pGrpArr,int& nid);
int GetCntrlIdIdx(unsigned int id);
int GetCntrlIdxId(unsigned int id);
int GetCntrlIdByName(char *cname);
int CheckCntrlIdByName(char *cname,int cid);
void LoadParamsTree(std::string dname,unsigned int id,StGUID *lstg);
void LoadAllParamsTree(std::string dname);
void ReadControllers(std::string fname,std::string grname);
void CrtControllers(int count){
  Controllers=new StController[count];
  RefsData=new void*[count];
};
void CrtControllersIP(int count){
  ControllersIP=new StControllerIP[count];
  //RefsData=new void*[count];
};
void CrtStParams(int count){
  Params=new StParams[count];
  memset(Params,0,sizeof(StParams)*count);
};
void CrtStTrees(int count){
  Trees=new StTree[count];
  memset(Trees,0,sizeof(StTree)*count);
};
void CrtStParamsControl(int count){
  ParamsControl=new StParamsControl[count];
  memset(ParamsControl,0,sizeof(StParamsControl)*count);
};
void DelControllers(){if(Controllers)delete[] Controllers;if(ControllersIP)delete[] ControllersIP;if(RefsData)delete[] RefsData;};
void DelStParams(){if(Params){for(int i=0;i<CntrlHead.controlcount;i++)Params[i].Del();delete[] Params;}};
void DelStTrees(){if(Trees){for(int i=0;i<CntrlHead.controlcount;i++)Trees[i].Del();delete[] Trees;}};
void DelStParamsControl(){if(ParamsControl){for(int i=0;i<CntrlHead.controlcount;i++)ParamsControl[i].Del();delete[] ParamsControl;}};
void DelIdx(){if(CntrlIdIdx)delete[] CntrlIdIdx;if(MaNames)delete[] MaNames;};
void Del(){DelIdx();DelControllers();DelStParams();DelStTrees();DelStParamsControl();};
};
//список станций
class StMachins{
private:
std::string InitPath;
bool oneMa;
StControllers *Machins[MachinsCount];
int MachinsUse[MachinsCount];
int UseCount;
void CrtMachine(unsigned char id);
void InitMachineCntrl(unsigned char MaId,short unsigned int CntId);
public:
void *RefData;
StMachins(std::string Path,bool one=false);
~StMachins();
//std::string MNames;
char InitMachine(unsigned char MaId);
void InitAllMachine();
void CrtUse();
int GetMaId(unsigned char Id);
int GetUseCount(){return UseCount;};
StControllers* GetControllers(unsigned char MaId);
StTree* GetTree(unsigned char MaId,short unsigned int CntId);
StController* GetCntrl(unsigned char MaId,short unsigned int CntId);
StControllerIP* GetCntrlIP(unsigned char MaId,short unsigned int CntId);
char* GetCntrlMaName(unsigned char MaId,short unsigned int CntId);
int GetCntrlIdByName(unsigned char MaId,char *cname);
void* GetRefData(unsigned char MaId,short unsigned int CntId);
StParamsControl* GetParamsControl(unsigned char MaId,short unsigned int CntId);
StParam* GetParam(unsigned char MaId,short unsigned int CntId,unsigned int ParamId);
bool GetDynParam(unsigned char MaId,short unsigned int CntId,unsigned int ParamId,StDynParam *DynParam);
bool GetDynParamControl(unsigned char MaId,short unsigned int CntId,unsigned int ParamId,StDynParamControl *DynParam);
};


StControllers ReadControllers(std::string fname);
void ReadParams(std::string fname);
//
extern StMachins *Machins;

#ifdef FORCE_PACKING
#pragma pack(pop)
#endif
#endif
