#ifndef FastParamConstH
#define FastParamConstH

// идентификаторы типов параметров (как в Windows FastParamConst)
const short ParamTypeKlogic = 222;
const short ParamTypeKlogicControll = 223;
const short ParamTypeAlarmFrom = 111;
const short ParamTypeAlarmTo = 119;
const short ParamTypeAnyFrom = 1;
const short ParamTypeAnyTo = 11;

// типы директорий дерева (KLogic / Алармы / Любые)
enum PosTreeType { pttKlogic, pttAlarm, pttAny, pttUnknown, pttReserved };

extern const char* KlogicName;
extern const char* KlogicControlName;

#endif
