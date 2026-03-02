#ifndef FastParamTypeH
#define FastParamTypeH

#include <stdint.h>
#include <vector>

#pragma pack(push, 1)

typedef struct
{
  union
  {
    struct
    {
	  uint8_t StationID;
	  uint8_t TypePasp;
	  uint8_t GroupID;
	  uint16_t PaspID;
	};
    uint32_t dwPaspID;
  };
  int FastID;
  uint64_t TimeStamp;
  union
  {
    struct
    {
      uint8_t Lo;
      uint8_t Hi;
    };
    uint16_t Q;
  } Quality;
  uint8_t ValueType;
  union
  {
	float fValue;
	uint8_t bValue;
  } TPasportValue;
} TPassport;

typedef struct
{
  const char* StationName;
  const char* StationTypeName;
  const char* PassportTypeName;
  const char* ParameterID;
  const char* GroupName;
  const char* FullName;
  const char* MeasureUnit;
  float UpMeasuringBorder;
  float DownMeasuringBorder;
  float UpPreCrashingBorder;
  float DownPreCrashingBorder;
  float UpCrashingBorder;
  float DownCrashingBorder;
} TPasspInfo;

typedef struct TParamAddr
{
  uint16_t TagID;
  uint16_t ModbusAddr;
  StGUID ArchiveGUID;
  uint8_t NumberinArchive;
  float Multiplier;
  float Summand;
  bool  Invert;
  uint8_t Type;
  bool Write;
  float AMA;
  float AMI;
  float PMA;
  float PMI;
  float NechFrom;
  float NechTo;
  float NechVal;
  float KoefFilt;
  bool isDemo;
} TParamAddr;

typedef std::vector<TParamAddr> TPAA;
typedef std::vector<TParamAddr*> TPAptrA;

enum TGroupType:char {gtPasspRoot, gtKLogicRoot, gtStation, gtController, gtGroup};

typedef struct TGroupInfo
{
  const char* Name;
  TGroupType Type;
  uint8_t Id;
  int ImageIndex;
} TGroupInfo;

#pragma pack(pop)

#define nmmaxint 1000
typedef const char** TPCharPtrArray[nmmaxint];
typedef TPassport* TPasspPtrArray[nmmaxint];
typedef int* TIntPtrArray[nmmaxint];
typedef uint16_t TTUAddrArray[nmmaxint];
typedef std::vector<uint16_t> TTUAddrDynArray;
typedef std::vector<int> TIntArr;

typedef struct{
  char type;
  int MaId;
  int CntId;
  short int count;
  short int level[256];
  void *Data;
  void *ChildData;
  void *RootData;
} TCurrentPos;

#endif
